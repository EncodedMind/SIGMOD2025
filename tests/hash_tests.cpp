#include <catch2/catch_test_macros.hpp>

#include <table.h>
#include <plan.h>
#include <vector>

namespace robinhood_ns{
    #include <robinhood.h>
}
namespace cuckoo_ns{
    #include <cuckoo.h>
}
namespace hopscotch_ns{
    #include <hopscotch.h>
}

using Robinhood = robinhood_ns::Hashalgorithm<int32_t>;
using Cuckoo = cuckoo_ns::Hashalgorithm<int32_t>;
using Hopscotch = hopscotch_ns::Hashalgorithm<int32_t>;

TEST_CASE("Robin Hood Hashing", "[robinhood]"){

    SECTION("Basic Insert and Find"){
        Robinhood hashtable(11); // N = 32

        hashtable.insert(1, {static_cast<size_t>(10)});
        hashtable.insert(16, {static_cast<size_t>(20)});

        REQUIRE(!hashtable.find_values(1).empty());
        REQUIRE(!hashtable.find_values(16).empty());

        REQUIRE(hashtable.hashtable[hashtable.hash_function(1)].key == 1);
        REQUIRE(hashtable.hashtable[hashtable.hash_function(1)].psl == 0);

        REQUIRE(hashtable.hashtable[hashtable.hash_function(16)].key == 16);
        REQUIRE(hashtable.hashtable[hashtable.hash_function(16)].psl == 0);
    }

    SECTION("Non Existant Key"){
        Robinhood hashtable(11);

        REQUIRE(hashtable.find_values(8).empty());

        for(int i = 0; i < 11; ++i){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }
        
        for(int i = 0; i < 11; ++i){
            REQUIRE(!hashtable.find_values(i).empty());
        }

        REQUIRE(hashtable.find_values(11).empty());
        REQUIRE(hashtable.find_values(16).empty());
    }

    SECTION("Hashtable Size"){
        Robinhood hashtable(14);
        REQUIRE(hashtable.N == 32);

        Robinhood ht(16);
        REQUIRE(ht.N == 32);

        Robinhood high(200);
        REQUIRE(high.N == 512);
    }

    SECTION("Find Test"){
        Robinhood hashtable(8);

        hashtable.insert(0, {static_cast<size_t>(0)});
        hashtable.insert(1, {static_cast<size_t>(1)});
        hashtable.insert(8, {static_cast<size_t>(8)});
        hashtable.insert(9, {static_cast<size_t>(9)});
        hashtable.insert(16, {static_cast<size_t>(16)});
        hashtable.insert(17, {static_cast<size_t>(17)});

        // Find should terminate bc our psl > psl[pos]

        // Hash 1
        REQUIRE(hashtable.find_values(25).empty());
        
        // Hash 0
        REQUIRE(hashtable.find_values(24).empty());

    }

    SECTION("Random Insertions"){
        Robinhood hashtable(7);

        // Hash = 0
        hashtable.insert(10, {static_cast<size_t>(100)}); // pos = 0, psl = 0
        hashtable.insert(28, {static_cast<size_t>(280)}); // pos = 1, psl = 1
        hashtable.insert(36, {static_cast<size_t>(360)}); // pos = 2, psl = 2

        REQUIRE(hashtable.hashtable[0].key == 10);
        REQUIRE(hashtable.hashtable[0].psl == 0);
        REQUIRE(hashtable.hashtable[1].key == 28);
        REQUIRE(hashtable.hashtable[1].psl == 1);
        REQUIRE(hashtable.hashtable[2].key == 36);
        REQUIRE(hashtable.hashtable[2].psl == 2);

        // Hash = 1
        hashtable.insert(14, {static_cast<size_t>(140)});  // pos = 3, psl = 2
        hashtable.insert(20, {static_cast<size_t>(200)});  // pos = 4, psl = 3

        REQUIRE(hashtable.hashtable[3].key == 14);
        REQUIRE(hashtable.hashtable[3].psl == 2);
        REQUIRE(hashtable.hashtable[4].key == 20);
        REQUIRE(hashtable.hashtable[4].psl == 3);

        // Hash = 2
        hashtable.insert(13, {static_cast<size_t>(130)});  // pos = 5, psl = 3 
        hashtable.insert(33, {static_cast<size_t>(330)});  // pos = 6, psl = 4 

        REQUIRE(hashtable.hashtable[5].key == 13);
        REQUIRE(hashtable.hashtable[5].psl == 3);
        REQUIRE(hashtable.hashtable[6].key == 33);
        REQUIRE(hashtable.hashtable[6].psl == 4);
    }

    SECTION("Insertions with Identical Key"){
        Robinhood hashtable(24);
        REQUIRE(hashtable.N == 64);

        for(int i = 0; i < 23; ++i){
            hashtable.insert(0, {static_cast<size_t>(i*10)});
        }

        REQUIRE(!hashtable.find_values(0).empty());
        REQUIRE(hashtable.hashtable[0].values.size() == 23);
        REQUIRE(hashtable.hashtable[0].psl == 0);

        for(int i = 0; i < 23; ++i){
            REQUIRE(hashtable.hashtable[0].values[i] == static_cast<size_t>(i * 10));
        }

        hashtable.insert(53, {static_cast<size_t>(10)});
        REQUIRE(hashtable.hashtable[1].key == 53);
        REQUIRE(hashtable.hashtable[1].psl == 0);
    }

    SECTION("Domino Swaps from the End"){
        Robinhood hashtable(4);
        REQUIRE(hashtable.N == 8);

        std::vector<int> keys = {0, 14, 13, 8, 7, 4, 2, 1}; 

        for(int i = 1; i < hashtable.N; ++i){
            hashtable.insert(keys[i], {static_cast<size_t>(keys[i])});
        }

        // Everything should have a PSL = 0 
        for(int i = 1; i < hashtable.N; ++i){
            REQUIRE(hashtable.hashtable[i].key == keys[i]);
            REQUIRE(hashtable.hashtable[i].psl == 0);
        }

        // The first position should be empty, we are going to add something soon though
        REQUIRE(!hashtable.hashtable[0].occupied);

        hashtable.insert(20, {static_cast<size_t>(20)}); // hashes to 1

        // The "first" one escapes bc they have the same PSL, but the "second" one NO, so it begins!
        REQUIRE(hashtable.hashtable[1].key == 14);
        REQUIRE(hashtable.hashtable[1].psl == 0);

        REQUIRE(hashtable.hashtable[2].key == 20);
        REQUIRE(hashtable.hashtable[2].psl == 1);

        for(int i = 3; i < hashtable.N; ++i){
            REQUIRE(hashtable.hashtable[i].key == keys[i-1]);
            REQUIRE(hashtable.hashtable[i].psl == 1);
        }

        // Now 1 should be at pos[0]
        REQUIRE(hashtable.hashtable[0].key == 1);
        REQUIRE(hashtable.hashtable[0].psl == 1);
    }

    SECTION("Inserting a Poor Key into a Rich Cluster"){
        Robinhood hashtable(4);

        // Hash = 0
        std::vector<int> keys = {0, 6, 10, 17, 27, 28, 32};
        for(int i = 0; i < hashtable.N-1; ++i){
            hashtable.insert(keys[i], {static_cast<size_t>(keys[i])});
        }

        // Because all of them are hashing to the same bucket, PSL = i
        for(int i = 0; i < hashtable.N-1; i++){
            REQUIRE(hashtable.hashtable[i].key == keys[i]);
            REQUIRE(hashtable.hashtable[i].psl == i);
        }

        // This one should be free
        REQUIRE(!hashtable.hashtable[hashtable.N-1].occupied);

        hashtable.insert(20, {static_cast<size_t>(20)}); // hashes to 1

        for(int i = 0; i < hashtable.N-1; i++){
            REQUIRE(hashtable.hashtable[i].key == keys[i]);
            REQUIRE(hashtable.hashtable[i].psl == i);
        }

        // The "unlucky" one should be on the very end
        REQUIRE(hashtable.hashtable[hashtable.N-1].key == 20);
        REQUIRE(hashtable.hashtable[hashtable.N-1].psl == (hashtable.N - 2));
    }

    SECTION("Mix Cluster Competition"){
        Robinhood hashtable(4);

        // Inserting 3 with different hash (0, 1 and 2 respectively)
        hashtable.insert(6, {static_cast<size_t>(6)});
        hashtable.insert(20, {static_cast<size_t>(20)});
        hashtable.insert(25, {static_cast<size_t>(25)});

        REQUIRE(hashtable.hashtable[0].psl == 0);
        REQUIRE(hashtable.hashtable[1].psl == 0);
        REQUIRE(hashtable.hashtable[2].psl == 0);
        
        // Hash 0
        hashtable.insert(10, {static_cast<size_t>(10)});

        /*      key:  6, 10, 20, 25
                psl:  0,  1,  1,  1       */
        
        REQUIRE(hashtable.hashtable[0].key == 6);
        REQUIRE(hashtable.hashtable[1].key == 10);
        REQUIRE(hashtable.hashtable[2].key == 20);
        REQUIRE(hashtable.hashtable[3].key == 25);

        // Hash 1
        hashtable.insert(38, {static_cast<size_t>(38)});

        /*      key: 6, 10, 20, 38, 25
                psl: 0,  1,  1,  2,  2   */

        REQUIRE(hashtable.hashtable[2].key == 20);
        REQUIRE(hashtable.hashtable[3].key == 38);
        REQUIRE(hashtable.hashtable[4].key == 25);

        // Hash 2
        hashtable.insert(29, {static_cast<size_t>(29)});

        /*      key: 6, 10, 20, 38, 25, 29
                psl: 0,  1,  1,  2,  2,  3       */
        
        REQUIRE(hashtable.hashtable[5].key == 29);
        REQUIRE(hashtable.hashtable[5].psl == 3);

    }
}

TEST_CASE("Cuckoo Hashing", "[cuckoo]"){

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

TEST_CASE("Hopscotch Hashing", "[hopscotch]"){

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