#!/bin/bash
# Explicit Euler + central difference for the diffusion equation, N = 32, 64, 128, 256, at rd = 1/2 and rd = 1/6
mkdir -p output
./diffusion 0.5 | tail -5
./diffusion 0.16666666666666666 | tail -5
