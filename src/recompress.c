#include <zlib.h>
#include "zipzop.h"
#include "deflate.h"

const int METHOD_DEFLATE = 8;

static void inflate_src(uchar *src, size_t src_size,
			uchar *dst, size_t dst_size) {
  z_stream zs = { .zalloc = Z_NULL, .zfree = Z_NULL, .opaque = Z_NULL };
  if (inflateInit2(&zs, -15) != Z_OK) {
    puts("ERROR: Cannot initialize inflate z_stream");
    exit(1);
  }

  zs.next_in = src;
  zs.avail_in = src_size;
  zs.next_out = dst;
  zs.avail_out = dst_size;

  while (zs.avail_out > 0 && zs.avail_in > 0) {
    int ret = inflate(&zs, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
      return;
    }

    if (ret != Z_OK) {
      printf("ERROR: Compressed file is corrupted (inflate() returns %d)\n", ret);
      exit(1);
    }
  }
}

static void zopfli_deflate(const uchar *src, size_t src_size,
			   uchar **dst_p, size_t *dst_size_p,
			   int num_iterations) {
  Options options;
  InitOptions(&options);
  options.numiterations = num_iterations;

  uchar b = 0;
  Deflate(&options, 2, 1,
	  src, src_size,
	  &b,
	  dst_p, dst_size_p);
}

static void copy_filename_suffix(const char *src, size_t src_size,
				 char *buf, size_t buf_size) {
  if (src_size <= buf_size) {
    strncpy(buf, src, src_size);
    
    if (src_size < buf_size) {
      buf[src_size] = '\0';
    }
    
    return;
  }

  buf[0] = buf[1] = buf[2] = '.';
  size_t copy_len = buf_size - 3;
  strncpy(buf + 3, src + src_size - copy_len, copy_len);
}

#define FILENAME_BUF_SIZE 24

void recompress_entry(FILE *infile,
		      FILE *outfile,
		      LocalFileHeader *header,
		      int num_iterations) {
  char filename[FILENAME_BUF_SIZE + 1];
  copy_filename_suffix(header->ext, header->filename_len, filename, FILENAME_BUF_SIZE);
  filename[FILENAME_BUF_SIZE] = '\0';

  printf("%-24s : %ld -> ", filename, header->comp_size);

  size_t src_size = header->comp_size;
  uchar *src = (uchar *)allocate_or_exit(src_size);
  read_bytes(src, src_size, infile);
  
  if (header->method != METHOD_DEFLATE) {
    write_local_file_header(outfile, header);
    write_bytes(src, src_size, outfile);
    free(src);
    return;
  }

  size_t uncomp_size = header->uncomp_size;
  uchar *tmp = (uchar *)allocate_or_exit(uncomp_size);
  inflate_src(src, src_size, tmp, uncomp_size);

  uchar *dst = NULL;
  size_t dst_size = 0;
  zopfli_deflate(tmp, uncomp_size, &dst, &dst_size, num_iterations);

  if (dst_size > src_size) {
    free(dst);
    dst = src;
    dst_size = src_size;
    src = NULL;
  }

  header->comp_size = dst_size;
  printf("%d\n", dst_size);

  write_local_file_header(outfile, header);
  write_bytes(dst, dst_size, outfile);

  free(dst);
  free(src);
}
