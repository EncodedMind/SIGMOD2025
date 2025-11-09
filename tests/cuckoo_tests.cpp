#include <catch2/catch_test_macros.hpp>

#include <table.h>
#include <plan.h>
#include "cuckoo.h"

TEST_CASE("Cuckoo Hashing", "[cuckoo]"){

    using Cuckoo = Hashalgorithm<int32_t>;

    SECTION("Basic Insert and Find"){
        Cuckoo hashtable(11);
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
        Cuckoo hashtable(8);

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
        
        REQUIRE(hashtable.hashtable1.size() == 8);
        REQUIRE(hashtable.hashtable2.size() == 8);
        REQUIRE(hashtable.inserted == 9);
    }

    SECTION("Non-Stop Resizing"){
        Cuckoo hashtable(32);
        REQUIRE(hashtable.N == 32);

        for(int i = 0; i <= 32; i++){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }

        /* Next insertion is going to trigger a resize (we are working with kicks for rehashing 
            but usually its happening with a load factor of 0.565)*/
        hashtable.insert(100, {static_cast<size_t>(100)});
        REQUIRE(hashtable.N == 64);

        for(int i = 0; i <= 256; i++){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }

        // Now we are going to do two in a row
        REQUIRE(hashtable.N == 256);
    }

    SECTION("Random Insertions"){
        Cuckoo hashtable(1024);
        REQUIRE(hashtable.N == 1024);

        for(int i = 0; i < 1024; i++){
            hashtable.insert(i, {static_cast<size_t>(i*10)});
        }

        for(int i = 0; i < 1024; i++){
            REQUIRE(!hashtable.find_values(i).empty());
            REQUIRE(hashtable.find_values(i)[0] == static_cast<size_t>(i*10));
        }

        REQUIRE(hashtable.inserted == 1024);
        REQUIRE(hashtable.N == 1024);
    
    }

    SECTION("Many Collisions - Resize"){
        Cuckoo hashtable(4);
        REQUIRE(hashtable.N == 4);

        // h1 = 1 and h2 = 0        
        hashtable.insert(1, {static_cast<size_t>(10)});
        REQUIRE(hashtable.inserted == 1);

        // h1 = 1 and h2 = 0
        hashtable.insert(5, {static_cast<size_t>(50)});

        // Resize must NOT happen
        REQUIRE(hashtable.N == 4);
        REQUIRE(hashtable.inserted == 2);

        // h1 = 1 and h2 = 0
        hashtable.insert(9, {static_cast<size_t>(90)});

        // Resize MUST happen
        REQUIRE(hashtable.N == 8);
        REQUIRE(hashtable.inserted == 3);        

        // h1 = 6
        hashtable.insert(14, {static_cast<size_t>(140)});

        // Resize must NOT happen
        REQUIRE(hashtable.N == 8);
        REQUIRE(hashtable.inserted == 4);
        REQUIRE(!hashtable.find_values(1).empty());
        REQUIRE(!hashtable.find_values(5).empty());
        REQUIRE(!hashtable.find_values(9).empty());
        REQUIRE(!hashtable.find_values(14).empty());
    }

    SECTION("Collision"){
        Cuckoo hashtable(4);
        REQUIRE(hashtable.N == 4);
        REQUIRE(hashtable.hashtable1.size() == 4);
        REQUIRE(hashtable.hashtable1.size() == 4);

        // h1 = 1 and h2 = 0        
        hashtable.insert(1, {static_cast<size_t>(100)});
        REQUIRE(hashtable.inserted == 1);

        // Hand picking our insertions to test the very worst occasions
        /* 262145 = 1 + (4 << 16)
            h1 = 1 and h2 = 0 */
        hashtable.insert(262145, {static_cast<size_t>(200)});
    
        // Resize must NOT happen
        REQUIRE(hashtable.hashtable1.size() == 4);
        REQUIRE(hashtable.hashtable2.size() == 4);
        REQUIRE(hashtable.N == 4);
        REQUIRE(hashtable.inserted == 2);

        REQUIRE(!hashtable.find_values(1).empty());
        REQUIRE(!hashtable.find_values(262145).empty());
 
    }

}