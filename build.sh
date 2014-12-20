#!/bin/sh

./autogen.sh
./configure
make
cd debian && ./make_deb.sh
