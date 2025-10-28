#define CATCH_CONFIG_MAIN
#include "catch_amalgamated.hpp"
#include "robinhood.hpp"

// Tests 
TEST_CASE("Robin Hood Hashing", "[robinhood]"){

    SECTION("Basic Insert and Find"){
        // N = 16
        Robinhood hashtable(11);

        hashtable.insert(1, {10});
        hashtable.insert(16, {20});

        REQUIRE(hashtable.find(1) == true);
        REQUIRE(hashtable.find(16) == true);

        REQUIRE(hashtable.hashtable[1].key == 1);
        REQUIRE(hashtable.hashtable[1].psl == 0);

        REQUIRE(hashtable.hashtable[0].key == 16);
        REQUIRE(hashtable.hashtable[0].psl == 0);
    }

    SECTION("Non Existant Key"){
        Robinhood hashtable(11);

        REQUIRE(hashtable.find(8) == false);

        for(int i = 0; i < 11; ++i){
            hashtable.insert(i, {i*10});
        }
        
        for(int i = 0; i < 11; ++i){
            REQUIRE(hashtable.find(i) == true);
        }

        REQUIRE(hashtable.find(11) == false);
        REQUIRE(hashtable.find(16) == false);
    }

    SECTION("Hashtable Size"){
        Robinhood hashtable(14);
        REQUIRE(hashtable.N == 16);

        Robinhood ht(16);
        REQUIRE(ht.N == 16);

        Robinhood high(200);
        REQUIRE(high.N == 256);
    }

    SECTION("Find Test"){
        Robinhood hashtable(8);

        hashtable.insert(0, {0});
        hashtable.insert(1, {1});
        hashtable.insert(8, {8});
        hashtable.insert(9, {9});
        hashtable.insert(16, {16});
        hashtable.insert(17, {17});

        /* 
        keys: 0, 8, 16 , 9, 1, 17
        psl:  0, 1, 2 , 2,  3, 3
        */

        REQUIRE(hashtable.find(25) == false);
        REQUIRE(hashtable.find(24) == false);

    }

    SECTION("Random Insertions"){
        Robinhood hashtable(11);

        // Hash = 0
        hashtable.insert(16, {160}); // pos = 0, psl = 0
        hashtable.insert(32, {320}); // pos = 1, psl = 1
        hashtable.insert(48, {480}); // pos = 2, psl = 2

        // Hash = 1
        hashtable.insert(1, {10}); // pos = 3, psl = 2
        hashtable.insert(17, {170}); // pos = 4, psl = 3

        // Hash = 2
        hashtable.insert(2, {20}); // pos = 5, psl = 3 
        hashtable.insert(18, {180}); // pos = 6, psl = 4 

        REQUIRE(hashtable.hashtable[0].key == 16);
        REQUIRE(hashtable.hashtable[0].psl == 0);
        REQUIRE(hashtable.hashtable[1].key == 32);
        REQUIRE(hashtable.hashtable[1].psl == 1);
        REQUIRE(hashtable.hashtable[2].key == 48);
        REQUIRE(hashtable.hashtable[2].psl == 2);
        REQUIRE(hashtable.hashtable[3].key == 1);
        REQUIRE(hashtable.hashtable[3].psl == 2);
        REQUIRE(hashtable.hashtable[4].key == 17);
        REQUIRE(hashtable.hashtable[4].psl == 3);
        REQUIRE(hashtable.hashtable[5].key == 2);
        REQUIRE(hashtable.hashtable[5].psl == 3);
        REQUIRE(hashtable.hashtable[6].key == 18);
        REQUIRE(hashtable.hashtable[6].psl == 4);
    }

    SECTION("Insertions with Identical Key"){
        Robinhood hashtable(8);
        REQUIRE(hashtable.N == 8);

        for(int i = 0; i < 23; ++i){
            hashtable.insert(0, {i*10});
        }

        REQUIRE(hashtable.find(0) == true);
        REQUIRE(hashtable.hashtable[0].values.size() == 23);
        REQUIRE(hashtable.hashtable[0].psl == 0);

        for(int i = 0; i < 23; ++i){
            REQUIRE(hashtable.hashtable[0].values[i] == (i * 10));
        }

        hashtable.insert(1, {10});
        REQUIRE(hashtable.hashtable[1].key == 1);
        REQUIRE(hashtable.hashtable[1].psl == 0);
    }

    SECTION("Domino Swaps from the End"){
        Robinhood hashtable(32);

        for(int i = 1; i < hashtable.N; ++i){
            hashtable.insert(i, {i});
        }

        // Everything should have a PSL = 0 
        for(int i = 1; i < hashtable.N; ++i){
            REQUIRE(hashtable.hashtable[i].key == i);
            REQUIRE(hashtable.hashtable[i].psl == 0);
        }

        // The first position should be empty, we are going to add something soon though
        REQUIRE(hashtable.hashtable[0].key == -1);

        hashtable.insert(33, {33});

        // The "first" one escapes bc they have the same PSL, but the "second" one NO, so it begins!
        REQUIRE(hashtable.hashtable[1].key == 1);
        REQUIRE(hashtable.hashtable[1].psl == 0);

        REQUIRE(hashtable.hashtable[2].key == 33);
        REQUIRE(hashtable.hashtable[2].psl == 1);

        for(int i = 3; i < hashtable.N; ++i){
            REQUIRE(hashtable.hashtable[i].key == (i-1));
            REQUIRE(hashtable.hashtable[i].psl == 1);
        }
    }

    SECTION("Inserting a Poor Key into a Rich Cluster"){
        Robinhood hashtable(32);

        // Hash = 0
        for(int i = 0; i < hashtable.N -1; ++i){
            int key = i * hashtable.N;
            hashtable.insert(key, {key});
        }

        // Because all of them are hashing to the same bucket, PSL = i
        for(int i = 0; i < hashtable.N -1; i++){
            REQUIRE(hashtable.hashtable[i].key == (i * hashtable.N));
            REQUIRE(hashtable.hashtable[i].psl == i);
        }

        // This one should be free
        REQUIRE(hashtable.hashtable[31].key == -1);

        hashtable.insert(1, {1});

        for(int i = 0; i < hashtable.N -1; i++){
            REQUIRE(hashtable.hashtable[i].key == (i * hashtable.N));
            REQUIRE(hashtable.hashtable[i].psl == i);
        }

        // The "unlucky" one should be on the very end
        REQUIRE(hashtable.hashtable[31].key == 1);
        REQUIRE(hashtable.hashtable[31].psl == (hashtable.N - 2));
    }
}

