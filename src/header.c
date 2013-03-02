/*
 * Copyright (c) 2013 KOMIYA Atsushi
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

#include "zipzop.h"

void read_local_file_header(FILE *infile, Header *_header) {
  LocalFileHeader *header = (LocalFileHeader *)_header;

  header->ver_extract = read_u16(infile);
  header->bit_flag = read_u16(infile);
  header->method = read_u16(infile);
  read_bytes(header->not_used1, sizeof(header->not_used1), infile);
  header->crc32 = read_u32(infile);
  header->comp_size = read_u32(infile);
  header->uncomp_size = read_u32(infile);
  header->filename_len = read_u16(infile);
  header->extra_len = read_u16(infile);
  
  size_t ext_size = header->filename_len + header->extra_len;
  read_bytes(header->ext, ext_size, infile);
}

void write_local_file_header(FILE *outfile, LocalFileHeader *header) {
  write_u32(header->signature, outfile);
  write_u16(header->ver_extract, outfile);
  write_u16(header->bit_flag, outfile);
  write_u16(header->method, outfile);
  write_bytes(header->not_used1, sizeof(header->not_used1), outfile);
  write_u32(header->crc32, outfile);
  write_u32(header->comp_size, outfile);
  write_u32(header->uncomp_size, outfile);
  write_u16(header->filename_len, outfile);
  write_u16(header->extra_len, outfile);

  size_t ext_size = header->filename_len + header->extra_len;
  write_bytes(header->ext, ext_size, outfile);
}

void read_central_directory_file_header(FILE *infile, Header *_header) {
  CentralDirectoryFileHeader *header = (CentralDirectoryFileHeader *)_header;

  header->ver_made_by = read_u16(infile);
  header->ver_extract = read_u16(infile);
  header->bit_flag = read_u16(infile);
  header->method = read_u16(infile);
  read_bytes(header->not_used1, sizeof(header->not_used1), infile);
  header->crc32 = read_u32(infile);
  header->comp_size = read_u32(infile);
  header->uncomp_size = read_u32(infile);
  header->filename_len = read_u16(infile);
  header->extra_len = read_u16(infile);
  header->comment_len = read_u16(infile);
  read_bytes(header->not_used2, sizeof(header->not_used2), infile);
  header->offset = read_u32(infile);

  size_t ext_size = header->filename_len + header->extra_len + header->comment_len;
  read_bytes(header->ext, ext_size, infile);
}

void write_central_dicrectory_file_header(FILE *outfile, CentralDirectoryFileHeader *header) {
  write_u32(header->signature, outfile);
  write_u16(header->ver_made_by, outfile);
  write_u16(header->ver_extract, outfile);
  write_u16(header->bit_flag, outfile);
  write_u16(header->method, outfile);
  write_bytes(header->not_used1, sizeof(header->not_used1), outfile);
  write_u32(header->crc32, outfile);
  write_u32(header->comp_size, outfile);
  write_u32(header->uncomp_size, outfile);
  write_u16(header->filename_len, outfile);
  write_u16(header->extra_len, outfile);
  write_u16(header->comment_len, outfile);
  write_bytes(header->not_used2, sizeof(header->not_used2), outfile);
  write_u32(header->offset, outfile);

  size_t ext_size = header->filename_len + header->extra_len + header->comment_len;
  write_bytes(header->ext, ext_size, outfile);
}

void read_end_record(FILE *infile, Header *_header) {
  CentralDirectoryEndRecord *record = (CentralDirectoryEndRecord *)_header;
  record->num_disks = read_u16(infile);
  record->disk = read_u16(infile);
  record->num_records = read_u16(infile);
  record->total_records = read_u16(infile);
  record->central_dir_size = read_u32(infile);
  record->offset = read_u32(infile);
  record->comment_len = read_u16(infile);
  read_bytes(record->ext, record->comment_len, infile);
}

void write_end_record(FILE *outfile, CentralDirectoryEndRecord *record) {
  write_u32(record->signature, outfile);
  write_u16(record->num_disks, outfile);
  write_u16(record->disk, outfile);
  write_u16(record->num_records, outfile);
  write_u16(record->total_records, outfile);
  write_u32(record->central_dir_size, outfile);
  write_u32(record->offset, outfile);
  write_u16(record->comment_len, outfile);
  write_bytes(record->ext, record->comment_len, outfile);
}
