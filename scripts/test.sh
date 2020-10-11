#!/usr/bin/bash

# Copyright (C) 2020 Mate Soos
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; version 2
# of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.


set -e
#set -x

rm -f graph.txt graph.weights graph.bin
rm -f level2 level3 level4 level5 part ../src/mygraph.cpp

./mytest.py $1
awk '{print "gplain.add_edge(" $1 ", " $2 ", " $3 "L);"}' graph.txt > ../src/mygraph.cpp
./comml-convert -i graph.txt -o graph.bin -w graph.weights
# OLD ./comml-louvain  graph.bin -w graph.weights -l -1 -e 0.001 -v > graph.tree
./comml-louvain  graph.bin -w graph.weights -l -1 -v > graph.tree
rm graph.bin graph.weights
make
./example > final_level_example 2> out2
./example-libuse > final_level_example2
cat out2
lastlevel=`grep level out2 | tail -n 1  | awk '{print $2}' | sed "s/://"`
echo "Last level: $lastlevel"
./comml-hierarchy graph.tree -l $lastlevel > level_${lastlevel}
set +e
ret=`diff final_level_example level_${lastlevel}`
if [ "$ret" == "" ]; then
    echo "OK. Match. Example and normal pipeline give the same result"
else
    echo "NO MATCH!!!! Plain example is wrong!!"
    exit -1
fi

ret=`diff final_level_example2 level_${lastlevel}`
if [ "$ret" == "" ]; then
    echo "OK. Match. Example-LIBUSE and normal pipeline give the same result"
else
    echo "NO MATCH!!!! LIBUSE is wrong!!"
    exit -1
fi
set -e

echo "Python found components:"
cat part | awk '{print $2}' | sort -n | tail -n 1

echo "C++ found commponents:"
cat level_$lastlevel | awk '{print $2}' | sort -n | tail -n 1

echo "Unweighted C++ found components:"
./louvain graph_unw.txt out_unw > out_unw
cat out_unw | awk '{print $2}' | sort -n | tail -n 1
