gcovh
=====

gcovの出力ファイル(*.gcov)をhtmlにコンバートする簡易版lcov.
Windowsでも動きます。

使い方
-----------

あらかじめgcovで.gcovファイルを生成しておき、生成した.gcovを引数に与えます。

`./gcov foo.c bar.c'
`./gcovh foo.c.gcov bar.c.gcov`

ビルド
-----------
`make`


