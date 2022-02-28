PREFIX =

ZOPFLI_DIR = zopfli

CC = $(PREFIX)gcc
OPT_CFLAGS = -O2
DEBUG =
WARN = -W -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
CFLAGS = -c $(DEBUG) $(OPT_CFLAGS) $(WARN) $(DEFINES)
INCLUDES = -I$(ZOPFLI_DIR)/src/zopfli

LD = $(PREFIX)gcc
OPT_LDFLAGS =
LDFLAGS = $(OPT_LDFLAGS)

ZIPZOP_OBJS = header.o recompress.o zz_util.o zipzop.o
ZOPFLI_OBJS = $(ZOPFLI_DIR)/libzopfli.a
#OBJS = $(ZOPFLI_OBJS) $(ZIPZOP_OBJS)
OBJS = $(ZIPZOP_OBJS)
LIBS = -lz -lm -L$(ZOPFLI_DIR) -lzopfli

E =
TARGET = zipzop$(E)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
#	strip $(TARGET)

header.o: src/header.c
	$(CC) $< $(CFLAGS) $(INCLUDES)
recompress.o: src/recompress.c $(ZOPFLI_DIR)/src/zopfli/deflate.h
	$(CC) $< $(CFLAGS) $(INCLUDES)
zz_util.o: src/zz_util.c
	$(CC) $< $(CFLAGS) $(INCLUDES)
zipzop.o: src/zipzop.c
	$(CC) $< $(CFLAGS) $(INCLUDES)


zopfli: zopfli
	git clone https://github.com/google/zopfli
	make -C zopfli


clean:
	rm -rf *.o $(TARGET)

distclean:
	rm -Rf zopfli

