#include <catch2/catch_test_macros.hpp>

#include <table.h>
#include <plan.h>
#include "cuckoo.h"

TEST_CASE("Cuckoo Hashing", "[cuckoo]"){

    using Cuckoo = Hashalgorithm<int32_t>;

    SECTION("Basic Insert and Find"){
        Cuckoo hashtable(8);
        REQUIRE(hashtable.N == 16);
        REQUIRE(hashtable.hashtable1.size() == hashtable.N);
        REQUIRE(hashtable.hashtable2.size() == hashtable.N);

        hashtable.insert(1, {static_cast<size_t>(10)});
        hashtable.insert(17, {static_cast<size_t>(170)});

        REQUIRE(!hashtable.find_values(1).empty());
        REQUIRE(!hashtable.find_values(17).empty());
        REQUIRE(hashtable.inserted == 2);

        REQUIRE(hashtable.find_values(99).empty());
    }

    SECTION("Insertions with Identical Key"){
        Cuckoo hashtable(24);
        REQUIRE(hashtable.N == 64);

        for(int i = 0; i < 23; ++i){
            hashtable.insert(0, {static_cast<size_t>(i*10)});
        }

        REQUIRE(!hashtable.find_values(0).empty());
        for(int i = 0; i < 23; ++i){
            REQUIRE(hashtable.hashtable1[0].values[i] == static_cast<size_t>(i * 10));
        }

        REQUIRE(hashtable.hashtable1[0].values.size() == 23);
        REQUIRE(hashtable.inserted == 1);

        /* Some random insertions after the identical keys insertions 
           but dont want yet to trigger a resize */
        for(int i = 1; i < 9; ++i){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }
        
        REQUIRE(hashtable.hashtable1.size() == 64);
        REQUIRE(hashtable.hashtable2.size() == 64);
        REQUIRE(hashtable.inserted == 9);
    }

    SECTION("Non-Stop Resizing"){
        Cuckoo hashtable(7);
        REQUIRE(hashtable.N == 16);

        std::vector<int> keys = {272, 73, 82, 0, 153, 386};

        for(int k : keys){
            hashtable.insert(k, {static_cast<size_t>(k*10)});
        }

        // table1: a, b, c, empty  (indexes 0,1,2)
        // table2: d, e, f, empty  (indexes 0,1,2)
        REQUIRE(hashtable.hashtable1[0].key == 0);
        REQUIRE(hashtable.hashtable1[1].key == 153);
        REQUIRE(hashtable.hashtable1[2].key == 386);
        REQUIRE(!hashtable.hashtable1[3].occupied);

        REQUIRE(hashtable.hashtable2[0].key == 272);
        REQUIRE(hashtable.hashtable2[1].key == 73);
        REQUIRE(hashtable.hashtable2[2].key == 82);
        REQUIRE(!hashtable.hashtable2[3].occupied);

        hashtable.insert(10, {static_cast<size_t>(100)});

        REQUIRE(hashtable.N >= 8); 

        // Verify all keys are present
        for(int k : keys) {
            auto v = hashtable.find_values(k);
            REQUIRE(!v.empty());
            REQUIRE(v[0] == static_cast<size_t>(k*10));
        }
        auto vg = hashtable.find_values(10);
        REQUIRE(!vg.empty());
        REQUIRE(vg[0] == static_cast<size_t>(100));
    }

    SECTION("Random Insertions"){
        Cuckoo hashtable(512);
        REQUIRE(hashtable.N == 1024);

        for(int i = 0; i < 512; i++){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }

        for(int i = 0; i < 512; i++){
            REQUIRE(!hashtable.find_values(i).empty());
            REQUIRE(hashtable.find_values(i)[0] == static_cast<size_t>(i*10));
        }

        REQUIRE(hashtable.inserted == 512);
        REQUIRE(hashtable.N == 1024);
    
    }

    SECTION("Collision"){
        Cuckoo hashtable(2);
        REQUIRE(hashtable.N == 16);
        REQUIRE(hashtable.hashtable1.size() == 16);
        REQUIRE(hashtable.hashtable1.size() == 16);

        // h1 = 0 and h2 = 1        
        hashtable.insert(0, {static_cast<size_t>(0)});
        REQUIRE(hashtable.inserted == 1);

        // h1 = 0 and h2 = 1 */
        hashtable.insert(153, {static_cast<size_t>(153)});
    
        // Resize must NOT happen
        REQUIRE(hashtable.hashtable1.size() == 16);
        REQUIRE(hashtable.hashtable2.size() == 16);
        REQUIRE(hashtable.N == 16);
        REQUIRE(hashtable.inserted == 2);

        REQUIRE(!hashtable.find_values(0).empty());
        REQUIRE(!hashtable.find_values(153).empty());
    }
    
}