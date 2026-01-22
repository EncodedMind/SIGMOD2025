#include <catch2/catch_test_macros.hpp>
#include <plan.h>
#include <column_t.h>
#include <mycopyscan.h>
#include <threaded_table.h>
#include <vector>
#include <cstring>
#include <thread>
#include <memory>

namespace {
    inline Page* create_dense_int32_page(const std::vector<int32_t>& values) {
        // if num_rows == num_values then page is dense
        auto* page = new Page();
        uint16_t num_values = static_cast<uint16_t>(values.size());
        
        auto* header_rows = reinterpret_cast<uint16_t*>(page->data);
        auto* header_values = reinterpret_cast<uint16_t*>(page->data + 2);
        *header_rows = num_values;
        *header_values = num_values;

        auto* data_ptr = reinterpret_cast<int32_t*>(page->data + 4);
        for(size_t i = 0; i < values.size(); ++i){
            data_ptr[i] = values[i];
        }

        constexpr size_t PAGE_SIZE = 8192;
        size_t bitmap_size = (num_values + 7) / 8;
        auto* bitmap = reinterpret_cast<uint8_t*>(page->data + PAGE_SIZE - bitmap_size);
        std::memset(bitmap, 0xFF, bitmap_size);  // all bits set = all rows present
        
        return page;
    }
}

TEST_CASE("Dense INT32 column reference", "[join]"){
    // ColumnarTable with one dense INT32 column
    ColumnarTable table;
    table.num_rows = 3;
    table.columns.emplace_back(DataType::INT32);
    
    auto& col = table.columns[0];
    
    std::vector<int32_t> test_values = {10, 20, 30}; // dense INT32 page
    col.pages.push_back(create_dense_int32_page(test_values));
    
    std::vector<std::tuple<size_t, DataType>> output_attrs = {
        {0, DataType::INT32}
    };

    auto result_cols = mycopyscan::copy_scan_value_t(table, output_attrs, 0);
    
    // The result column should have ref set (not nullptr)
    REQUIRE(result_cols.size() == 1);
    REQUIRE(result_cols[0].ref != nullptr);
    REQUIRE(result_cols[0].num_values == 3);

    REQUIRE(result_cols[0][0].intvalue == 10);
    REQUIRE(result_cols[0][1].intvalue == 20);
    REQUIRE(result_cols[0][2].intvalue == 30);
}

TEST_CASE("Parallelized hash table building", "[join]"){
    const size_t num_threads = 4;
    const size_t num_partitions = 4;
    const size_t rows_per_thread = 1000;
    
    threaded::GlobalAllocator globalAlloc;
    std::vector<std::unique_ptr<threaded::TupleCollector>> collectors;
    collectors.reserve(num_threads);

    for(size_t i = 0; i < num_threads; ++i){
        collectors.push_back(std::make_unique<threaded::TupleCollector>(globalAlloc, num_partitions));
    }

    std::vector<std::thread> build_threads;
    build_threads.reserve(num_threads);
    
    for(size_t t = 0; t < num_threads; ++t){
        build_threads.emplace_back([&, t](){
            auto& collector = *collectors[t];
            size_t start = t * rows_per_thread;
            size_t end = start + rows_per_thread;
            
            for(size_t row_idx = start; row_idx < end; ++row_idx){
                int32_t key = static_cast<int32_t>(row_idx % 500);  // we use row_idx as the key for simplicity
                collector.consume(threaded::HashEntry(key, row_idx));
            }
        });
    }
    for(auto& t : build_threads) t.join();
    
    // Merge phase
    std::vector<threaded::Block*> partition_heads = threaded::merge_partitions(collectors, num_partitions);
    
    // Count total tuples
    size_t total_tuples = 0;
    for(const auto& col : collectors){
        for(size_t c : col->counts) total_tuples += c;
    }
    
    REQUIRE(total_tuples == num_threads * rows_per_thread);
    
    // Build final table
    threaded::FinalTable final_table(total_tuples, num_partitions);
    
    // Compute partition offsets
    std::vector<size_t> partition_offsets(num_partitions, 0);
    size_t running_count = 0;
    std::vector<size_t> global_partition_counts(num_partitions, 0);
    
    for(size_t p = 0; p < num_partitions; ++p){
        for(const auto& col : collectors){
            global_partition_counts[p] += col->counts[p];
        }
    }
    
    for(size_t p = 0; p < num_partitions; ++p){
        partition_offsets[p] = running_count;
        running_count += global_partition_counts[p];
    }
    
    // Post-process build
    std::vector<std::thread> post_build_threads;
    post_build_threads.reserve(num_partitions);
    
    for(size_t p = 0; p < num_partitions; ++p){
        post_build_threads.emplace_back([&, p](){
            final_table.postProcessBuild(p, partition_offsets[p], partition_heads);
        });
    }
    for(auto& t : post_build_threads) t.join();

    size_t len = 0;
    const auto* entries = final_table.find_range(0, len);
    REQUIRE(entries != nullptr);
    REQUIRE(len > 0);

    size_t count_key_0 = 0;
    for(size_t i = 0; i < len; ++i){
        if(entries[i].key == 0) count_key_0++;
    }
    REQUIRE(count_key_0 > 0);

    entries = final_table.find_range(1, len);
    REQUIRE(entries != nullptr);
    REQUIRE(len > 0);
}

