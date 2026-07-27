# SIGMOD 2025: Join Pipeline Optimization

This repository documents the evolution of a SIGMOD 2025 contest solution from a baseline hash-join implementation into a staged set of optimizations for scans, joins, materialization, and parallel execution. The goal is to make the project readable as a portfolio piece: what was built, why it was built, and where each step lives in the codebase.

## Project Story

The work developed in three clear phases:

1. Baseline join variants.
	We started with different hash table designs for the join operator: Robin Hood, Cuckoo, and Hopscotch hashing.
2. Data-movement optimizations.
	We reduced unnecessary copying with late materialization, a column-store representation, and an unchained hash table for better duplicate handling.
3. Parallel execution and final tuning.
	We then optimized the final executor with indexing for dense columns, parallel hash-table build, parallel probing, and work stealing.

The final implementation is the one in [src/execute.cpp](src/execute.cpp), while the intermediate versions are preserved in [archives/](archives) as a documented development trail.

## What To Read First

If you are opening this repository for the first time, this is the best order:

1. [README.md](README.md) for the overall narrative and results.
2. [archives/README_d1.md](archives/README_d1.md) for the first join-optimization stage.
3. [archives/README_d2.md](archives/README_d2.md) for the second optimization stage.
4. The companion documentation site for the final parallelized executor narrative.
5. [src/execute.cpp](src/execute.cpp) for the final code path.

## Implementation Map

The repository is organized around the techniques that were added over time, not around assignment paperwork.

### Stage 1: Hash-Join Variants

This stage explored alternative hash tables and the corresponding join executor.

- [archives/execute_initial.cpp](archives/execute_initial.cpp) shows the baseline join logic.
- [archives/execute_rob.cpp](archives/execute_rob.cpp), [archives/execute_cuc.cpp](archives/execute_cuc.cpp), and [archives/execute_hop.cpp](archives/execute_hop.cpp) contain the Robin Hood, Cuckoo, and Hopscotch versions.
- [archives/modules/](archives/modules) contains the standalone hash table prototypes used during that phase.
- [tests/hash_tests.cpp](tests/hash_tests.cpp) and the hash-related unit tests document the expected behavior.

### Stage 2: Late Materialization and Column Store

This stage reduced data movement and changed how intermediate results are represented.

- [archives/execute_d1.cpp](archives/execute_d1.cpp) captures the data-locality and materialization-oriented version.
- [archives/execute_initial_optimized.cpp](archives/execute_initial_optimized.cpp) and [archives/unchained_table.cpp](archives/unchained_table.cpp) show the unchained-table path.
- [tests/opt_tests.cpp](tests/opt_tests.cpp) documents the intended behavior of late materialization, column storage, and the unchained hash table.

### Stage 3: Parallel Build, Parallel Probe, and Work Stealing

This is the final version that aims to scale on multi-core hardware.

- [src/execute.cpp](src/execute.cpp) contains the final executor.
- The build phase is parallelized in the threaded table path.
- The probe phase uses chunked work distribution and work stealing.
- [tests/paral_tests.cpp](tests/paral_tests.cpp) covers the parallel execution behavior.

## Final Executor

The final implementation in [src/execute.cpp](src/execute.cpp) follows this structure:

- It executes scan nodes with copy-scan behavior into the late-materialized format.
- It chooses the build side dynamically based on cardinality.
- It uses the unchained hash table for the unthreaded path.
- It switches to a threaded build/probe pipeline when the input is large enough.
- It uses environment variables to tune thread counts and the build threshold.

Relevant tuning knobs:

```bash
SPC_FORCE_THREADS="value" SPC_THREADED_MIN_BUILD="value" ./build/fast plans.json
```

- `SPC_FORCE_THREADS` overrides the default thread count.
- `SPC_THREADED_MIN_BUILD` controls when the threaded build path is enabled.

## Results

The documented progression is also reflected in the performance numbers. On the benchmark system used for evaluation, the average runtime improved from the baseline to the final version as follows:

| Stage | Avg Time (ms) |
| --- | ---: |
| Base solution | 169242 |
| Indexing | 13017 |
| Building parallelization | 12919 |
| Probing parallelization | 7798 |
| Work stealing | 6208 |

The full stage-by-stage discussion and supporting tables are preserved in the archive README files and the companion documentation site.

## How To Run

Run these commands from the project root.

```bash
./download_imdb.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build -- -j $(nproc)
./build/build_database imdb.db
./build/run plans.json
```

If you prefer `Ninja Multi-Config`, use:

```bash
cmake -S . -B build -Wno-dev -G "Ninja Multi-Config"
cmake --build build --config Release -- -j $(nproc)
./build/Release/build_database imdb.db
./build/Release/run plans.json
```

For cache-based execution on UNIX-like systems:

```bash
./build/build_cache plans.json
./build/fast plans.json
```

## Environment

- Code is compiled with Clang 18.
- Performance tests were run on an 11th Gen Intel Core i5-11400F with 16 GB RAM in a virtual machine using the available host resources.

## Companion Documentation

The project is also documented as a small website:

https://encodedmind.github.io/sigmod25-documentation/

That site follows the same progression as this repository: problem statement, optimization stages, and performance results.