#!/bin/bash
# RK4 for the spring-mass-damper system, for the three damping coefficients c = 5, 40, 200.
# For each c the step size is halved 5 times starting from the value in input/input.txt.
mkdir -p output
P=input/sytstemParameters.txt
for c in 200 40 5; do
    printf '#mass m =\n20\n#spring stiffness  K = \n20\n#Damping coeff C = \n%s\n' $c > $P
    ./springMassDamper > /dev/null
done
# accuracy check against the exact solution (written for the under-damped case, c = 5, which is left in the input file)
python3 src/check.py
