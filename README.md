# SIGMOD 2025: Join Pipeline Optimization

This repository documents the evolution of a SIGMOD 2025 contest solution from a baseline hash-join implementation into a staged set of optimizations for scans, joins, materialization, and parallel execution.

## Project Story

The work developed in three clear phases, each one documented by the code preserved in [archives/](archives) and by the final executor in [src/execute.cpp](src/execute.cpp).

## Optimization 1: Hash-Join Variants

We implemented the join operator by developing three different versions of the Hash Join algorithm. In the baseline solution, the hash table uses `std::unordered_map` from the C++ STL. We implemented three alternative hash-based join strategies to improve performance:

1. Robin Hood Hashing
2. Cuckoo Hashing
3. Hopscotch Hashing

Relevant files:

- [archives/execute_initial.cpp](archives/execute_initial.cpp) shows the baseline join logic.
- [archives/execute_rob.cpp](archives/execute_rob.cpp), [archives/execute_cuc.cpp](archives/execute_cuc.cpp), and [archives/execute_hop.cpp](archives/execute_hop.cpp) contain the three join variants.
- [archives/modules/](archives/modules) contains the standalone hash table prototypes used during this stage.
- [tests/hash_tests.cpp](tests/hash_tests.cpp) documents the expected behavior of the hash-based join implementations.

## Optimization 2: Materialization and Data Layout

We implemented the join operator by developing three optimization techniques. In the baseline solution, strings are fully materialized and copied early during query execution, causing unnecessary data movement. Moreover, intermediate join results are currently materialized in a row-store format, preventing us from benefiting from column-store cache locality and performance. Last, the current hash join implementation does not handle skewed or duplicate-heavy data efficiently, limiting performance for selective joins. To address these issues, we implemented the following optimization strategies:

1.1. Late Materialization for strings  
1.2. Eliminating Intermediate Results at the Root Join  
2. Column Store  
3. Unchained Hashing  

Relevant files:

- [archives/execute_d1.cpp](archives/execute_d1.cpp) captures the data-locality and materialization-oriented version.
- [archives/execute_initial_optimized.cpp](archives/execute_initial_optimized.cpp) shows the optimized executor path used in that stage.
- [archives/unchained_table.cpp](archives/unchained_table.cpp) and [archives/unchained_table.h](archives/unchained_table.h) contain the unchained hash table implementation.
- [tests/opt_tests.cpp](tests/opt_tests.cpp) documents the expected behavior of late materialization, column storage, and the unchained hash table.

## Optimization 3: Parallel Execution and Final Tuning

we implemented an optimized join operator by developing an optimization technique and parallelizing critical execution paths. The baseline solution copies all columns including full INT32 pages, which significantly impacts performance. To address this and leverage multi-core architectures, we implemented the following optimization strategies:

1. Indexing: Reference for dense INT32 columns
2. Building Parallelization: Multi-threaded hash table construction
3. Probing Parallelization: Multi-threaded join probing
4. Work Stealing: Load balancing using atomic chunk scheduling

Relevant files:

- [src/execute.cpp](src/execute.cpp) contains the final executor.
- [include/threaded_table.h](include/threaded_table.h) and [src/threaded_table.cpp](src/threaded_table.cpp) support the parallel build path.
- [tests/paral_tests.cpp](tests/paral_tests.cpp) covers the parallel execution behavior.

The final implementation is the one in [src/execute.cpp](src/execute.cpp), while the intermediate versions are preserved in [archives/](archives) as a documented development trail.

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
| Optimization 1 | 173872 |
| Optimization 2 | 43311 |
| Optimization 3 | 6208 |

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