#include <catch2/catch_test_macros.hpp>

#include <random>
#include <chrono>
#include <unordered_set>

#include "hopscotch.h"

/*  ------------------------------------ HELPER FUNCTIONS ------------------------------------------------- */
// Helper function to generate random **keys** based on type
template <typename Key>
Key generateRandomKey(std::mt19937& rng, int i) {
    if constexpr (std::is_same_v<Key, int32_t> || std::is_same_v<Key, int64_t>) {
        std::uniform_int_distribution<Key> dist(1, 1'000'000);
        return dist(rng);
    } else if constexpr (std::is_same_v<Key, double>) {
        std::uniform_real_distribution<double> dist(1.0, 1'000'000.0);
        return dist(rng);
    } else if constexpr (std::is_same_v<Key, std::string>) {
        return std::to_string(i);
    }
}

// Helper function to generate **value** vector based on key type
template <typename Key>
std::vector<size_t> generateValueVector(const Key& key) {
    if constexpr (std::is_same_v<Key, int32_t> || std::is_same_v<Key, int64_t>) {
        return { static_cast<size_t>(key) * 3 };
    } else if constexpr (std::is_same_v<Key, double>) {
        return { static_cast<size_t>(static_cast<int64_t>(key) * 3) };
    } else if constexpr (std::is_same_v<Key, std::string>) {
        return { key.length() * 3 };
    }
}

/*  ------------------------------------ TEST FUNCTIONS -------------------------------------------------- */
template <typename Key>
void testIdenticalKeyInsertions() {
    using Hopscotch = Hashalgorithm<Key>;

    constexpr int inserts = 10;
    Hopscotch hashtable(11);

    Key key{};
    if constexpr (std::is_same_v<Key, int32_t>) key = static_cast<Key>(1);
    else if constexpr (std::is_same_v<Key, int64_t>) key = static_cast<Key>(1LL);
    else if constexpr (std::is_same_v<Key, double>) key = static_cast<Key>(1.0);
    else if constexpr (std::is_same_v<Key, std::string>) key = std::string("string");

    // Insert the identical key multiple times with distinct single-element value vectors
    for (int i = 0; i < inserts; ++i) {
        hashtable.insert(key, { static_cast<size_t>(i * 10) });
    }

    // Retrieve and verify all appended values are present and in insertion order
    auto vals = hashtable.find_values(key);
    REQUIRE(!vals.empty());
    REQUIRE(static_cast<int>(vals.size()) == inserts);

    for (int i = 0; i < inserts; ++i) {
        REQUIRE(vals[i] == static_cast<size_t>(i * 10));
    }
}

TEST_CASE("Identical-key accumulation int32_t", "[instert][identical][int32]") { testIdenticalKeyInsertions<int32_t>(); }
TEST_CASE("Identical-key accumulation int64_t", "[instert][identical][int64]") { testIdenticalKeyInsertions<int64_t>(); }
TEST_CASE("Identical-key accumulation double", "[instert][identical][fp64][double]") { testIdenticalKeyInsertions<double>(); }
TEST_CASE("Identical-key accumulation string", "[instert][identical][varchar][string]") { testIdenticalKeyInsertions<std::string>(); }


// Rehashing with random keys for all supported types (int32_t, int64_t, double, std::string)
// This test may fail occasionally due to the hash table filling up without triggering a rehash. This is expected behavior.
template <typename Key>
void testRehashWithRandomKeys() {
    using Hopscotch = Hashalgorithm<Key>;

    // Seed for random number generation
    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 rng(seed);

    // Initial size for the hash table (Works best for finding the rehashing condition)
    constexpr size_t initialSize = 1024 * 8;
    Hopscotch hashtable(initialSize);
    size_t initialTableSize = hashtable.N;

    std::unordered_set<Key> insertedKeys;
    bool rehashOccurred = false;

    SECTION("Insertions until Rehash") {
        for (int i = 0; i < 20000; ++i) {
            // Generate random key depending on type
            Key key = generateRandomKey<Key>(rng, i);

            if (insertedKeys.count(key)) continue; // Avoid duplicates

            // Create a value vector based on key type 
            std::vector<size_t> valueVec = generateValueVector(key);

            // Insert key-value pair and store key to insertedKeys for later verification
            hashtable.insert(key, valueVec);
            insertedKeys.insert(key);

            // Check if rehash happened by size increase
            if (hashtable.N > initialTableSize) {
                rehashOccurred = true;
                break;
            }

            INFO("Table full No Rehash Occurred\n" <<"Insertion count: " << i << " | Table Size: " << hashtable.N);
            REQUIRE(i < hashtable.N); // Safety clamp (should not exceed table size)
        }
        REQUIRE(rehashOccurred); // Ensure rehashing did occur
    }

    SECTION("Data Integrity Post-Rehash") {
        // Verify all inserted keys are accessible and values match expectation
        for (auto const& key : insertedKeys) {
            auto vals = hashtable.find_values(key);
            REQUIRE(!vals.empty()); // Key should exist in the table

            // Find the expected value based on key type
            auto expectedValue = generateValueVector(key);

            // Check that the retrieved value matches the expected value
            REQUIRE(vals == expectedValue);
        }
    }
}


