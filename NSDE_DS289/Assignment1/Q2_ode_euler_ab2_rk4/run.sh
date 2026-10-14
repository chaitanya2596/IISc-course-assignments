#!/bin/bash
# Runs all three schemes (0 = explicit Euler, 1 = RK4, 2 = Adams-Bashforth 2)
mkdir -p output
for s in 0 1 2; do ./numericalSolver $s > /dev/null; done
python3 src/order.py