TEST_CASE("Parallelized hash table probing", "[join]"){
    const size_t num_build_threads = 2;
    const size_t num_build_partitions = 2;
    const size_t build_rows = 500;
    
    threaded::GlobalAllocator globalAlloc;
    std::vector<std::unique_ptr<threaded::TupleCollector>> collectors;
    collectors.reserve(num_build_threads);

    for(size_t i = 0; i < num_build_threads; ++i){
        collectors.push_back(std::make_unique<threaded::TupleCollector>(globalAlloc, num_build_partitions));
    }

    std::vector<std::thread> build_threads;
    for(size_t t = 0; t < num_build_threads; ++t){
        build_threads.emplace_back([&, t](){
            auto& collector = *collectors[t];
            size_t start = t * build_rows;
            size_t end = start + build_rows;
            
            for(size_t row_idx = start; row_idx < end; ++row_idx){
                int32_t key = static_cast<int32_t>(row_idx % 50);  // 50 unique keys
                collector.consume(threaded::HashEntry(key, row_idx));
            }
        });
    }
    for(auto& t : build_threads) t.join();

    std::vector<threaded::Block*> partition_heads = threaded::merge_partitions(collectors, num_build_partitions);

    size_t total_tuples = 0;
    for(const auto& col : collectors){
        for(size_t c : col->counts) total_tuples += c;
    }

    threaded::FinalTable final_table(total_tuples, num_build_partitions);

    std::vector<size_t> partition_offsets(num_build_partitions, 0);
    size_t running_count = 0;
    std::vector<size_t> global_partition_counts(num_build_partitions, 0);
    
    for(size_t p = 0; p < num_build_partitions; ++p){
        for(const auto& col : collectors){
            global_partition_counts[p] += col->counts[p];
        }
    }
    
    for(size_t p = 0; p < num_build_partitions; ++p){
        partition_offsets[p] = running_count;
        running_count += global_partition_counts[p];
    }

    std::vector<std::thread> post_build_threads;
    for(size_t p = 0; p < num_build_partitions; ++p){
        post_build_threads.emplace_back([&, p](){
            final_table.postProcessBuild(p, partition_offsets[p], partition_heads);
        });
    }
    for(auto& t : post_build_threads) t.join();

    // Now test probing with work-stealing parallelization
    const size_t num_probe_threads = 4;
    const size_t num_probe_keys = 30;
    
    // Each thread will grab keys via atomic fetch_add (work-stealing)
    std::atomic<size_t> next_key{0};
    std::vector<std::vector<std::pair<int32_t, size_t>>> local_results(num_probe_threads);
    
    std::vector<std::thread> probe_threads;
    for(size_t t = 0; t < num_probe_threads; ++t){
        probe_threads.emplace_back([&, t](){
            auto& results = local_results[t];
            
            while(true){
                const size_t key_id = next_key.fetch_add(1, std::memory_order_relaxed);
                if(key_id >= num_probe_keys) break;
                
                int32_t key = static_cast<int32_t>(key_id);
                size_t len = 0;
                const auto* entries = final_table.find_range(key, len);
                
                if(entries && len > 0){
                    for(size_t i = 0; i < len; ++i){
                        if(entries[i].key == key){
                            results.emplace_back(key, entries[i].row_idx);
                        }
                    }
                }
            }
        });
    }
    for(auto& t : probe_threads) t.join();

    size_t total_matches = 0;
    for(const auto& local : local_results){
        total_matches += local.size();
    }
    
    // We built 1000 rows total (500 * 2 threads) with keys % 50
    // Each of 30 keys appears in 1000/50 = 20 rows
    // But we only probe keys 0-29, so we get 30 * 20 = 600 matches
    REQUIRE(total_matches == 600);

    std::map<int32_t, size_t> match_counts;
    for(const auto& local : local_results){
        for(const auto& [key, row_idx] : local){
            match_counts[key]++;
        }
    }
    
    for(int32_t k = 0; k < 30; ++k){
        REQUIRE(match_counts[k] == 20); // each key should match 1000/50 rows
    }
}

