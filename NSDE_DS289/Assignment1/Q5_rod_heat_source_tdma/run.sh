#!/bin/bash
# Temperature profile in the rod for the four heat-source strengths, then a grid-refinement check
mkdir -p output
for S in 100 500 1000 1500; do ./tdma $S | grep "max error"; done
echo "grid refinement, S = 1500:"
for n in 65 129 257 513 1025; do ./tdma 1500 $n | grep "max error"; done
echo "largest S that keeps the peak temperature at or below 100 (exact: 1 + S/4 = 100 -> S = 396):"
for S in 395 396 397; do ./tdma $S | grep "max error"; done
./tdma 1500 > /dev/null
