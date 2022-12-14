# wdnet 0.0.6

+ Updated function `rpanet`.
  + Renamed `seednetwork` to `initial.network` and changed `seednetwork = NULL`
  to `initial.network = list(edgelist = matrix(c(1, 2), nrow = 1))`;
  + Changed `control = NULL` to `control = list()`;
  + Renamed `naive` to `linear`; `nodelist` to `bag`; `edgesampler` to `bagx`;
  + Updated returns, put node strength and preference scores into a data frame.

+ Sort nodes from the seed network according to their preference scores before
  the sampling process.
+ Renamed `rpanet` control functions: `rpactl.foo()` to  `rpa_control_foo()`.
+ Renamed `cvxr.control()` to `cvxr_control()`.

# wdnet 0.0.5

## Minor changes

+ Updated returned items from `dprewire` and `dprewire.range`.
  + Removed solved `eta` and corresponding assortativity levels.
  + Added solver results from `CVXR`.
+ Allowed user-defined preference functions in `rpactl.preference`.

# wdnet 0.0.4

## Minor changes

+ Renamed internal C++ functions.
+ Edited `rpanet` with `binary` approach: renamed node structures (fix LTO issues).


# wdnet 0.0.3

+ First version of wdnet submitted to CRAN.
