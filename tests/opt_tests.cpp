#include <catch2/catch_test_macros.hpp>
#include <value_t.h>
#include <column_t.h>
#include <mycopyscan.h>
#include <mytocolumnar.h>
#include <unchained_table.h>
#include <table.h>
#include <common.h>
#include <vector>
#include <iostream>

using namespace valuet;
using namespace columnt;

// Tests for Late Materialization

TEST_CASE("Testing Value_t Structure") {

    SECTION("Size validity") {
        REQUIRE(sizeof(value_t) == 8);
    }

    SECTION("Integer Handling") {
        int32_t val = 12345;
        value_t v(val);
        
        REQUIRE_FALSE(v.is_null_int32());
        REQUIRE_FALSE(v.is_null_string());
        REQUIRE(v.intvalue == 12345);
    }

    SECTION("Null Integer Handling") {
        value_t v = value_t::null_int32();
        REQUIRE(v.is_null_int32());
        REQUIRE(v.intvalue == INT32_MIN);
    }

    SECTION("String Pointer Handling") {
        // Table 1, Col 2, Page 100, Offset 50
        NewString ns(1, 2, 100, 50); 
        value_t v(ns);

        REQUIRE_FALSE(v.is_null_int32());
        REQUIRE_FALSE(v.is_null_string());
        
        REQUIRE(v.stringvalue.table_id == 1);
        REQUIRE(v.stringvalue.column_id == 2);
        REQUIRE(v.stringvalue.page_id == 100);
        REQUIRE(v.stringvalue.offset_idx == 50);
    }

    SECTION("Null String Handling") {
        value_t v = value_t::null_string();
        REQUIRE(v.is_null_string());
        
        REQUIRE(v.stringvalue.table_id == 0xFF);
        REQUIRE(v.stringvalue.page_id == 0xFFFFFFFF);
    }
}

TEST_CASE("Testing Copy-Scan Functionality") {
    
    std::vector<DataType> types = {DataType::INT32, DataType::VARCHAR};
    std::vector<std::vector<Data>> data;
    
    // Row 0: 100, Hello
    data.push_back({Data(100), Data(std::string("Hello"))});
    // Row 1: 200, World
    data.push_back({Data(200), Data(std::string("World"))});

    Table t(data, types);
    ColumnarTable ct = t.to_columnar();

    std::vector<std::tuple<size_t, DataType>> output_attrs = {
        {0, DataType::INT32},
        {1, DataType::VARCHAR}
    };

    uint8_t table_id = 1;
    
    auto results = mycopyscan::copy_scan_value_t(ct, output_attrs, table_id);

    // Making sure, that everything worked out
    REQUIRE(results.size() == 2);

    auto& col_int = results[0]; 
    REQUIRE(col_int.size() == 2);
    
    REQUIRE(col_int[0].intvalue == 100);
    REQUIRE(col_int[1].intvalue == 200);

    auto& col_str = results[1];
    REQUIRE(col_str.size() == 2);

    REQUIRE_FALSE(col_str[0].is_null_string());
    REQUIRE(col_str[0].stringvalue.table_id == table_id);
    REQUIRE(col_str[0].stringvalue.column_id == 1); 
    REQUIRE(col_str[0].stringvalue.page_id == 0); 
    
    REQUIRE_FALSE(col_str[1].is_null_string());
    
    REQUIRE(col_str[0].stringvalue.offset_idx == 0);
    REQUIRE(col_str[1].stringvalue.offset_idx == 1);
}
TEST_CASE("Long Strings and Multiple Pages") {
    
    // Helper function, to create strings
    auto make_string = [](size_t len) {
        return std::string(len, 'a');
    };

    SECTION("Long String Handling") {
        
        std::string long_str = make_string(9000);
        
        std::vector<DataType> types = {DataType::VARCHAR};
        std::vector<std::vector<Data>> data;
        data.push_back({Data(long_str)});

        Table t(data, types);
        ColumnarTable ct = t.to_columnar();

        std::vector<std::tuple<size_t, DataType>> output_attrs = {{0, DataType::VARCHAR}};
        auto results = mycopyscan::copy_scan_value_t(ct, output_attrs, 1);

        REQUIRE(results.size() == 1);
        REQUIRE(results[0].size() == 1);
        
        auto& val = results[0][0];
        REQUIRE_FALSE(val.is_null_string());
        
        REQUIRE(val.stringvalue.offset_idx == 0); 
        
        REQUIRE(val.stringvalue.page_id < 100); 
    }

    SECTION("Multiple Pages Handling") {
        
        std::vector<DataType> types = {DataType::VARCHAR};
        std::vector<std::vector<Data>> data;
        
        int total_rows = 2000;
        for(int i = 0; i < total_rows; ++i) {
            data.push_back({Data(std::string("small_str"))});
        }

        Table t(data, types);
        ColumnarTable ct = t.to_columnar();

        std::vector<std::tuple<size_t, DataType>> output_attrs = {{0, DataType::VARCHAR}};
        auto results = mycopyscan::copy_scan_value_t(ct, output_attrs, 1);

        REQUIRE(results[0].size() == total_rows);

        // First one should be on the first page, last one should be to the last page
        auto first_page = results[0][0].stringvalue.page_id;
        auto last_page = results[0][total_rows - 1].stringvalue.page_id;

        REQUIRE(first_page == 0);
        REQUIRE(last_page > first_page);
    }
}

