#include <catch2/catch_test_macros.hpp>
#include <random>
#include <chrono>
#include <unordered_set>

#include "hopscotch.h"

// Helper function to test rehashing with random keys for all supported types (int32_t, int64_t, double, std::string)
template <typename Key>
void testRehashWithRandomKeys() {
    using Hopscotch = Hashalgorithm<Key>;

    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 rng(seed);

    constexpr size_t initialSize = 1024 * 8; // Initial size for the hash table (Works best for finding the rehashing condition)
    Hopscotch hashtable(initialSize);
    size_t initialTableSize = hashtable.N;

    std::unordered_set<Key> insertedKeys;
    bool rehashOccurred = false;

    // Distribution depends on Key type: for string, generate strings; for numbers fill numeric dist
    auto generateKey = [&](int i) -> Key {
        if constexpr (std::is_same_v<Key, int32_t> || std::is_same_v<Key, int64_t>) {
            std::uniform_int_distribution<Key> dist(1, 1'000'000);
            return dist(rng); // Generate random integer
        } else if constexpr (std::is_same_v<Key, double>) {
            std::uniform_real_distribution<double> dist(1.0, 1'000'000.0);
            return dist(rng);  // Generate random double
        } else if constexpr (std::is_same_v<Key, std::string>) {
            return std::to_string(i); // Simple string generation based on index
        } else {
            static_assert(false, "Unsupported Key type in test");
        }
    };

    SECTION("Insertions until Rehash") {
        for (int i = 0; i < 20000; ++i) {
            Key key = generateKey(i);

            if (insertedKeys.count(key)) continue; // Avoid duplicates

            // Create a value vector based on key type
            std::vector<size_t> valueVec;
            if constexpr (std::is_same_v<Key, int32_t> || std::is_same_v<Key, int64_t>) {
                valueVec = { static_cast<size_t>(key) * 3 };
            } else if constexpr (std::is_same_v<Key, double>) {
                valueVec = { static_cast<size_t>(static_cast<int64_t>(key) * 3) };
            } else if constexpr (std::is_same_v<Key, std::string>) {
                valueVec = { key.length() * 3 };
            }

         

            // Insert key-value pair and store key to insertedKeys for later verification
            hashtable.insert(key, valueVec);
            insertedKeys.insert(key);

            if (hashtable.N > initialTableSize) {
                rehashOccurred = true;
                break;
            }

            REQUIRE(i < hashtable.N); // Safety clamp (should not exceed table size)
            
        }
        REQUIRE(rehashOccurred); // Ensure rehashing did occur
    }

    SECTION("Data Integrity Post-Rehash") {

        for (auto const& key : insertedKeys) {

            auto vals = hashtable.find_values(key);

            REQUIRE(!vals.empty()); // Key should exist in the table

            // Find the expected value based on key type
            size_t expectedValue;
            if constexpr (std::is_same_v<Key, int32_t> || std::is_same_v<Key, int64_t>) {
                expectedValue = static_cast<size_t>(key) * 3;
            } else if constexpr (std::is_same_v<Key, double>) {
                expectedValue = static_cast<size_t>(static_cast<int64_t>(key) * 3);
            } else if constexpr (std::is_same_v<Key, std::string>) {
                expectedValue = key.length() * 3;
            }

            // Check that the retrieved value matches the expected value
            REQUIRE(vals.front() == expectedValue); 
        }
    }
}

template <typename Key>
void test

TEST_CASE("Rehash on randomised int32_t keys", "[rehash][random][int32]") { testRehashWithRandomKeys<int32_t>(); }
TEST_CASE("Rehash on randomised int64_t keys", "[rehash][random][int64]") { testRehashWithRandomKeys<int64_t>(); }
TEST_CASE("Rehash on randomised double keys", "[rehash][random][fp64]") { testRehashWithRandomKeys<double>(); }
TEST_CASE("Rehash on randomised string keys", "[rehash][random][varchar]") { testRehashWithRandomKeys<std::string>(); }


TEST_CASE("Collision Handling", "[collision]"){
    using Hopscotch = Hashalgorithm<int32_t>;

    size_t tableSize = 1028;
    Hopscotch hashtable(tableSize);
    int collisionCount = 0;

    int i = 0;
    while (i < tableSize && !collisionCount) {
        size_t pos = hashtable.hash_function(i);
        if (hashtable.hashtable[pos].occupied) {
            INFO("Collision detected for key " << i << " at position " << pos << "\n");
            collisionCount++;
        }
        hashtable.insert(i, {static_cast<size_t>(i*10)});
        i++;
    }

    // Verify that all keys are placed within neighborhood distance when collisions occur
    SECTION("Collisions") {
        
        REQUIRE(collisionCount > 0);

        for (int i = 4; i < 256; i += 4) {
            size_t startPos = hashtable.hash_function(i);

            // Find actual position of key i in the table
            size_t actualPos = hashtable.N; 
            for (size_t pos = 0; pos < hashtable.N; ++pos) {
                if (hashtable.hashtable[pos].occupied && hashtable.hashtable[pos].key == i) {
                    actualPos = pos;
                    break;
                }
            }

            REQUIRE(actualPos != hashtable.N); // Verify Key Existence

            size_t dist = (actualPos + hashtable.N - startPos) % hashtable.N;
            REQUIRE(dist < hashtable.H); // Key is within neighborhood distance
        }
    }

    // Verify all inserted keys and values are correct
    SECTION("Proper Key Value input") {
        for (int i = 4; i< 256; i+=4) {
            auto values = hashtable.find_values(i);
            REQUIRE(!values.empty());
            REQUIRE(values[0] == static_cast<size_t>(i*10));
        }
    }
}