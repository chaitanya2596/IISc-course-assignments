#!/bin/bash
# Implicit Euler + Jacobi for the diffusion equation.
# First with the tolerance asked in the question (1e-4), then with a tight tolerance (1e-10)
# to separate the error of the scheme from the error left by stopping Jacobi early.
mkdir -p output
./implicitDiffusion 1e-4  | grep -v "^$"
./implicitDiffusion 1e-10 | grep -v "^$"
