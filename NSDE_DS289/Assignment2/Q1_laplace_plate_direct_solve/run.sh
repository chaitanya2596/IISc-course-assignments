#!/bin/bash
# Solves the plate on the grid given in input/input.txt (128 x 256 by default) and plots the contours.
# input/input_64x128.txt is a coarser grid; copy it over input/input.txt to run it.
mkdir -p output
./heat
python3 src/plot.py 128 256
