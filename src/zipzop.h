#ifndef _zipzop_h
#define _zipzop_h

#include <stdio.h>

typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned char uchar;

typedef struct {
  u32 signature;
  char ext[1];
} Header;
  
typedef struct {
  u32 signature;
  u16 ver_extract;
  u16 bit_flag;
  u16 method;
  char not_used1[4];
  u32 crc32;
  u32 comp_size;
  u32 uncomp_size;
  u16 filename_len;
  u16 extra_len;
  char ext[1];
} LocalFileHeader;

typedef struct {
  u32 signature;
  u16 ver_made_by;
  u16 ver_extract;
  u16 bit_flag;
  u16 method;
  char not_used1[4];
  u32 crc32;
  u32 comp_size;
  u32 uncomp_size;
  u16 filename_len;
  u16 extra_len;
  u16 comment_len;
  char not_used2[8];
  u32 offset;
  char ext[1];
} CentralDirectoryFileHeader;

typedef struct {
  u32 signature;
  u16 num_disks;
  u16 disk;
  u16 num_records;
  u16 total_records;
  u32 central_dir_size;
  u32 offset;
  u16 comment_len;
  char ext[1];
} CentralDirectoryEndRecord;

/* header.c */
void read_local_file_header(FILE *infile, Header *_header);
void write_local_file_header(FILE *outfile, LocalFileHeader *header);

void read_central_directory_file_header(FILE *infile, Header *_header);
void write_central_dicrectory_file_header(FILE *outfile, CentralDirectoryFileHeader *header);

void read_end_record(FILE *infile, Header *_header);
void write_end_record(FILE *outfile, CentralDirectoryEndRecord *record);

/* util.c */
u32 read_u32(FILE *infile);
u16 read_u16(FILE *infile);
void read_bytes(void *buf, size_t size, FILE *infile);

void write_u32(u32 val, FILE *outfile);
void write_u16(u16 val, FILE *outfile);
void write_bytes(void *buf, size_t size, FILE *outfile);

void *allocate_or_exit(size_t size);

/* recompress.c */
void recompress_entry(FILE *infile,
		      FILE *outfile,
		      LocalFileHeader *header,
		      int num_iterations);

#endif
