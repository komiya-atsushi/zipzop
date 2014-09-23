PREFIX =

CC = $(PREFIX)gcc
OPT_CFLAGS = 
CFLAGS = -c -O2 -W -Wall -Wextra $(OPT_CFLAGS)

LD = $(PREFIX)gcc
OPT_LDFLAGS = 
LDFLAGS = -lz -lm $(OPT_LDFLAGS)

ZIPZOP_OBJS = header.o recompress.o zz_util.o zipzop.o
ZOPFLI_OBJS = blocksplitter.o cache.o deflate.o hash.o katajainen.o lz77.o squeeze.o tree.o util.o zopfli_lib.o zlib_container.o gzip_container.o
OBJS = $(ZOPFLI_OBJS) $(ZIPZOP_OBJS)

E = 
TARGET = zipzop$(E)

$(TARGET): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(TARGET)
	strip $(TARGET)

header.o: src/header.c
	$(CC) $< -I./zopfli/src/zopfli $(CFLAGS)
recompress.o: src/recompress.c zopfli/src/zopfli/deflate.h
	$(CC) $< -I./zopfli/src/zopfli $(CFLAGS)
zz_util.o: src/zz_util.c
	$(CC) $< -I./zopfli/src/zopfli $(CFLAGS)
zipzop.o: src/zipzop.c
	$(CC) $< -I./zopfli/src/zopfli $(CFLAGS)

blocksplitter.o: zopfli/src/zopfli/blocksplitter.c
	$(CC) $< $(CFLAGS)
cache.o: zopfli/src/zopfli/cache.c
	$(CC) $< $(CFLAGS)
deflate.o: zopfli/src/zopfli/deflate.c
	$(CC) $< $(CFLAGS)
hash.o: zopfli/src/zopfli/hash.c
	$(CC) $< $(CFLAGS)
katajainen.o: zopfli/src/zopfli/katajainen.c
	$(CC) $< $(CFLAGS)
lz77.o: zopfli/src/zopfli/lz77.c
	$(CC) $< $(CFLAGS)
squeeze.o: zopfli/src/zopfli/squeeze.c
	$(CC) $< $(CFLAGS)
tree.o: zopfli/src/zopfli/tree.c
	$(CC) $< $(CFLAGS)
util.o: zopfli/src/zopfli/util.c
	$(CC) $< $(CFLAGS)
zopfli_lib.o: zopfli/src/zopfli/zopfli_lib.c
	$(CC) $< $(CFLAGS)
zlib_container.o: zopfli/src/zopfli/zlib_container.c
	$(CC) $< $(CFLAGS)
gzip_container.o: zopfli/src/zopfli/gzip_container.c
	$(CC) $< $(CFLAGS)

zopfli/src/zopfli/blocksplitter.c: zopfli
zopfli/src/zopfli/cache.c: zopfli
zopfli/src/zopfli/deflate.c: zopfli
zopfli/src/zopfli/deflate.h: zopfli
zopfli/src/zopfli/hash.c: zopfli
zopfli/src/zopfli/katajainen.c: zopfli
zopfli/src/zopfli/lz77.c: zopfli
zopfli/src/zopfli/squeeze.c: zopfli
zopfli/src/zopfli/tree.c: zopfli
zopfli/src/zopfli/util.c: zopfli
zopfli/src/zopfli/zopfli_lib.c: zopfli
zopfli/src/zopfli/zlib_container.c: zopfli
zopfli/src/zopfli/gzip_container.c: zopfli

zopfli:
	git clone https://code.google.com/p/zopfli/

clean:
	rm -rf zopfli *.o $(TARGET)

