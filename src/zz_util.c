#include <stdio.h>
#include <stdlib.h>
#include "zipzop.h"

static void read_error_exit() {
  puts("ERROR: Input EOF reached");
  exit(1);
}

static void write_error_exit() {
  puts("ERROR: Cannot file output");
  exit(1);
}

static void put_byte(int val, FILE *outfile) {
  if (fputc(val, outfile) == EOF) {
    write_error_exit();
  }
}

u32 read_u32(FILE *infile) {
  int v1 = fgetc(infile);
  int v2 = fgetc(infile);
  int v3 = fgetc(infile);
  int v4 = fgetc(infile);
  if (v4 == EOF || v3 == EOF || v2 == EOF || v1 == EOF) {
    read_error_exit();
  }
  return (v4 << 24) | (v3 << 16) | (v2 << 8) | v1;
}

u16 read_u16(FILE *infile) {
  int v1 = fgetc(infile);
  int v2 = fgetc(infile);
  if (v2 == EOF || v1 == EOF) {
    read_error_exit();
  }
  return (u16)((v2 << 8) | v1);
}

void read_bytes(void *buf, size_t size, FILE *infile) {
  if (fread(buf, 1, size, infile) != size) {
    read_error_exit();
  }
}

void write_u32(u32 val, FILE *outfile) {
  put_byte(val & 0xff, outfile);
  put_byte((val >> 8) & 0xff, outfile);
  put_byte((val >> 16) & 0xff, outfile);
  put_byte((val >> 24) & 0xff, outfile);
}

void write_u16(u16 val, FILE *outfile) {
  put_byte(val & 0xff, outfile);
  put_byte((val >> 8) & 0xff, outfile);
}

void write_bytes(void *buf, size_t size, FILE *outfile) {
  if (fwrite(buf, 1, size, outfile) != size) {
    write_error_exit();
  }
}

void *allocate_or_exit(size_t size) {
  void *buf = malloc(size);

  if (buf == NULL) {
    puts("ERROR: Cannot allocate memory");
    exit(1);
  }

  return buf;
}
