#!/bin/bash

set -e

rm -rf lib* Test* tests* include tests CM* cmake* comml* src-* lib
cmake -DENABLE_TESTING=ON ..
make -j6
./test.sh s5378a_15_7.cnf

