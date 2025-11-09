#include <catch2/catch_test_macros.hpp>

#include <table.h>
#include <plan.h>
#include "hopscotch.h"

TEST_CASE("Hopscotch Hashing", "[hopscotch]"){

    using Hopscotch = Hashalgorithm<int32_t>;

    SECTION("Basic Insert and Find"){
        Hopscotch hashtable(11); // N = 32
        REQUIRE(hashtable.N == 32);

        hashtable.insert(1, {static_cast<size_t>(10)});
        hashtable.insert(16, {static_cast<size_t>(20)});

        REQUIRE(!hashtable.find_values(1).empty());
        REQUIRE(!hashtable.find_values(16).empty());
        REQUIRE(hashtable.find_values(20).empty());

        REQUIRE(hashtable.hashtable[hashtable.hash_function(1)].key == 1);
        REQUIRE(hashtable.hashtable[hashtable.hash_function(16)].key == 16);
    }

    SECTION("Insertions with Identical Key"){
        Hopscotch hashtable(24);
        REQUIRE(hashtable.N == 64);

        for(int i = 0; i < 23; ++i){
            hashtable.insert(0, {static_cast<size_t>(i*10)});
        }

        REQUIRE(!hashtable.find_values(0).empty());
        REQUIRE(hashtable.hashtable[0].values.size() == 23);

        for(int i = 0; i < 23; ++i){
            REQUIRE(hashtable.hashtable[0].values[i] == static_cast<size_t>(i * 10));
        }

        for(int i = 1; i < 9; ++i){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }
        
        REQUIRE(hashtable.N == 64);
    }

    SECTION("Random Insertions"){
        Hopscotch hashtable(512);
        REQUIRE(hashtable.N == 1024);

        for(int i = 0; i < 512; i++){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }

        for(int i = 0; i < 512; i++){
            REQUIRE(!hashtable.find_values(i).empty());
            REQUIRE(hashtable.find_values(i)[0] == static_cast<size_t>(i*10));
        }

        REQUIRE(hashtable.N == 1024);
    }

    SECTION("Neighborhood Full Resizing"){
        Hopscotch hashtable(32);
        REQUIRE(hashtable.N == 64);

        std::vector<int> keys = { 0, 235, 267, 286, 299, 362, 375, 423, 436, 504, 671,
                                  693, 750, 808, 829, 880, 968, 990, 1104, 1141, 1158,
                                  1211, 1227, 1317, 1342, 1364, 1402, 1452, 1533, 1649,
                                  1664, 1719, 1797 };
    
        for(int i = 0; i < 32; ++i){
            hashtable.insert(keys[i], {static_cast<size_t>(keys[i]*10)});
        }

        // Verify all inserted
        for(int i = 0; i < 32; ++i){
            auto vals = hashtable.find_values(keys[i]);
            REQUIRE(!vals.empty());
            REQUIRE(vals[0] == static_cast<size_t>(keys[i]*10));
        }

        // Insert the 33rd key -> should trigger a rehash because neighborhood is full
        hashtable.insert(keys[32], {static_cast<size_t>(keys[32]*10)});
        REQUIRE(hashtable.N > 64); // verify that resizing happened

        // Verify all keys are still present
        for(int i = 0; i < 33; ++i){
            auto vals = hashtable.find_values(keys[i]);
            REQUIRE(!vals.empty());
            REQUIRE(vals[0] == static_cast<size_t>(keys[i]*10));
        }
    }
}