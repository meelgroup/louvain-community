#!/usr/bin/bash
set -e
set -x

rm -f graph.txt graph.weights graph.bin

./mytest.py $1
./comml-convert -i graph.txt -o graph.bin -w graph.weights
./comml-louvain  graph.bin -w graph.weights -v -e 0.001 -l 100
