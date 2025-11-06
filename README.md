# SIGMOD 2025: Join Pipeline Optimization

This project is based on the [SIGMOD 2025 programming contest](https://sigmod-contest-2025.github.io). The task involves optimizing the join pipeline by implementing an efficient join algorithm to reduce execution time.

## Project Description

In the first part of the assignment, we implemented the join operator by developing three different versions of the Hash Join algorithm. In the baseline solution, the hash table uses `std::unordered_map` from the C++ STL. We implemented three alternative hash-based join strategies to improve performance:
1. Robin Hood Hashing
2. Cuckoo Hashing
3. Hopscotch Hashing

## Team Information

**Team Name:** CTRL+S our lives

| Name                 | Student ID     | GitHub Username |
| -------------------- | -------------- | --------------- |
| Andreakis Dimitrios  | 1115202300008  | EncodedMind     |
| Vasileiou Evaggelos  | 1115201900309  | VangelisVas     |
| Kolokouras Apostolos | 1115202100259  | TolisKlk        |

---

## File Structure
*\*Only the most essential files\**
```bash
k23a-2025-d1-ctrl-s-our-souls/
├── src/
│   ├── execute.cpp
├── include/
│   ├── robinhood.h
│   ├── cuckoo.h
│   ├── hopscotch.h
├── tests/
│   └── hash_tests.cpp
├── job/
├── CMakeLists.txt
├── Makefile
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

**Make sure that only ONE header file is uncommented on the top of the `execute.cpp` file.**

---

## Design Choices

### Rehash Logic

* Robin Hood: Robin Hood does not need rehash logic, because we are allocating a hash table that is always large enough.

* Cuckoo: We detect cycles by counting the number of elements that have been moved. If we reach the total number of elements in the array, then a cycle must necessarily exist.

* Hopscotch: We rehash when the neighbourhood of a key is full or when there is no availabe slot to move the "empty slot".

### Hash functions

* For hashing functions, we implemented **FNV-1a** and **Murmur-like** variants for performance testing, but they ended up being slower, so we stuck with `std::hash`.

---

## Performance Evaluation

Each algorithm was executed **five times** to account for random variations in query order and hash collisions. We then computed the **average total execution time** across all queries for a fair comparison.

We decided **not** to display each individual query time, since that data is too granular and noisy. Instead, we present the **average total time** per algorithm.

### Timing Comparison

| Algorithm         | Run 1 (ms)    | Run 2 (ms)    | Run 3 (ms)    | Run 4 (ms)    | Run 5 (ms)    | **Average (ms)**    |
| ----------------- | ------------- | ------------- | ------------- | ------------- | ------------- | ------------------- |
| Unordered map     | 163384        | 159158        | 159332        | 158695        | 159671        | 160048              |
| Robin Hood        | 922135        | 975615        | 915348        | 915743        | 915664        | 928901              |
| Cuckoo            | 201315        | 186279        | 183182        | 186543        | 183697        | 188203              |
| Hopscotch         | 235088        | 227406        | 229156        | 228974        | 228851        | 229895              |

- The performance comparison shows that `std::unordered_map` was the fastest implementation overall, achieving the lowest average runtime (≈160 seconds).
- Among the custom algorithms, Cuckoo hashing performed the best, followed by Hopscotch, while Robin Hood hashing was significantly slower.

---

## Team Contributions

| Member             | Contributions                                                                                                                                                                                             |
| -----------------  | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **D. Andreakis**   | • Implemented all three algorithms and integrated them into the main project.<br>• Set up continuous integration through GitHub Actions.<br>• Prepared the README.md.<br>• Executed performance testing.  |
| **Ev. Vasileiou**  | • Created the Makefile to simplify compilation.<br>• Implemented the unit tests for Robin Hood and co-implemented the unit tests for Cuckoo and Hopscotch.                                                |
| **Ap. Kolokouras** | • Proposed integrating all components into a single executable with modular header files.<br>• Co-implemented the unit tests for Cuckoo and Hopscotch.                                                    |

---

## System Specifications

All performance tests were conducted on the following system:

- **Processor:** 11th Gen Intel(R) Core(TM) i5-11400F @ 2.60GHz  
- **RAM:** 16 GB (15.9 GB usable)  
- **System Type:** 64-bit operating system, x64-based processor  

> Note: The code was run in a virtual machine configured to use all available resources of the host system.