#!/usr/bin/bash
set -e
set -x

rm -f graph.txt graph.weights graph.bin
rm -f level* part*

./mytest.py $1
./comml-convert -i graph.txt -o graph.bin -w graph.weights
# ./comml-louvain  graph.bin -w graph.weights -v -e 0.001 -l 100
./comml-louvain  graph.bin -l -1 -q 0 -w graph.weights -v > graph.tree
./comml-hierarchy graph.tree -l 2 > level2
./comml-hierarchy graph.tree -l 3 > level3
./comml-hierarchy graph.tree -l 4 > level4
./comml-hierarchy graph.tree -l 5 > level5

echo "Python found components:"
cat part | awk '{print $2}' | sort -n | tail -n 1

echo "C++ found commponents:"
cat level2 | awk '{print $2}' | sort -n | tail -n 1
cat level3 | awk '{print $2}' | sort -n | tail -n 1
cat level4 | awk '{print $2}' | sort -n | tail -n 1
cat level5 | awk '{print $2}' | sort -n | tail -n 1
