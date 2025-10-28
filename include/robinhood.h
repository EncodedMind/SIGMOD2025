#pragma once
#include <vector>
#include <bit>
#include <functional>

template <typename T>
struct Robinhood{
    size_t N;

    struct Entry{
        T key;
        std::vector<size_t> values;
        int psl;
        bool occupied;

        Entry(T k = T{}, const std::vector<size_t>& vec = {}, int p = -1, bool occ = false) : key(k), values(vec), psl(p), occupied(occ){};
    };

    std::vector<Entry> hashtable;

    Robinhood(size_t size){ // constructor
        N = std::bit_ceil(size);
        hashtable.resize(N, Entry());
    }

    size_t hash_function(const auto& key) const{
        return std::hash<T>{}(key) & (N-1);
    }

    void insert(const auto& inputkey, const std::vector<size_t>& inputvalues){
        T key = inputkey;
        std::vector<size_t> values = inputvalues;
        size_t pos = hash_function(key);
        int psl = 0;
        Entry current(key, values, psl, true);

        while(1){
            if(!hashtable[pos].occupied){ // empty spot
                hashtable[pos] = current;
                hashtable[pos].occupied = true;
                return;
            }
            if(hashtable[pos].key == key){ // key already exists, just add the value
                hashtable[pos].values.insert(hashtable[pos].values.end(), values.begin(), values.end());
                return;
            }
            if(current.psl > hashtable[pos].psl){
                std::swap(hashtable[pos], current);
                hashtable[pos].occupied = true;
            }
            current.psl++;
            if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
        }
    }

    std::vector<size_t> find_values(const T& key) const{
        size_t pos = hash_function(key);
        int psl = 0;

        while(1){
            if(!hashtable[pos].occupied) return {}; // empty spot
            if(hashtable[pos].key == key) return hashtable[pos].values; // found
            if(psl > hashtable[pos].psl) return {}; // if the psl of the key we are looking for is greater than the psl of the current element, then it means that the key is not in the table
            psl++;
            if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
        }
    }
};