// Edge case tests

TEST_CASE("Hash table edge case: all same key", "[join]"){
    // All rows have the same key
    const size_t num_threads = 2;
    const size_t num_partitions = 2;
    const size_t rows_per_thread = 500;
    const int32_t common_key = 42;
    
    threaded::GlobalAllocator globalAlloc;
    std::vector<std::unique_ptr<threaded::TupleCollector>> collectors;
    collectors.reserve(num_threads);

    for(size_t i = 0; i < num_threads; ++i){
        collectors.push_back(std::make_unique<threaded::TupleCollector>(globalAlloc, num_partitions));
    }

    std::vector<std::thread> build_threads;
    for(size_t t = 0; t < num_threads; ++t){
        build_threads.emplace_back([&, t](){
            auto& collector = *collectors[t];
            size_t start = t * rows_per_thread;
            size_t end = start + rows_per_thread;
            
            for(size_t row_idx = start; row_idx < end; ++row_idx){
                // test collision handling
                collector.consume(threaded::HashEntry(common_key, row_idx));
            }
        });
    }
    for(auto& t : build_threads) t.join();

    std::vector<threaded::Block*> partition_heads = threaded::merge_partitions(collectors, num_partitions);

    size_t total_tuples = 0;
    for(const auto& col : collectors){
        for(size_t c : col->counts) total_tuples += c;
    }
    REQUIRE(total_tuples == num_threads * rows_per_thread);

    threaded::FinalTable final_table(total_tuples, num_partitions);

    std::vector<size_t> partition_offsets(num_partitions, 0);
    size_t running_count = 0;
    std::vector<size_t> global_partition_counts(num_partitions, 0);
    
    for(size_t p = 0; p < num_partitions; ++p){
        for(const auto& col : collectors){
            global_partition_counts[p] += col->counts[p];
        }
    }
    
    for(size_t p = 0; p < num_partitions; ++p){
        partition_offsets[p] = running_count;
        running_count += global_partition_counts[p];
    }

    std::vector<std::thread> post_build_threads;
    for(size_t p = 0; p < num_partitions; ++p){
        post_build_threads.emplace_back([&, p](){
            final_table.postProcessBuild(p, partition_offsets[p], partition_heads);
        });
    }
    for(auto& t : post_build_threads) t.join();

    // Probe for the common key - should get all rows
    size_t len = 0;
    const auto* entries = final_table.find_range(common_key, len);
    REQUIRE(entries != nullptr);
    REQUIRE(len == num_threads * rows_per_thread);
    
    // Count matching entries
    size_t matches = 0;
    for(size_t i = 0; i < len; ++i){
        if(entries[i].key == common_key) matches++;
    }
    REQUIRE(matches == num_threads * rows_per_thread);

    // Probe for a key that doesn't exist
    len = 0;
    const auto* no_entries = final_table.find_range(999, len);
    REQUIRE(no_entries == nullptr);
    REQUIRE(len == 0);
}

