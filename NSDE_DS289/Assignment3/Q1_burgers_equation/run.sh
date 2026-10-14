#!/bin/bash
# (a) Euler + upwind, alpha = 0, N = 64 and 1024     (b) Euler + CD2, alpha = 0.001, N = 1024
mkdir -p output
./burgersEquation 64 0 0
./burgersEquation 1024 0 0
./burgersEquation 1024 0.001 1
# upwind on the non-conservative form, for comparison of the shock position
./burgersEquation 1024 0 2
python3 src/plot.py
