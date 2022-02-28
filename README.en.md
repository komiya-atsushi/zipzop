zipzop
======

Zip archive recompressor using Google's zopfli library.


Overview
----

Zipzop is recompression tool that uses Google's compression ratio optimization implementation of the deflate compression algorithm [zopfli](https://github.com/google/zopfli) recompress Zip archives and try to reduce the archive size.


Things necessary
----

 - [zlib](http://www.zlib.net/)
   - deflate is used to decompress files in compressed archives.

 - [git](https://git-scm.com/)
   - It is used to get the source code of [zopfli](https://github.com/google/zopfli).


How to use
----

The following command will generate an executable binary `zipzop`.

    $ git clone git: //github.com/komiya-atsushi/zipzop.git
    $ make

There are no nice thing like `make install`, so please use `zipzop` appropriately.

    $ zipzop NUM_ITERATIONS IN_FILE OUT_FILE

 - `NUM_ITERATIONS` ... The number of times to work hard on compression optimization. Specify a number greater than or equal to 1. The larger the value, the better the compression ratio will be, but the processing time will increase in proportion to the value.
 - `IN_FILE` ... Specify the Zip archive you want to recompress.
 - `OUT_FILE` ... Specifies the name of the Zip archive after recompression. Please note that it will overwrite the existing file and try to output it.


Limitations
----

 - Only deflate-compressed files can be recompressed. Files compressed by other compression methods, including deflate64, are output to the archive as is without being recompressed.
 - Cannot process encrypted Zip archives.


License
----

zlib / libpng license. Please read license.txt for details.


Acknowledgments
----

This program uses the following libraries:

 - zopfli (Copyright 2011 Google Inc. All Rights Reserved.)
 - zlib (Copyright 1995-2012 Jean-loup Gailly and Mark Adler.)