TEST_CASE("Hash table edge case: large scale", "[join]"){
    // Stress test with 100k rows to test allocator scalability
    const size_t num_threads = 4;
    const size_t num_partitions = 4;
    const size_t rows_per_thread = 25000;  // 100k total rows
    
    threaded::GlobalAllocator globalAlloc;
    std::vector<std::unique_ptr<threaded::TupleCollector>> collectors;
    collectors.reserve(num_threads);

    for(size_t i = 0; i < num_threads; ++i){
        collectors.push_back(std::make_unique<threaded::TupleCollector>(globalAlloc, num_partitions));
    }

    std::vector<std::thread> build_threads;
    for(size_t t = 0; t < num_threads; ++t){
        build_threads.emplace_back([&, t](){
            auto& collector = *collectors[t];
            size_t start = t * rows_per_thread;
            size_t end = start + rows_per_thread;
            
            for(size_t row_idx = start; row_idx < end; ++row_idx){
                int32_t key = static_cast<int32_t>(row_idx % 1000);  // 1000 unique keys
                collector.consume(threaded::HashEntry(key, row_idx));
            }
        });
    }
    for(auto& t : build_threads) t.join();

    std::vector<threaded::Block*> partition_heads = threaded::merge_partitions(collectors, num_partitions);

    size_t total_tuples = 0;
    for(const auto& col : collectors){
        for(size_t c : col->counts) total_tuples += c;
    }
    REQUIRE(total_tuples == 100000);

    threaded::FinalTable final_table(total_tuples, num_partitions);

    std::vector<size_t> partition_offsets(num_partitions, 0);
    size_t running_count = 0;
    std::vector<size_t> global_partition_counts(num_partitions, 0);
    
    for(size_t p = 0; p < num_partitions; ++p){
        for(const auto& col : collectors){
            global_partition_counts[p] += col->counts[p];
        }
    }
    
    for(size_t p = 0; p < num_partitions; ++p){
        partition_offsets[p] = running_count;
        running_count += global_partition_counts[p];
    }

    std::vector<std::thread> post_build_threads;
    for(size_t p = 0; p < num_partitions; ++p){
        post_build_threads.emplace_back([&, p](){
            final_table.postProcessBuild(p, partition_offsets[p], partition_heads);
        });
    }
    for(auto& t : post_build_threads) t.join();

    // Sample probe: key 0 should appear 100 times (100k / 1000)
    size_t len = 0;
    const auto* entries = final_table.find_range(0, len);
    REQUIRE(entries != nullptr);
    REQUIRE(len > 0);
    
    size_t count_key_0 = 0;
    for(size_t i = 0; i < len; ++i){
        if(entries[i].key == 0) count_key_0++;
    }
    REQUIRE(count_key_0 == 100);
}

TEST_CASE("Hash table edge case: single row", "[join]"){
    // Minimal degenerate case
    const size_t num_threads = 1;
    const size_t num_partitions = 1;
    
    threaded::GlobalAllocator globalAlloc;
    std::vector<std::unique_ptr<threaded::TupleCollector>> collectors;
    collectors.reserve(num_threads);

    for(size_t i = 0; i < num_threads; ++i){
        collectors.push_back(std::make_unique<threaded::TupleCollector>(globalAlloc, num_partitions));
    }

    int32_t key_value = 5;
    collectors[0]->consume(threaded::HashEntry(key_value, 0));

    std::vector<threaded::Block*> partition_heads = threaded::merge_partitions(collectors, num_partitions);

    size_t total_tuples = 1;
    threaded::FinalTable final_table(total_tuples, num_partitions);

    std::vector<size_t> partition_offsets(num_partitions, 0);
    final_table.postProcessBuild(0, 0, partition_heads);

    // Probe for the single row
    size_t len = 0;
    const auto* entries = final_table.find_range(key_value, len);
    REQUIRE(entries != nullptr);
    REQUIRE(len == 1);
    REQUIRE(entries[0].key == key_value);
    REQUIRE(entries[0].row_idx == 0);

    // Probe for non-existent key
    len = 0;
    const auto* no_entries = final_table.find_range(100, len);
    REQUIRE(no_entries == nullptr);
    REQUIRE(len == 0);
}

