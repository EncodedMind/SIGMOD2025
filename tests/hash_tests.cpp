#include <catch2/catch_test_macros.hpp>

#include <table.h>
#include <plan.h>
// Uncomment the algorithm to use for testing (only one can be uncommented)
// #include "robinhood.h"
#include "cuckoo.h"


// Robin Hood Tests

// TEST_CASE("Robin Hood Hashing", "[robinhood]"){

//     using Robinhood = Hashalgorithm<int32_t>;

//     SECTION("Basic Insert and Find"){
//         // N = 16
//         Robinhood hashtable(11);

//         hashtable.insert(1, {static_cast<size_t>(10)});
//         hashtable.insert(16, {static_cast<size_t>(20)});

//         REQUIRE(!hashtable.find_values(1).empty());
//         REQUIRE(!hashtable.find_values(16).empty());

//         REQUIRE(hashtable.hashtable[1].key == 1);
//         REQUIRE(hashtable.hashtable[1].psl == 0);

//         REQUIRE(hashtable.hashtable[0].key == 16);
//         REQUIRE(hashtable.hashtable[0].psl == 0);
//     }

//     SECTION("Non Existant Key"){
//         Robinhood hashtable(11);

//         REQUIRE(hashtable.find_values(8).empty());

//         for(int i = 0; i < 11; ++i){
//             hashtable.insert(i, {static_cast<size_t>(i*10)});
//         }
        
//         for(int i = 0; i < 11; ++i){
//             REQUIRE(!hashtable.find_values(i).empty());
//         }

//         REQUIRE(hashtable.find_values(11).empty());
//         REQUIRE(hashtable.find_values(16).empty());
//     }

//     SECTION("Hashtable Size"){
//         Robinhood hashtable(14);
//         REQUIRE(hashtable.N == 16);

//         Robinhood ht(16);
//         REQUIRE(ht.N == 16);

//         Robinhood high(200);
//         REQUIRE(high.N == 256);
//     }

//     SECTION("Find Test"){
//         Robinhood hashtable(8);

//         hashtable.insert(0, {static_cast<size_t>(0)});
//         hashtable.insert(1, {static_cast<size_t>(1)});
//         hashtable.insert(8, {static_cast<size_t>(8)});
//         hashtable.insert(9, {static_cast<size_t>(9)});
//         hashtable.insert(16, {static_cast<size_t>(16)});
//         hashtable.insert(17, {static_cast<size_t>(17)});

//         /*  keys: 0, 8, 16, 9, 1, 17
//             psl:  0, 1,  2, 2, 3,  4    */

//         // Find should terminate bc our psl > psl[pos]

//         // Hash 1
//         REQUIRE(hashtable.find_values(25).empty());
        
//         // Hash 0
//         REQUIRE(hashtable.find_values(24).empty());

//     }

//     SECTION("Random Insertions"){
//         Robinhood hashtable(11);

//         // Hash = 0
//         hashtable.insert(16, {static_cast<size_t>(160)}); // pos = 0, psl = 0
//         hashtable.insert(32, {static_cast<size_t>(320)}); // pos = 1, psl = 1
//         hashtable.insert(48, {static_cast<size_t>(480)}); // pos = 2, psl = 2

//         // Hash = 1
//         hashtable.insert(1, {static_cast<size_t>(10)}); // pos = 3, psl = 2
//         hashtable.insert(17, {static_cast<size_t>(170)}); // pos = 4, psl = 3

//         // Hash = 2
//         hashtable.insert(2, {static_cast<size_t>(20)}); // pos = 5, psl = 3 
//         hashtable.insert(18, {static_cast<size_t>(180)}); // pos = 6, psl = 4 

//         REQUIRE(hashtable.hashtable[0].key == 16);
//         REQUIRE(hashtable.hashtable[0].psl == 0);
//         REQUIRE(hashtable.hashtable[1].key == 32);
//         REQUIRE(hashtable.hashtable[1].psl == 1);
//         REQUIRE(hashtable.hashtable[2].key == 48);
//         REQUIRE(hashtable.hashtable[2].psl == 2);
//         REQUIRE(hashtable.hashtable[3].key == 1);
//         REQUIRE(hashtable.hashtable[3].psl == 2);
//         REQUIRE(hashtable.hashtable[4].key == 17);
//         REQUIRE(hashtable.hashtable[4].psl == 3);
//         REQUIRE(hashtable.hashtable[5].key == 2);
//         REQUIRE(hashtable.hashtable[5].psl == 3);
//         REQUIRE(hashtable.hashtable[6].key == 18);
//         REQUIRE(hashtable.hashtable[6].psl == 4);
//     }

//     SECTION("Insertions with Identical Key"){
//         Robinhood hashtable(8);
//         REQUIRE(hashtable.N == 8);

//         for(int i = 0; i < 23; ++i){
//             hashtable.insert(0, {static_cast<size_t>(i*10)});
//         }