// Tests for Column Store

TEST_CASE("Column Store Logic") {

    REQUIRE(columnt::PAGE_SIZE == 8192);
    size_t expected_values_per_page = 8192 / 8;
    REQUIRE(columnt::VALUES_PER_PAGE == expected_values_per_page);

    SECTION("Single Page Allocation") {
        column_t col;
        // Filling up a whole page
        for(size_t i = 0; i < columnt::VALUES_PER_PAGE; ++i) {
            col.push_back(value_t((int32_t)i));
        }

        REQUIRE(col.size() == columnt::VALUES_PER_PAGE);
        // Everything should fit in one page
        REQUIRE(col.pages.size() == 1);

        REQUIRE(col[columnt::VALUES_PER_PAGE - 1].intvalue == (int32_t)(columnt::VALUES_PER_PAGE - 1));
    }

    SECTION("Multi Page Allocation") {
        column_t col;
        // Filling up a whole page + 1
        size_t total_items = columnt::VALUES_PER_PAGE + 1;
        
        for(size_t i = 0; i < total_items; ++i) {
            col.push_back(value_t((int32_t)i));
        }

        REQUIRE(col.size() == total_items);
        // We must have two pages
        REQUIRE(col.pages.size() == 2);

        REQUIRE(col[columnt::VALUES_PER_PAGE].intvalue == (int32_t)columnt::VALUES_PER_PAGE);
    }

    SECTION("Large Scale Data Integrity") {
        column_t col;
        // Now we are going to fill up 5 pages
        size_t total_items = columnt::VALUES_PER_PAGE * 5; 
        
        for(size_t i = 0; i < total_items; ++i) {
            col.push_back(value_t((int32_t)(i * 2)));
        }

        REQUIRE(col.pages.size() == 5);
        REQUIRE(col.size() == total_items);

        // Verification loop, everything should be to the expected place
        bool passed = true;
        for(size_t i = 0; i < total_items; ++i) {
            if(col[i].intvalue != (int32_t)(i * 2)) {
                passed = false;
                break;
            }
        }
        REQUIRE(passed);
    }
}

TEST_CASE("ScanNode Integration with Column Store") {
    
    // PAGE_SIZE (8192) / 8 bytes = 1024 values per page.
    // With 1500 we are going to have 2 pages
    size_t num_rows = 1500;
    std::vector<DataType> types = {DataType::INT32};
    std::vector<std::vector<Data>> data;
    
    for(size_t i = 0; i < num_rows; ++i) {
        data.push_back({Data((int32_t)i)});
    }

    Table t(data, types);
    ColumnarTable ct = t.to_columnar();

    std::vector<std::tuple<size_t, DataType>> output_attrs = {{0, DataType::INT32}};
    uint8_t table_id = 1;
    
    std::vector<columnt::column_t> results = mycopyscan::copy_scan_value_t(ct, output_attrs, table_id);

    // Verification checks
    REQUIRE(results.size() == 1);
    
    auto& col = results[0];
    
    REQUIRE(col.size() == num_rows);
    
    // 1500 > 1024, so 2 pages
    REQUIRE(col.pages.size() == 2);
    
    // Scan verifications
    REQUIRE(col[0].intvalue == 0);
    REQUIRE(col[1023].intvalue == 1023);
    REQUIRE(col[1024].intvalue == 1024);
    REQUIRE(col[1499].intvalue == 1499);
}

