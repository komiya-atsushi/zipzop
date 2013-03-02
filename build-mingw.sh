#!/bin/bash

if [ ! -d zlib ]; then
    git clone git://github.com/madler/zlib.git
fi

cd zlib
make clean -f win32/Makefile.gcc
make PREFIX=i586-mingw32msvc- -f win32/Makefile.gcc
cd ../

make PREFIX=i586-mingw32msvc- OPT_CFLAGS=-I./zlib OPT_LDFLAGS="-L./zlib -static" E=.exe
