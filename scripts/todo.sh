#!/usr/bin/bash
set -e
#set -x

rm -f graph.txt graph.weights graph.bin
rm -f level2 level3 level4 level5 part

./mytest.py $1
./comml-convert -i graph.txt -o graph.bin -w graph.weights
# OLD ./comml-louvain  graph.bin -w graph.weights -l -1 -e 0.001 -v > graph.tree
#./comml-louvain  graph.bin -w graph.weights -l -1 -v > graph.tree
./example > graph.tree 2> out2
cat out2
lastlevel=`grep level out2 | tail -n 1  | awk '{print $2}' | sed "s/://"`
echo "Last level: $lastlevel"
./comml-hierarchy graph.tree -l $lastlevel > level_${lastlevel}

echo "Python found components:"
cat part | awk '{print $2}' | sort -n | tail -n 1

echo "C++ found commponents:"
cat level_$lastlevel | awk '{print $2}' | sort -n | tail -n 1

echo "Unweighted C++ found components:"
./louvain graph_unw.txt out_unw > out_unw
cat out_unw | awk '{print $2}' | sort -n | tail -n 1
