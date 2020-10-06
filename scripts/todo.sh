#!/usr/bin/bash
set -e
set -x

rm -f graph.txt graph.weights graph.bin

./mywork.py $1
./convert -i graph.txt -o graph.bin -w graph.weights
./louvain  graph.bin -w graph.weights -v -e 0.001 -l 100
