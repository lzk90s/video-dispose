#!/bin/sh

make -C algo/seemmo/vendor install  -j
make -C algo/seemmo/service install  -j
make -C algo/seemmo/stub install -j
