// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: lode.vandevenne@gmail.com (Lode Vandevenne)
// Author: jyrki.alakuijala@gmail.com (Jyrki Alakuijala)

// See zopflipng_lib.h

#include "zopflipng_lib.h"

#include <stdio.h>
#include <vector>

#include "lodepng/lodepng.h"
#include "lodepng/lodepng_util.h"
#include "../zopfli/deflate.h"

ZopfliPNGOptions::ZopfliPNGOptions()
  : lossy_transparent(false)
  , lossy_8bit(false)
  , auto_filter_strategy(true)
  , use_zopfli(true)
  , num_iterations(15)
  , num_iterations_large(5)
  , block_split_strategy(1) {
}

// Deflate compressor passed as fuction pointer to LodePNG to have it use Zopfli
// as its compression backend.
unsigned CustomPNGDeflate(unsigned char** out, size_t* outsize,
                          const unsigned char* in, size_t insize,
                          const LodePNGCompressSettings* settings) {
  const ZopfliPNGOptions* png_options =
      static_cast<const ZopfliPNGOptions*>(settings->custom_context);
  unsigned char bp = 0;
  ZopfliOptions options;
  ZopfliInitOptions(&options);

  options.numiterations = insize < 200000
      ? png_options->num_iterations : png_options->num_iterations_large;

  if (png_options->block_split_strategy == 3) {
    // Try both block splitting first and last.
    unsigned char* out2 = 0;
    size_t outsize2 = 0;
    options.blocksplittinglast = 0;
    ZopfliDeflate(&options, 2 /* Dynamic */, 1, in, insize, &bp, out, outsize);
    bp = 0;
    options.blocksplittinglast = 1;
    ZopfliDeflate(&options, 2 /* Dynamic */, 1,
                  in, insize, &bp, &out2, &outsize2);

    if (outsize2 < *outsize) {
      free(*out);
      *out = out2;
      *outsize = outsize2;
      printf("Block splitting last was better\n");
    } else {
      free(out2);
    }
  } else {
    if (png_options->block_split_strategy == 0) options.blocksplitting = 0;
    options.blocksplittinglast = png_options->block_split_strategy == 2;
    ZopfliDeflate(&options, 2 /* Dynamic */, 1, in, insize, &bp, out, outsize);
  }

  return 0;  // OK
}

// Remove RGB information from pixels with alpha=0
void LossyOptimizeTransparent(unsigned char* image, unsigned w, unsigned h) {
  // First check if we want to preserve potential color-key background color,
  // or instead use the last encountered RGB value all the time to save bytes.
  bool key = true;
  for (size_t i = 0; i < w * h; i++) {
    if (image[i * 4 + 3] > 0 && image[i * 4 + 3] < 255) {
      key = false;
      break;
    }
  }

  // Choose the color key if color keying is used.
  int r = 0, g = 0, b = 0;
  if (key) {
    for (size_t i = 0; i < w * h; i++) {
      if (image[i * 4 + 3] == 0) {
        // Use first encountered transparent pixel as the color key
        r = image[i * 4 + 0];
        g = image[i * 4 + 1];
        b = image[i * 4 + 2];
      }
    }
  }

  for (size_t i = 0; i < w * h; i++) {
    // if alpha is 0
    if (image[i * 4 + 3] == 0) {
      image[i * 4 + 0] = r;
      image[i * 4 + 1] = g;
      image[i * 4 + 2] = b;
    } else {
      if (!key) {
        // Use the last encountered RGB value if no color keying is used.
        r = image[i * 4 + 0];
        g = image[i * 4 + 1];
        b = image[i * 4 + 2];
      }
    }
  }
}

