# DS 289 Assignment 3: Burgers' equation and finite elements

Question sheet: [question.pdf](question.pdf). Questions 1 and 4 are solved in code.

```
cd Q1_burgers_equation && make && ./run.sh
cd Q4_fem_bar_deflection && ./run.sh
```

## Viscous Burgers' equation (Q1)

`u_t + u u_x = alpha u_xx` on a periodic domain of length 1, dt = 0.0004, end time 0.075,
`u(x,0) = sin(4 pi x) + sin(6 pi x) + sin(10 pi x)`. `./burgersEquation <points> <alpha> <scheme>`:

- scheme 0: explicit Euler, first order upwind on the conservative form `u_t + (u^2/2)_x = 0`
- scheme 1: explicit Euler, second order central differences
- scheme 2: explicit Euler, first order upwind on the non-conservative form `u u_x`, for comparison

`run.sh` runs (a) upwind with alpha = 0 on 64 and 1024 points, (b) central differences with
alpha = 0.001 on 1024 points, plus scheme 2, and plots all of them in `output/burgers.png`.

| Case | First shock at | Largest u | Integral of u |
|---|---|---|---|
| upwind, N = 64, alpha = 0 | 0.2109 | 1.77 | -8.8e-17 |
| upwind, N = 1024, alpha = 0 | 0.2104 | 2.21 | 3.2e-17 |
| central, N = 1024, alpha = 0.001 | 0.2104 | 2.21 | -1.1e-16 |
| non-conservative upwind, N = 1024 | 0.1665 | 1.78 | 1.1e-3 |

A reference solution (4096 points, conservative Lax-Friedrichs flux, time step 2e-5) puts the shock at
0.2103. On 64 points the upwind scheme smears the shock and loses peak height (1.77 against 2.21), the
numerical diffusion of a first order scheme. The non-conservative form keeps the shocks at the zeros of
the initial condition. The largest `u dt/dx` is 1.05 on the fine grid at t = 0, at the stability limit.

## Deflection of a bar by finite elements (Q4)

`Ac E u'' = P`, u = 0 at both ends, Ac = 0.1 m^2, E = 200e9 N/m^2, L = 10 m, P = 100 N/m, 20 linear
elements of 0.5 m (`src/Question_Four.py`). The largest deflection is -6.25e-8 m at x = 5 m, equal to
the exact solution `u = P x (x - L) / (2 Ac E)`; the difference at the 21 nodes is below 1e-22 m.
