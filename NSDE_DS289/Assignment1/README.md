# DS 289 Assignment 1: ordinary differential equations

Question sheet: [question.pdf](question.pdf). Questions 2 to 5 are solved in code.

```
cd Q2_ode_euler_ab2_rk4      # or any other question folder
make
./run.sh
```

## Explicit Euler, Adams-Bashforth 2 and RK4 (Q2)

`dy/dt = y t^2 - 1.1 y`, `y(0) = 1`, solved with step sizes 0.5 to 0.03125 and compared with the
exact solution `y = exp(t^3/3 - 1.1 t)` on `0 <= t <= 2`. `run.sh` runs all three schemes and prints
the order of accuracy from the errors.

| Scheme | Average error, h = 0.5 | h = 0.03125 | Observed order | Expected |
|---|---|---|---|---|
| Explicit Euler | 3.82e-1 | 2.62e-2 | 0.99 | 1 |
| Adams-Bashforth 2 | 1.84e-1 | 1.37e-3 | 1.95 | 2 |
| RK4 | 2.10e-3 | 1.79e-8 | 4.16 | 4 |

## Damped spring-mass system with RK4 (Q3)

`m x'' + c x' + k x = 0`, m = 20 kg, k = 20 N/m, x(0) = 1 m, x'(0) = 0, `0 <= t <= 15 s`, for
c = 5, 40 and 200, each with step sizes from 1 s down to 0.03125 s. The system is written as two
first-order equations and integrated with RK4; the c = 5 case is compared with the exact solution.

| h (s) | 1 | 0.5 | 0.25 | 0.125 | 0.0625 | 0.03125 |
|---|---|---|---|---|---|---|
| max error (c = 5) | 2.60e-2 | 1.60e-3 | 9.82e-5 | 6.10e-6 | 3.79e-7 | 2.36e-8 |
| order | | 4.02 | 4.03 | 4.01 | 4.01 | 4.00 |

## Stiff equation with implicit Euler (Q4)

`dy/dx = -200000 y + 200000 exp(-x) - exp(-x)`, y(0) = 0, `0 <= x <= 2`. Explicit Euler is stable
only for `h < 2/200000 = 1e-5`. Implicit Euler with h = 0.1 is stable; the largest error against the
exact solution `y = exp(-x) - exp(-200000 x)` is 4.98e-5.

## Temperature in a circular rod with a heat source (Q5)

`T'' + T'/r + S = 0` on `0 <= r <= 1`, `T(1) = 1`, `dT/dr(0) = 0`, second-order central differences
on 1024 intervals, solved with the Thomas algorithm and compared with `T = 1 + S (1 - r^2) / 4`.

| S | 100 | 500 | 1000 | 1500 |
|---|---|---|---|---|
| T(0) computed | 25.9999 | 125.999 | 250.999 | 375.998 |
| T(0) exact | 26 | 126 | 251 | 376 |
| max error | 1.06e-4 | 5.30e-4 | 1.06e-3 | 1.59e-3 |

The largest S with peak temperature at most 100 is S = 396 (T(0) = 99.9996; S = 397 gives 100.25).