// Tries to optimize given a single PNG filter strategy.
// Returns 0 if ok, other value for error
unsigned TryOptimize(
    const std::vector<unsigned char>& image, unsigned w, unsigned h,
    const lodepng::State& inputstate, bool bit16,
    const std::vector<unsigned char>& origfile,
    ZopfliPNGFilterStrategy filterstrategy,
    bool use_zopfli, int windowsize, const ZopfliPNGOptions* png_options,
    std::vector<unsigned char>* out) {
  unsigned error = 0;

  lodepng::State state;
  state.encoder.zlibsettings.windowsize = windowsize;
  if (use_zopfli && png_options->use_zopfli) {
    ZopfliPNGOptions custom_context = *png_options;
    state.encoder.zlibsettings.custom_deflate = CustomPNGDeflate;
    state.encoder.zlibsettings.custom_context = &custom_context;
  }

  if (inputstate.info_png.color.colortype == LCT_PALETTE) {
    // Make it preserve the original palette order
    lodepng_color_mode_copy(&state.info_raw, &inputstate.info_png.color);
    state.info_raw.colortype = LCT_RGBA;
    state.info_raw.bitdepth = 8;
  }
  if (bit16) {
    state.info_raw.bitdepth = 16;
  }

  state.encoder.filter_palette_zero = 0;

  std::vector<unsigned char> filters;
  switch (filterstrategy) {
    case kStrategyZero:
      state.encoder.filter_strategy = LFS_ZERO;
      break;
    case kStrategyMinSum:
      state.encoder.filter_strategy = LFS_MINSUM;
      break;
    case kStrategyEntropy:
      state.encoder.filter_strategy = LFS_ENTROPY;
      break;
    case kStrategyBruteForce:
      state.encoder.filter_strategy = LFS_BRUTE_FORCE;
      break;
    case kStrategyOne:
    case kStrategyTwo:
    case kStrategyThree:
    case kStrategyFour:
      // Set the filters of all scanlines to that number.
      filters.resize(h, filterstrategy);
      state.encoder.filter_strategy = LFS_PREDEFINED;
      state.encoder.predefined_filters = &filters[0];
      break;
    case kStrategyPredefined:
      lodepng::getFilterTypes(filters, origfile);
      state.encoder.filter_strategy = LFS_PREDEFINED;
      state.encoder.predefined_filters = &filters[0];
      break;
    default:
      break;
  }

  state.encoder.add_id = false;
  state.encoder.text_compression = 1;

  error = lodepng::encode(*out, image, w, h, state);

  // For very small output, also try without palette, it may be smaller thanks
  // to no palette storage overhead.
  if (!error && out->size() < 4096) {
    lodepng::State teststate;
    std::vector<unsigned char> temp;
    lodepng::decode(temp, w, h, teststate, *out);
    LodePNGColorMode& color = teststate.info_png.color;
    if (color.colortype == LCT_PALETTE) {
      std::vector<unsigned char> out2;
      state.encoder.auto_convert = LAC_ALPHA;
      bool grey = true;
      for (size_t i = 0; i < color.palettesize; i++) {
        if (color.palette[i * 4 + 0] != color.palette[i * 4 + 2]
            || color.palette[i * 4 + 1] != color.palette[i * 4 + 2]) {
          grey = false;
          break;
        }
      }
      if (grey) state.info_png.color.colortype = LCT_GREY_ALPHA;

      error = lodepng::encode(out2, image, w, h, state);
      if (out2.size() < out->size()) out->swap(out2);
    }
  }

  if (error) {
    printf("Encoding error %u: %s\n", error, lodepng_error_text(error));
    return error;
  }

  return 0;
}

// Use fast compression to check which PNG filter strategy gives the smallest
// output. This allows to then do the slow and good compression only on that
// filter type.
unsigned AutoChooseFilterStrategy(const std::vector<unsigned char>& image,
                                  unsigned w, unsigned h,
                                  const lodepng::State& inputstate, bool bit16,
                                  const std::vector<unsigned char>& origfile,
                                  int numstrategies,
                                  ZopfliPNGFilterStrategy* strategies,
                                  bool* enable) {
  std::vector<unsigned char> out;
  size_t bestsize = 0;
  int bestfilter = 0;

  // A large window size should still be used to do the quick compression to
  // try out filter strategies: which filter strategy is the best depends
  // largely on the window size, the closer to the actual used window size the
  // better.
  int windowsize = 8192;

  for (int i = 0; i < numstrategies; i++) {
    out.clear();
    unsigned error = TryOptimize(image, w, h, inputstate, bit16, origfile,
                                 strategies[i], false, windowsize, 0, &out);
    if (error) return error;
    if (bestsize == 0 || out.size() < bestsize) {
      bestsize = out.size();
      bestfilter = i;
    }
  }

  for (int i = 0; i < numstrategies; i++) {
    enable[i] = (i == bestfilter);
  }

  return 0;  /* OK */
}