//         REQUIRE(!hashtable.find_values(0).empty());
//         REQUIRE(hashtable.hashtable[0].values.size() == 23);
//         REQUIRE(hashtable.hashtable[0].psl == 0);

//         for(int i = 0; i < 23; ++i){
//             REQUIRE(hashtable.hashtable[0].values[i] == static_cast<size_t>(i * 10));
//         }

//         hashtable.insert(1, {static_cast<size_t>(10)});
//         REQUIRE(hashtable.hashtable[1].key == 1);
//         REQUIRE(hashtable.hashtable[1].psl == 0);
//     }

//     SECTION("Domino Swaps from the End"){
//         Robinhood hashtable(32);

//         for(int i = 1; i < hashtable.N; ++i){
//             hashtable.insert(i, {static_cast<size_t>(i)});
//         }

//         // Everything should have a PSL = 0 
//         for(int i = 1; i < hashtable.N; ++i){
//             REQUIRE(hashtable.hashtable[i].key == i);
//             REQUIRE(hashtable.hashtable[i].psl == 0);
//         }

//         // The first position should be empty, we are going to add something soon though
//         REQUIRE(!hashtable.hashtable[0].occupied);

//         hashtable.insert(33, {static_cast<size_t>(33)});

//         // The "first" one escapes bc they have the same PSL, but the "second" one NO, so it begins!
//         REQUIRE(hashtable.hashtable[1].key == 1);
//         REQUIRE(hashtable.hashtable[1].psl == 0);

//         REQUIRE(hashtable.hashtable[2].key == 33);
//         REQUIRE(hashtable.hashtable[2].psl == 1);

//         for(int i = 3; i < hashtable.N; ++i){
//             REQUIRE(hashtable.hashtable[i].key == (i-1));
//             REQUIRE(hashtable.hashtable[i].psl == 1);
//         }

//         // Now 31 should be at pos[0]
//         REQUIRE(hashtable.hashtable[0].key == 31);
//         REQUIRE(hashtable.hashtable[0].psl == 1);
//     }

//     SECTION("Inserting a Poor Key into a Rich Cluster"){
//         Robinhood hashtable(32);

//         // Hash = 0
//         for(int i = 0; i < hashtable.N-1; ++i){
//             int32_t key = i * hashtable.N;
//             hashtable.insert(key, {static_cast<size_t>(key)});
//         }

//         // Because all of them are hashing to the same bucket, PSL = i
//         for(int i = 0; i < hashtable.N-1; i++){
//             REQUIRE(hashtable.hashtable[i].key == (i * hashtable.N));
//             REQUIRE(hashtable.hashtable[i].psl == i);
//         }

//         // This one should be free
//         REQUIRE(!hashtable.hashtable[31].occupied);

//         hashtable.insert(1, {static_cast<size_t>(1)});

//         for(int i = 0; i < hashtable.N -1; i++){
//             REQUIRE(hashtable.hashtable[i].key == (i * hashtable.N));
//             REQUIRE(hashtable.hashtable[i].psl == i);
//         }

//         // The "unlucky" one should be on the very end
//         REQUIRE(hashtable.hashtable[31].key == 1);
//         REQUIRE(hashtable.hashtable[31].psl == (hashtable.N - 2));
//     }

//     SECTION("Mix Cluster Competition"){
//         Robinhood hashtable(16);

//         // Inserting 3 with different hash (0, 1 and 2 respectively)
//         hashtable.insert(16, {static_cast<size_t>(16)});
//         hashtable.insert(1, {static_cast<size_t>(1)});
//         hashtable.insert(2, {static_cast<size_t>(2)});

//         REQUIRE(hashtable.hashtable[0].psl == 0);
//         REQUIRE(hashtable.hashtable[1].psl == 0);
//         REQUIRE(hashtable.hashtable[2].psl == 0);
        
//         // Hash 0
//         hashtable.insert(32, {static_cast<size_t>(32)});

//         /*      key: 16, 32, 1, 2
//                 psl:  0,  1, 1, 1       */
        
//         REQUIRE(hashtable.hashtable[0].key == 16);
//         REQUIRE(hashtable.hashtable[1].key == 32);
//         REQUIRE(hashtable.hashtable[2].key == 1);
//         REQUIRE(hashtable.hashtable[3].key == 2);

//         // Hash 1
//         hashtable.insert(17, {static_cast<size_t>(17)});

//         /*      key: 16, 32, 1, 17, 2
//                 psl:  0,  1, 1,  2, 2   */
        
//         REQUIRE(hashtable.hashtable[2].key == 1);
//         REQUIRE(hashtable.hashtable[3].key == 17);
//         REQUIRE(hashtable.hashtable[4].key == 2);

//         // Hash 2
//         hashtable.insert(18, {static_cast<size_t>(18)});

//         /*      key: 16, 32, 1, 17, 2, 18
//                 psl:  0,  1, 1,  2, 2,  3       */
        
//         REQUIRE(hashtable.hashtable[5].key == 18);
//         REQUIRE(hashtable.hashtable[5].psl == 3);

//     }
// }


// Cuckoo Tests

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