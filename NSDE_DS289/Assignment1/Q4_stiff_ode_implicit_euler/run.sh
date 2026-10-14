#!/bin/bash
# Implicit Euler for the stiff ODE y' = -200000 y + 199999 exp(-x), y(0)=0
mkdir -p output
./solution > /dev/null
python3 src/check.py
