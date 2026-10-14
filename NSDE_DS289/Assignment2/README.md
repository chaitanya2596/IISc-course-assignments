# DS 289 Assignment 2: elliptic and parabolic equations

Question sheet: [question.pdf](question.pdf). Questions 1, 4 and 5 are solved in code.

```
cd Q4_diffusion_explicit_euler     # or any other question folder
make
./run.sh
```

## Steady heat conduction in a plate (Q1)

`T_xx + T_yy = 0` on `2 <= x <= 3`, `4 <= y <= 6` with T = 30 and 60 on the left and right edges,
`dT/dy = 0` at the bottom and `dT/dy = T - 60` at the top, second-order central differences on a
128 x 256 grid. The unknowns are numbered row by row, which gives a banded matrix (half band width 128);
it is stored in band form and solved directly by banded Gaussian elimination. The derivative boundary
conditions use ghost points. `src/plot.py` draws the contour plot `output/contour_128x256.png`.

Result: temperature between 27.69 and 60, largest residual `|A T - b|` = 4.5e-13, run time 0.35 s.

## Diffusion equation, explicit Euler (Q4)

`u_t = 1.5 u_xx`, `u(x,0) = sin(4x) + sin(x)`, periodic domain of length 2 pi, constant
`rd = alpha dt / dx^2`, N = 32 to 256, end time 0.4, average absolute error E(N) against
`u = exp(-1.5 t) sin(x) + exp(-24 t) sin(4x)`. A short final step lands exactly on t = 0.4; the error
is also reported at the last full step.

| N | E(N), rd = 1/2 | order | E(N), rd = 1/6, last full step | order | E(N), rd = 1/6, t = 0.4 | order |
|---|---|---|---|---|---|---|
| 32 | 1.35e-3 | | 6.18e-7 | | 2.27e-6 | |
| 64 | 3.36e-4 | 2.00 | 3.82e-8 | 4.02 | 1.49e-7 | 3.93 |
| 128 | 8.42e-5 | 2.00 | 2.38e-9 | 4.00 | 3.42e-9 | 5.44 |
| 256 | 2.10e-5 | 2.00 | 1.49e-10 | 4.00 | 3.85e-10 | 3.15 |

rd = 1/2 gives second order. rd = 1/6 gives fourth order, because the leading time and space errors
cancel at this value. The short final step (not at rd = 1/6) makes the rates in the last column uneven.

## Diffusion equation, implicit Euler with Jacobi (Q5)

Same problem with implicit Euler; the periodic tridiagonal system of each step is solved with Jacobi
iterations starting from the previous time level. `run.sh` uses tolerance 1e-4 (as in the question) and 1e-10.

| N | E(N), rd = 1/2, tol 1e-4 | E(N), rd = 1/2, tol 1e-10 | order | explicit, rd = 1/2 |
|---|---|---|---|---|
| 32 | 3.63e-3 | 2.65e-3 | | 1.35e-3 |
| 64 | 4.23e-3 | 6.70e-4 | 1.98 | 3.36e-4 |
| 128 | 1.39e-2 | 1.68e-4 | 1.99 | 8.42e-5 |
| 256 | 5.03e-2 | 4.21e-5 | 2.00 | 2.10e-5 |

With a tight tolerance the implicit scheme is second order and its error is twice the explicit one,
as the truncation errors predict (`rd/2 + 1/12 = 1/3` against `|1/12 - rd/2| = 1/6`). With tolerance 1e-4
the error grows with N: the change per time step shrinks like 1/N^2, so Jacobi stops after two or three
sweeps per step (5,243 sweeps for 1,992 steps at N = 256) and the remaining error accumulates.
