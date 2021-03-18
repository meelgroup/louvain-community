#!/bin/bash

set -e

rm -rf lib* Test* tests* include tests CM* cmake* comml* src-* lib
cmake -DENABLE_TESTING=OFF -DSTATICCOMPILE=ON ..
make -j6
