# EasyBC 2-subset division-property experiments

This directory holds the experiment harness for the 2-subset bit-based
division property MILP integral search implemented in `Div2SetMILP`.

The pipeline is intentionally minimal: the C++ code only emits structured
`[BENCH]` lines on stderr, and Python scripts here orchestrate runs,
parse those lines, and produce CSVs / LaTeX tables.

## Layout

```
experiments/
├── README.md              (this file)
├── configs/
│   ├── correctness.yaml   Table 1 — paper-result reproduction matrix
│   ├── perf.yaml          Table 2 — phase-by-phase timing matrix (multi-thread sweep)
│   └── reduction.yaml     Table 3 — S-box reduction-method ablation
├── bench.py               Matrix runner — invokes ./EasyBC, parses stderr, writes CSV
├── parse_log.py           Single-log inspector — JSON or human-readable summary
├── make_tables.py         CSV → LaTeX tables + matplotlib plots
├── golden/                Ground-truth balanced-bit JSON per config (Table 1 reference)
├── results/               CSV outputs + per-run logs (gitignore-able)
└── tables/                Rendered LaTeX / PNG outputs
```

## Prerequisites

- Build EasyBC (Gurobi 12.0.2 set up per repo CMakeLists):
  ```
  cmake --build cmake-build-debug-2080 --target EasyBC -- -j
  ```
- Python 3.9+ with PyYAML (`pip install pyyaml`); matplotlib only required for plots.

## Quick start

```bash
# (from repo root)
cd experiments

# 1. Smoke-test correctness (one trial per config)
python bench.py --config configs/correctness.yaml

# 2. Inspect the result of a specific run
python parse_log.py results/logs/PRESENT_R9_60_m1_t8_trial1_*.log --with-balanced

# 3. Once golden/ is populated, render Table 1
python make_tables.py --table 1 results/correctness_*.csv > tables/table1.tex

# 4. Full performance sweep (5 trials × multiple thread counts)
python bench.py --config configs/perf.yaml
python make_tables.py --table 2 results/perf_*.csv        > tables/table2.tex
python make_tables.py --plot scaling results/perf_*.csv

# 5. Reduction-method ablation
python bench.py --config configs/reduction.yaml
python make_tables.py --table 3 results/reduction_*.csv   > tables/table3.tex
```

## Structured log format

Each EasyBC run emits one or more of the following lines on stderr:

```
[BENCH] config cipher=<C> reduction=<M> rounds=<R> activebits=<A>
[BENCH] phase=trail      sbox=<S> elapsed_ms=<ms> n_trails=<N>
[BENCH] phase=ineq_gen   cipher=<C> sbox=<S> elapsed_ms=<ms> n_ineq=<N>
[BENCH] phase=reduce     cipher=<C> sbox=<S> method=<M> elapsed_ms=<ms> n_ineq_before=<B> n_ineq_after=<A>
[BENCH] phase=preprocess cipher=<C> elapsed_ms=<ms> n_ineq_loaded=<N>
[BENCH] phase=build      cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> n_xvars=<V> n_dvars=<D> block_size=<B>
[BENCH] phase=model_load cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> n_vars=<V> n_cons=<C>
[BENCH] phase=solve      cipher=<C> rounds=<R> activebits=<A> elapsed_ms=<ms> total_ms=<ms> gurobi_status=<S> distinguisher_found=<0|1> n_zero_coords=<N> block_size=<B> n_iter=<N> threads=<T>
```

Multiple S-boxes (e.g. LBlock has 10) ⇒ multiple `trail` / `ineq_gen` /
`reduce` lines per run; `bench.py` aggregates them in the CSV's
`*_total` columns and `parse_log.py` keeps them broken out by S-box.

## Methodology notes (matches the plan)

- Repeat each cell 3–5 times; CSV records every trial; `make_tables.py`
  computes median + (min, max).
- Gurobi seed defaults to 0 (default in Gurobi 12 is also 0; the seed-
  variance experiment is a separate ad-hoc run).
- Single-thread vs 8-thread numbers come from `threads_sweep` in
  `perf.yaml`; bench.py runs each cell at every listed thread count.
- Configurations that exceed `timer_sec` are tagged in the CSV
  (`gurobi_status` ≠ 2 ≠ 3). Mark with † when reporting.
- Hardware specs and Gurobi version belong in the paper's evaluation
  intro — record them outside this harness.

## Comparison against the Python reference

To compare with `MILP_Division_Property-master` on the same machine:

```bash
# Run the same configs through the reference (outside this repo)
# then merge its CSV with bench.py's CSV (matching cipher/rounds/activebits)
# and let make_tables.py emit the speedup column.
```

`make_tables.py` does not currently auto-join; merge with `pandas` or by
hand for now. The CSV columns are stable so a `pandas.merge` on
`(cipher, rounds, activebits, threads)` will work.

## CI integration

The `correctness.yaml` matrix doubles as a regression suite. After a
clean reproduction, populate `golden/<key>.json` and add to CI:

```bash
python bench.py --config configs/correctness.yaml --repeat 1
python make_tables.py --table 1 results/correctness_*.csv | grep -q '\\times' && exit 1
exit 0
```

(Any mismatched balanced-bit set surfaces as `\times` in the LaTeX.)
