# SIGMOD 2025: Join Pipeline Optimization

This project is based on the [SIGMOD 2025 programming contest](https://sigmod-contest-2025.github.io). The task involves optimizing the join pipeline by implementing an efficient join algorithm to reduce execution time.

## Project Description 

In the second part of the assignment, we implemented the join operator by developing three optimization techniques. In the baseline solution, strings are fully materialized and copied early during query execution, causing unnecessary data movement. Moreover, intermediate join results are currently materialized in a row-store format, preventing us from benefiting from column-store cache locality and performance. Last, the current hash join implementation does not handle skewed or duplicate-heavy data efficiently, limiting performance for selective joins. To address these issues, we implemented the following optimization strategies:

1.1. Late Materialization for strings  
1.2. Eliminating Intermediate Results at the Root Join  
2. Column Store  
3. Unchained Hashing  

## Team Information

**Team Name:** CTRL+S our lives

| Name                 | Student ID     |       Academic email      | GitHub Username   |
| -------------------- | -------------- | ------------------------- |------------------ |
| Andreakis Dimitrios  | 1115202300008  | sdi2300008@di.uoa.gr      | EncodedMind       |
| Vasileiou Evaggelos  | 1115201900309  | sdi1900309@di.uoa.gr      | VangelisVas       |
| Kolokouras Apostolos | 1115202100259  | sdi2100259@di.uoa.gr      | TolisKlk          |

---

## File Structure

*\*Only the most essential files\**
```bash
k23a-2025-d1-ctrl-s-our-souls/
├── src/
│   ├── execute.cpp
│   ├── unchained_hashtable.cpp
├── include/
│   ├── unchained_hashtable.h
│   ├── execute_root.h
│   ├── value_t.h
│   ├── column_t.h
│   ├── mycopyscan.h
│   ├── mytocolumnar.h
├── tests/
│   └── opt_tests.cpp
├── job/
├── CMakeLists.txt
└── README.md
```

---

## How to Run

### Quick start

> [!TIP]
> Run all the following commands in the root directory of this project.

First, download the imdb dataset.

```bash
./download_imdb.sh
```

Second, build the project.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build -- -j $(nproc)
```

Third, prepare the DuckDB database for correctness checking.

```bash
./build/build_database imdb.db
```

Now, you can run the tests:
```bash
./build/run plans.json
```
> [!TIP]
> If you want to use `Ninja Multi-Config` as the generator. The commands will look like:
> 
>```bash
> cmake -S . -B build -Wno-dev -G "Ninja Multi-Config"
> cmake --build build --config Release -- -j $(nproc)
> ./build/Release/build_database imdb.db
> ./build/Release/run plans.json
> ```

### Cache

**This section is only for UNIX users** \
There are 2 new executables with this repository. They cache the join tables and
result of each query and mmap them for faster loading times and getting rid of duckdb.

To build the cache you need to run:
```bash
./build/build_cache plans.json
```

> [!TIP] 
> If you are using `Linux x86_64` you can download our prebuilt cache with:
> ```
> wget http://share.uoa.gr/protected/all-download/sigmod25/sigmod25_cache_x86.tar.gz
> ```
> If you are using `macOS arm64` you can download our prebuilt cache with:
> ```
> wget http://share.uoa.gr/protected/all-download/sigmod25/sigmod25_cache_arm.tar.gz
> ```
> For all other systems you will need to build the cache on your own.

After the cache is built you can run the queries using:
```bash
./build/fast plans.json
```

Also after you have built the cache you no longer need to build the `run` executable
every time (which depends on duckdb and can be slow to compile). Just compile 
the executable that uses the cache:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build -- -j $(nproc) fast
```

Code is compiled with Clang 18.

---

## Performance Evaluation

Each algorithm was executed **five times** to account for random variations in query order and hash collisions. We then computed the **average total execution time** across all queries for a fair comparison.

We decided **not** to display each individual query time. Instead, we present the **average total time** per algorithm.

### Timing Comparison

| Algorithm         | Run 1 (ms)    | Run 2 (ms)    | Run 3 (ms)    | Run 4 (ms)    | Run 5 (ms)    | **Average (ms)**    |
| ----------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------------- |
| Base Solution         | 170014    | 169454        | 169220        | 168890        | 168634        | 169242              |
| Late Materialization  | 105971    | 105489        | 104977        | 105581        | 105710        | 105546              |                
| Column Store          | 62120     | 62255         | 62023         | 62236         | 62088         | 62144               |
| No root IR            | 60325     | 57856         | 55908         | 60855         | 59563         | 58901               |
| Unchained table       | 46276     | 46231         | 46182         | 46145         | 46246         | 46216               |

- Performance results show that the unchained table, as described in the paper, achieves the fastest execution, being more than 4× faster than the base solution. All other optimizations also have a significant impact on runtime, with the column store in particular providing a large performance improvement.

---

## Team Contributions

| Member             | Contributions                                                                                                                                                                                             |
| -----------------  | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **D. Andreakis**   |  • Implemented Late Materialization and Column Store optimization techniques. <br> • Co-implemented Unchained Hashtable <br> • Set up continuous integration through GitHub Actions. <br>• Executed performance testing.|
| **Ev. Vasileiou**  | • Implemented Unit-Tests for Column Store, Late Materialization. <br> • Co-implemented Unchained table Unit-Test. |
| **A. Kolokouras** | • Co-implemented Unchained Hashtable. <br> • Co-implemented Unchained table Unit-Test. <br> • Utilized profiling tools for optimization of execution time.|

---

## System Specifications

All performance tests were conducted on the following system:

- **Processor:** AMD Ryzen 5 7530U with Radeon Graphics @ 2.00GHz
- **RAM:** 16 GB   
- **System Type:** 64-bit operating system, x86_64-based processor  
