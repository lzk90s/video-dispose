#!/bin/sh

make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_server install
make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_stub install
make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_wrapper install
make -C algo/vendor/gosun -j$(nproc) -B -f Makefile_stub install
make -C algo/stub -j$(nproc) -B install
make -C vfilter -j$(nproc) -B install
make -C vfilter install_font