TEST_CASE("to_columnar_value_t Conversion") {
    
    SECTION("INT32 Round-Trip") {
        // Create original table with two columns
        std::vector<DataType> types = {DataType::INT32, DataType::INT32};
        std::vector<std::vector<Data>> data;
        
        data.push_back({Data(100), Data(200)});
        data.push_back({Data(300), Data(400)});
        data.push_back({Data(500), Data(600)});
        
        Table t(data, types);
        ColumnarTable ct = t.to_columnar();
        
        // Create a mock plan for to_columnar_value_t
        Plan plan;
        plan.inputs.emplace_back(std::move(ct));
        
        PlanNode root_node(ScanNode{.base_table_id = 0}, {{0, DataType::INT32}, {1, DataType::INT32}});
        plan.nodes.push_back(std::move(root_node));
        plan.root = 0;
        
        // Copy-scan to late-materialized format
        std::vector<std::tuple<size_t, DataType>> output_attrs = {{0, DataType::INT32}, {1, DataType::INT32}};
        std::vector<column_t> late_mat = mycopyscan::copy_scan_value_t(plan.inputs[0], output_attrs, 0);
        
        // Convert back to columnar
        ColumnarTable result = mytocolumnar::to_columnar_value_t(late_mat, plan);
        
        REQUIRE(result.num_rows == 3);
        REQUIRE(result.columns.size() == 2);
        REQUIRE(result.columns[0].type == DataType::INT32);
        REQUIRE(result.columns[1].type == DataType::INT32);
        
        // Verify first column
        auto* page0 = reinterpret_cast<uint8_t*>(result.columns[0].pages[0]->data);
        uint16_t num_rows0 = *reinterpret_cast<uint16_t*>(page0);
        uint16_t num_values0 = *reinterpret_cast<uint16_t*>(page0 + 2);
        REQUIRE(num_rows0 == 3);
        REQUIRE(num_values0 == 3);
        
        auto* data0 = reinterpret_cast<int32_t*>(page0 + 4);
        REQUIRE(data0[0] == 100);
        REQUIRE(data0[1] == 300);
        REQUIRE(data0[2] == 500);
        
        // Verify second column
        auto* page1 = reinterpret_cast<uint8_t*>(result.columns[1].pages[0]->data);
        uint16_t num_rows1 = *reinterpret_cast<uint16_t*>(page1);
        uint16_t num_values1 = *reinterpret_cast<uint16_t*>(page1 + 2);
        REQUIRE(num_rows1 == 3);
        REQUIRE(num_values1 == 3);
        
        auto* data1 = reinterpret_cast<int32_t*>(page1 + 4);
        REQUIRE(data1[0] == 200);
        REQUIRE(data1[1] == 400);
        REQUIRE(data1[2] == 600);
    }

    
    SECTION("Multi-Page INT32") {
        // VALUES_PER_PAGE = 8192/8 = 1024. Create 2049 rows for 3 pages.
        size_t total_rows = columnt::VALUES_PER_PAGE * 2 + 1;
        std::vector<DataType> types = {DataType::INT32};
        std::vector<std::vector<Data>> data;
        
        for(size_t i = 0; i < total_rows; ++i) {
            data.push_back({Data((int32_t)(i * 10))});
        }
        
        Table t(data, types);
        ColumnarTable ct = t.to_columnar();
        
        Plan plan;
        plan.inputs.emplace_back(std::move(ct));
        
        PlanNode root_node(ScanNode{.base_table_id = 0}, {{0, DataType::INT32}});
        plan.nodes.push_back(std::move(root_node));
        plan.root = 0;
        
        std::vector<std::tuple<size_t, DataType>> output_attrs = {{0, DataType::INT32}};
        std::vector<column_t> late_mat = mycopyscan::copy_scan_value_t(plan.inputs[0], output_attrs, 0);
        
        ColumnarTable result = mytocolumnar::to_columnar_value_t(late_mat, plan);
        
        REQUIRE(result.num_rows == total_rows);
        REQUIRE(result.columns.size() == 1);
        // The conversion may pack more efficiently - just verify multiple pages exist
        REQUIRE(result.columns[0].pages.size() >= 2);
        
        // Verify first page values
        auto* page0 = reinterpret_cast<uint8_t*>(result.columns[0].pages[0]->data);
        auto* data0 = reinterpret_cast<int32_t*>(page0 + 4);
        REQUIRE(data0[0] == 0);
        REQUIRE(data0[100] == 1000);
        
        // Verify there are multiple pages with data
        REQUIRE(result.columns[0].pages.size() >= 2);
    }
}

