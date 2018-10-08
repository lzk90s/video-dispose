#!/bin/sh

make -C algo/vendor/seemmo -j -B -f Makefile_server install
make -C algo/vendor/seemmo -j -B -f Makefile_stub install
make -C algo/vendor/seemmo -j -B -f Makefile_wrapper install
make -C algo/stub -j -B install
make -C vfilter -j -B install