// Keeps chunks with given names from the original png by literally copying them
// into the new png
void KeepChunks(const std::vector<unsigned char>& origpng,
                const std::vector<std::string>& keepnames,
                std::vector<unsigned char>* png) {
  std::vector<std::string> names[3];
  std::vector<std::vector<unsigned char> > chunks[3];

  lodepng::getChunks(names, chunks, origpng);
  std::vector<std::vector<unsigned char> > keepchunks[3];

  // There are 3 distinct locations in a PNG file for chunks: between IHDR and
  // PLTE, between PLTE and IDAT, and between IDAT and IEND. Keep each chunk at
  // its corresponding location in the new PNG.
  for (size_t i = 0; i < 3; i++) {
    for (size_t j = 0; j < names[i].size(); j++) {
      for (size_t k = 0; k < keepnames.size(); k++) {
        if (keepnames[k] == names[i][j]) {
          keepchunks[i].push_back(chunks[i][j]);
        }
      }
    }
  }

  lodepng::insertChunks(*png, keepchunks);
}

int ZopfliPNGExternalOptimize(const unsigned char* in, size_t insize, std::vector<unsigned char>* resultpng)
{
	ZopfliPNGOptions png_options = ZopfliPNGOptions();
	unsigned error = 0;
	std::vector<unsigned char> origpng(in, in + insize);
	std::vector<unsigned char> result;
	std::vector<unsigned char> image;
    unsigned w, h;

	lodepng::State inputstate;
	error = ZopfliPNGOptimize(origpng, png_options, false, &result);

    // Verify result, check that the result causes no decoding errors
    if (!error)
      error = lodepng::decode(image, w, h, inputstate, result);

    if (!error)
	{
		int length = result.size();
		(*resultpng).swap(result);
		return length;
	}

	return error;
}

int ZopfliPNGOptimize(const std::vector<unsigned char>& origpng,
    const ZopfliPNGOptions& png_options,
    bool verbose,
    std::vector<unsigned char>* resultpng) {
  // Use the largest possible deflate window size
  int windowsize = 32768;

  ZopfliPNGFilterStrategy filterstrategies[kNumFilterStrategies] = {
    kStrategyZero, kStrategyOne, kStrategyTwo, kStrategyThree, kStrategyFour,
    kStrategyMinSum, kStrategyEntropy, kStrategyPredefined, kStrategyBruteForce
  };
  bool strategy_enable[kNumFilterStrategies] = {
    false, false, false, false, false, false, false, false, false
  };
  std::string strategy_name[kNumFilterStrategies] = {
    "zero", "one", "two", "three", "four",
    "minimum sum", "entropy", "predefined", "brute force"
  };
  for (size_t i = 0; i < png_options.filter_strategies.size(); i++) {
    strategy_enable[png_options.filter_strategies[i]] = true;
  }


  std::vector<unsigned char> image;
  unsigned w, h;
  unsigned error;
  lodepng::State inputstate;
  error = lodepng::decode(image, w, h, inputstate, origpng);

  if (error) {
    if (verbose) {
      printf("Decoding error %u: %s\n", error, lodepng_error_text(error));
    }
    return error;
  }

  bool bit16 = false;  // Using 16-bit per channel raw image
  if (inputstate.info_png.color.bitdepth == 16 && !png_options.lossy_8bit) {
    // Decode as 16-bit
    image.clear();
    error = lodepng::decode(image, w, h, origpng, LCT_RGBA, 16);
    bit16 = true;
  }

  if (!error) {
    // If lossy_transparent, remove RGB information from pixels with alpha=0
    if (png_options.lossy_transparent && !bit16) {
      LossyOptimizeTransparent(&image[0], w, h);
    }

    if (png_options.auto_filter_strategy) {
      error = AutoChooseFilterStrategy(image, w, h, inputstate, bit16,
                                       origpng,
                                       /* Don't try brute force */
                                       kNumFilterStrategies - 1,
                                       filterstrategies, strategy_enable);
    }
  }

  if (!error) {

	size_t bestsize = 0;
    for (int i = 0; i < kNumFilterStrategies; i++) {
      if (!strategy_enable[i]) continue;

      std::vector<unsigned char> temp;
      error = TryOptimize(image, w, h, inputstate, bit16, origpng,
                          filterstrategies[i], true /* use_zopfli */,
                          windowsize, &png_options, &temp);
      if (!error) {
        if (verbose) {
          printf("Filter strategy %s: %d bytes\n",
                 strategy_name[i].c_str(), (int) temp.size());
        }
        if (bestsize == 0 || temp.size() < bestsize) {
			bestsize = temp.size();
          (*resultpng).swap(temp);  // Store best result so far in the output.
        }
      }
    }

    if (!png_options.keepchunks.empty()) {
      KeepChunks(origpng, png_options.keepchunks, resultpng);
    }
  }

  return error;
}
