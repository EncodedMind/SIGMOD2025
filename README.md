# SIGMOD 2025: Join Pipeline Optimization

This project is based on the [SIGMOD 2025 programming contest](https://sigmod-contest-2025.github.io). The task involves optimizing the join pipeline by implementing an efficient join algorithm to reduce execution time.

## Second Assignment Description 


## Team Name: CTRL+S our lives

| Name                 | Student ID     |       Academic email      | GitHub Username   |
| -------------------- | -------------- | ------------------------- |------------------ |
| Andreakis Dimitrios  | 1115202300008  | sdi2300008@di.uoa.gr    | EncodedMind       |
| Vasileiou Evaggelos  | 1115201900309  | sdi1900309@di.uoa.gr    | VangelisVas       |
| Kolokouras Apostolos | 1115202100259  | sdi2100259@di.uoa.gr      | TolisKlk          |

---

## File Structure

****Essential files only***
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

## Run Instunctions

### Refer to original SIGMOD repository for full details
[SIGMOD 2025 programming contest](https://sigmod-contest-2025.github.io)

#### Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev
cmake --build build -- -j $(nproc)
```

#### Run (Not cached)
```bash
./build/run plans.json
```

#### Cache Build

To build the cache you need to run:
```bash
./build/build_cache plans.json
```

#### Cache Run
After the cache is built you can run the queries using:
```bash
./build/fast plans.json
```

#### Build after Cache is ready
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
| **Ev. Vasileiou**  | • Implemented Unit-Tests for Column Store, Late Materialization. <br> • Co-implemented unchaned table Unit-Test. |
| **A. Kolokouras** | • Co-implemented Unchained Hashtable. <br> • Co-implemented Unchained table Unit-Test. <br> • Utilized profiling tools for optimization of execution time.|

---

## System Specifications

All performance tests were conducted on the following system:

- **Processor:** AMD Ryzen 5 7530U with Radeon Graphics @ 2.00GHz (Base), up to 4.55GHz
- **RAM:** 16 GB   
- **System Type:** 64-bit operating system, x86_64-based processor  