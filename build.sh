#!/bin/sh

make -C algo/vendor/seemmo -j -B -f Makefile_server
make -C algo/vendor/seemmo -j -B -f Makefile_stub
make -C algo/vendor/seemmo -j -B -f Makefile_wrapper
make -C algo/stub -j -B
make -C vfilter -j -B


