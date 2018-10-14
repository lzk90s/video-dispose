#!/bin/sh

make -C algo/vendor/seemmo -j$(nproc) -B -f Makefile_stub install
if [ $? -ne 0 ]; then
	exit -1
fi

make -C algo/vendor/gosun -j$(nproc) -B -f Makefile_stub install
if [ $? -ne 0 ]; then
	exit -1
fi

make -C algo/stub -j$(nproc) -B install
if [ $? -ne 0 ]; then
	exit -1
fi

make -C vfilter -j$(nproc) -B install
if [ $? -ne 0 ]; then
	exit -1
fi

make -C vfilter install_font
if [ $? -ne 0 ]; then
	exit -1
fi

