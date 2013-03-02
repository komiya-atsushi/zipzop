#!/bin/sh

if [ ! -d zopfli ]; then
    echo "cloning..."
    git clone https://code.google.com/p/zopfli
fi

cd zopfli
gcc *.c -c -O2 -W -Wall -Wextra
rm zopfli.o
cp *.o deflate.h util.h ../

cd ../
gcc *.c *.o -O2 -Wall -Wextra -lm -lz -o zipzop
