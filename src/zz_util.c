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
