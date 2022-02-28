zipzop
======

Zip archive recompressor with Google's zopfli library.


概要
----

Google による deflate 圧縮アルゴリズムの圧縮率最適化実装 [zopfli](https://code.google.com/p/zopfli/) を利用して Zip アーカイブを圧縮し直し、アーカイブサイズの削減を試みるツールです。


必要なもの
----------

 - [zlib](http://www.zlib.net/)
   - deflate 圧縮されたアーカイブ内のファイルを復元するために利用しています。

 - git
   - zopfli のソースコードを取得する際に利用しています。


使い方
------

以下のコマンドを実行することで、実行可能なバイナリ `zipzop` が生成されます。

    $ git clone git://github.com/komiya-atsushi/zipzop.git
    $ make

`make install` みたいな気の利いたものは用意していませんので、適当に `zipzop` をお使いください。

    $ zipzop NUM_ITERATIONS IN_FILE OUT_FILE

 - `NUM_ITERATIONS` ... 圧縮最適化を頑張る回数です。1 以上の数値を指定します。大きな値を指定すればするほどちょっとずつ圧縮率が改善しますが、数値に比例して処理時間がかかるようになります。
 - `IN_FILE` ... 再圧縮したい Zip アーカイブを指定します。
 - `OUT_FILE` ... 再圧縮後の Zip アーカイブの名前を指定します。既存のファイルを上書きして出力しようとするのでご注意ください。


制限
----

 - 再圧縮ができるのは deflate 圧縮されたファイルのみです。deflate64 を含め、その他の圧縮メソッドにより圧縮されたファイルは再圧縮せずにそのままアーカイブに出力されます。
 - 暗号化された Zip アーカイブは処理できません。


ライセンス
----------

zlib / libpng ライセンスです。詳しくは license.txt をお読み下さい。


謝辞
----

このプログラムは以下のライブラリを利用しています。

 - zopfli ( Copyright 2011 Google Inc. All Rights Reserved. )
 - zlib ( Copyright 1995-2012 Jean-loup Gailly and Mark Adler. )

   