// UnchainedHashTable tests
TEST_CASE("UnchainedHashTable basic operations", "[unchained]") {
    UnchainedHashTable ht;
    ht.reserve(10);

    REQUIRE(ht.capacity == 1024); // Minimum capacity

    // Insert some keys
    ht.insert(1, 0);
    ht.insert(2, 1);
    ht.insert(1, 2);  // duplicate key
    ht.insert(3, 3);

    ht.finalize();

    // Find values using pointer-based API
    size_t len = 0;
    const HashEntry* entries = ht.find_range(1, len);
    REQUIRE(len == 2); // Duplicate key
    REQUIRE(entries[0].row_idx == 0);
    REQUIRE(entries[1].row_idx == 2);

    entries = ht.find_range(2, len);
    REQUIRE(len == 1);
    REQUIRE(entries[0].row_idx == 1);

    entries = ht.find_range(3, len);
    REQUIRE(len == 1);
    REQUIRE(entries[0].row_idx == 3);

    entries = ht.find_range(4, len);  // not present
    REQUIRE(len == 0);

    REQUIRE(ht.size() == 4);
}

TEST_CASE("UnchainedHashTable empty table", "[unchained]") {
    UnchainedHashTable ht;
    ht.reserve(10); 

    ht.finalize();

    size_t len = 0;
    const HashEntry* entries = ht.find_range(1, len);
    REQUIRE(len == 0);
    REQUIRE(ht.size() == 0);
}

TEST_CASE("UnchainedHashTable single key multiple values", "[unchained]") {
    UnchainedHashTable ht;
    ht.reserve(10);

    for (size_t i = 0; i < 10; ++i) {
        ht.insert(42, i);
    }

    ht.finalize();

    size_t len = 0;
    const HashEntry* entries = ht.find_range(42, len);
    REQUIRE(len == 10);
    for (size_t i = 0; i < 10; ++i) {
        REQUIRE(entries[i].row_idx == i);
    }
}

TEST_CASE("UnchainedHashTable large table", "[unchained]") {
    UnchainedHashTable ht;
    size_t table_size = 1025;
    ht.reserve(table_size); // force larger capacity

    REQUIRE(ht.capacity == 2048); // Next power of 2 >= 1025 is 2048

    for (int32_t i = 0; i < static_cast<int32_t>(table_size); ++i) {
        ht.insert(i, static_cast<size_t>(i));
    }

    ht.finalize();

    for (int32_t i = 0; i < static_cast<int32_t>(table_size); ++i) {
        size_t len = 0;
        const HashEntry* entries = ht.find_range(i, len);
        REQUIRE(len >= 1); // At least one entry for each key
        // Find our specific row_idx
        bool found = false;
        for (size_t j = 0; j < len; ++j) {
            if (entries[j].key == i && entries[j].row_idx == static_cast<size_t>(i)) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }

    REQUIRE(ht.size() == table_size);
}

TEST_CASE("UnchainedHashTable slot collision handling", "[unchained]") {
    UnchainedHashTable ht;
    ht.reserve(100);

    int32_t key1 = 41234; // Sample key 
    ht.insert(key1, 0);

    uint64_t h1 = UnchainedHashTable::hash(key1);
    uint64_t slot1 = h1 >> ht.shift; // Find the slot for key1
    
    int32_t colliding_key = -1;
    // Loop until a key with the same slot is found
    int32_t candidate;
    for (candidate = 0; candidate < 1000000; ++candidate) {
        if (candidate == key1) continue;
        uint64_t hc = UnchainedHashTable::hash(candidate);
        uint64_t slotc = hc >> ht.shift;
        // Check if the candidate has the same slot
        if (slotc == slot1) {
            colliding_key = candidate;
            break;
        }
    }

    INFO("Colliding key not found");
    REQUIRE(colliding_key != -1);
    
    ht.insert(colliding_key, 1);
    ht.finalize();

    size_t len = 0;
    const HashEntry* entries1 = ht.find_range(key1, len);
    REQUIRE(len >= 1);
    // Find entry with key1 and row_idx 0
    bool found1 = false;
    for (size_t i = 0; i < len; ++i) {
        if (entries1[i].key == key1 && entries1[i].row_idx == 0) {
            found1 = true;
            break;
        }
    }
    REQUIRE(found1);

    const HashEntry* entries2 = ht.find_range(colliding_key, len);
    REQUIRE(len >= 1);
    // Find entry with colliding_key and row_idx 1
    bool found2 = false;
    for (size_t i = 0; i < len; ++i) {
        if (entries2[i].key == colliding_key && entries2[i].row_idx == 1) {
            found2 = true;
            break;
        }
    }
    REQUIRE(found2);
}
