#include <stdio.h>
#include <stdlib.h>

#include "zipzop.h"

#define IS_LOCAL_FILE_HEADER(sig) ((((sig) >> 16) & 0xffff) == 0x0403)
#define IS_CENTRAL_DIRECTORY_FILE_HEADER(sig) ((((sig) >> 16) & 0xffff) == 0x0201)
#define IS_END_RECORD(sig) ((((sig) >> 16) & 0xffff) == 0x0605)

typedef struct {
  u32 comp_size;
  u32 uncomp_size;
  u32 offset;
} CompressionResult;

void read_header(FILE *infile, Header *header) {
  header->signature = read_u32(infile);
  if ((header->signature & 0xffff) != 0x4b50) {
    puts("ERROR: Invalid zip format.");
    exit(1);
  }

  if (IS_LOCAL_FILE_HEADER(header->signature)) {
    read_local_file_header(infile, header);

  } else if (IS_CENTRAL_DIRECTORY_FILE_HEADER(header->signature)) {
    read_central_directory_file_header(infile, header);

  } else if (IS_END_RECORD(header->signature)) {
    read_end_record(infile, header);
  }
}

void recompress_zip(FILE *infile, FILE *outfile, int num_iterations) {
  Header *header = (Header *)allocate_or_exit(65536 * 3 + 256);
  CompressionResult *comp_result
    = (CompressionResult *)allocate_or_exit(sizeof(CompressionResult) * 65537);

  int file_count = 0;
  int central_dir_count = 0;

  size_t central_dir_offset = 0xffffffff;

  while (file_count < 65537) {
    read_header(infile, header);

    if (IS_LOCAL_FILE_HEADER(header->signature)) {
      LocalFileHeader *_header = (LocalFileHeader *)header;
      comp_result[file_count].offset = ftell(outfile);
      
      recompress_entry(infile, outfile, _header, num_iterations);

      comp_result[file_count].comp_size = _header->comp_size;
      comp_result[file_count].uncomp_size = _header->uncomp_size;
      file_count++;

    } else if (IS_CENTRAL_DIRECTORY_FILE_HEADER(header->signature)) {
      if (central_dir_offset == 0xffffffff) {
	central_dir_offset = ftell(outfile);
      }
      CentralDirectoryFileHeader *_header = (CentralDirectoryFileHeader *)header;

      _header->comp_size = comp_result[central_dir_count].comp_size;
      _header->uncomp_size = comp_result[central_dir_count].uncomp_size;
      _header->offset = comp_result[central_dir_count].offset;
      central_dir_count++;

      write_central_dicrectory_file_header(outfile, _header);

    } else if (IS_END_RECORD(header->signature)) {
      CentralDirectoryEndRecord *record = (CentralDirectoryEndRecord *)header;

      record->offset = central_dir_offset;
      write_end_record(outfile, record);

      break;
    }
  }

  free(header);
  free(comp_result);
}

void show_result_size(FILE *infile, FILE *outfile) {
  size_t orig = ftell(infile);
  size_t recomp = ftell(outfile);

  u32 diff = (int)recomp - orig;
  printf("\noriginal -> recompressed\n\t%d -> %d bytes (%ld bytes)\n", orig, recomp, diff);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    puts("usage: zipzop NUM_ITERATIONS IN_FILE OUT_FILE");
    return 0;
  }

  int num_iterations = atoi(argv[1]);
  if (num_iterations <= 0) {
    printf("ERROR: Invalid iteration count: %s\n", argv[1]);
  }

  FILE *infile = fopen(argv[2], "rb");
  if (infile == NULL) {
    printf("ERROR: Cannot open input file: %s\n", argv[2]);
    return 1;
  }

  FILE *outfile = fopen(argv[3], "wb");
  if (outfile == NULL) {
    printf("ERROR: Cannot open output file: %s\n", argv[3]);
    return 1;
  }

  recompress_zip(infile, outfile, num_iterations);
  show_result_size(infile, outfile);

  fclose(infile);
  fclose(outfile);

  return 0;
}