TEST_CASE("Rehash on randomised int32_t keys", "[rehash][random][int32]") { testRehashWithRandomKeys<int32_t>(); }
TEST_CASE("Rehash on randomised int64_t keys", "[rehash][random][int64]") { testRehashWithRandomKeys<int64_t>(); }
TEST_CASE("Rehash on randomised double keys", "[rehash][random][fp64][double]") { testRehashWithRandomKeys<double>(); }
TEST_CASE("Rehash on randomised string keys", "[rehash][random][varchar][string]") { testRehashWithRandomKeys<std::string>(); }


template <typename Key>
void testCollisionHandling() {
    using Hopscotch = Hashalgorithm<Key>;

    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 rng(seed);

    constexpr size_t tableSize = 1024;
    Hopscotch hashtable(tableSize);

    bool collisionDetected = false;
    Key collidedKey{};
    size_t collidedPos = 0;
    Key prevKey{};

    std::vector<size_t> prevValues;
    std::vector<size_t> collidedValues;

    int i = 1;
    // Insert keys until a collision is detected
    while (!collisionDetected && i < tableSize) {
        Key key = generateRandomKey<Key>(rng, i);
        std::vector<size_t> valueVec = generateValueVector(key);

        size_t pos = hashtable.hash_function(key);

        
        if (hashtable.hashtable[pos].occupied) {
            if (hashtable.hashtable[pos].key == key) continue;// Key already exists, skip

            // Key hashes to an occupied position - collision detected
            collisionDetected = true;
            collidedKey = key;
            collidedPos = pos;
            prevKey = hashtable.hashtable[pos].key;
            prevValues = hashtable.hashtable[pos].values;
            collidedValues = valueVec;
            hashtable.insert(key, valueVec);
            break;
        } else {
            hashtable.insert(key, valueVec);
        }
        ++i;
    }

    
    REQUIRE(collisionDetected);

    SECTION("Verify keys presence post-collision") {
        auto prevFoundVals = hashtable.find_values(prevKey);
        REQUIRE(!prevFoundVals.empty());
        REQUIRE(prevFoundVals == prevValues);

        auto collidedVals = hashtable.find_values(collidedKey);
        REQUIRE(!collidedVals.empty());
        REQUIRE(collidedVals == collidedValues);

        size_t startPos = hashtable.hash_function(collidedKey);
        size_t actualPos = hashtable.N;
        for (size_t p = 0; p < hashtable.N; ++p) {
            if (hashtable.hashtable[p].occupied && hashtable.hashtable[p].key == collidedKey) {
                actualPos = p;
                break;
            }
        }
        REQUIRE(actualPos != hashtable.N);

        size_t dist = (actualPos + hashtable.N - startPos) % hashtable.N;
        REQUIRE(dist < hashtable.H);
    }
}

// Collision tests for all supported key types
TEST_CASE("Collision Handling int32_t", "[collision][int32]") { testCollisionHandling<int32_t>(); }
TEST_CASE("Collision Handling int64_t", "[collision][int64]") { testCollisionHandling<int64_t>(); }
TEST_CASE("Collision Handling double", "[collision][fp64][double]") { testCollisionHandling<double>(); }
TEST_CASE("Collision Handling string", "[collision][varchar][string]") { testCollisionHandling<std::string>(); }


/*  Specific Edge Case Test for Full Neighborhood Rehash
    This test includes 32 picked keys that hash to the same position to fill the neighborhood and trigger a rehash */
TEST_CASE ("Full Neighborhood Rehash", "[rehash][neighborhood]") {
    using Hopscotch = Hashalgorithm<int32_t>;
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

        // Verify that all keys hash to the same position and that their neighborhood is full
        for (int i=0; i< 32; ++i) {
            size_t pos = hashtable.hash_function(keys[i]);
            REQUIRE(hashtable.hashtable[pos].bitmap.all() == true);
        }

        // Insert the 33rd key -> should trigger a rehash because neighborhood is full
        hashtable.insert(keys[32], {static_cast<size_t>(keys[32]*10)});
        REQUIRE(hashtable.N > 64); // Verify that resizing happened

        // Verify all keys are still present
        for(int i = 0; i < 33; ++i){
            auto vals = hashtable.find_values(keys[i]);
            REQUIRE(!vals.empty());
            REQUIRE(vals[0] == static_cast<size_t>(keys[i]*10));
        }
    }
}