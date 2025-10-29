#pragma once
#include <vector>
#include <bit>
#include <functional>

template <typename T>
struct Hashalgorithm{
    size_t N;
    size_t inserted;

    struct Entry{
        T key;
        std::vector<size_t> values;
        bool occupied;

        Entry(T k = T{}, const std::vector<size_t>& vec = {}, bool occ = false) : key(k), values(vec), occupied(occ){}
    };

    std::vector<Entry> hashtable1, hashtable2;

    Hashalgorithm(size_t size){ // constructor
        N = std::bit_ceil(size);
        hashtable1.resize(N, Entry());
        hashtable2.resize(N, Entry());
        inserted = 0;
    }

    size_t hash_function1(const auto& key) const{
        return std::hash<T>{}(key) & (N-1);
    }

    size_t hash_function2(const auto& key) const{
        return (std::hash<T>{}(key) >> 16) & (N-1);
    }

    void rehash(){
        std::vector<Entry> old1 = hashtable1;
        std::vector<Entry> old2 = hashtable2;

        N *= 2;
        inserted = 0;

        hashtable1.clear();
        hashtable1.resize(N);
        hashtable2.clear();
        hashtable2.resize(N);

        for(auto& e : old1){
            if(e.occupied) insert(e.key, e.values);
        }
        for(auto& e : old2){
            if(e.occupied) insert(e.key, e.values);
        }
    }

    void insert(const T& inputkey, const std::vector<size_t>& inputvalues){
        T key = inputkey;
        std::vector<size_t> values = inputvalues;
        bool table1 = true;
        Entry current(key, values, true);

        int kicks = 0;

        // if key already exists, check both tables and append the value
        size_t hash1 = hash_function1(key);
        size_t hash2 = hash_function2(key);

        if(hashtable1[hash1].occupied && hashtable1[hash1].key == key){
            hashtable1[hash1].values.insert(hashtable1[hash1].values.end(), values.begin(), values.end());
            return;
        }
        
        if(hashtable2[hash2].occupied && hashtable2[hash2].key == key){
            hashtable2[hash2].values.insert(hashtable2[hash2].values.end(), values.begin(), values.end());
            return;
        }

        while(1){
            if(table1){
                size_t hash1 = hash_function1(current.key);
                if(!hashtable1[hash1].occupied){
                    hashtable1[hash1] = current;
                    hashtable1[hash1].occupied = true;
                    inserted++;
                    return;
                }
                std::swap(hashtable1[hash1], current);
                hashtable1[hash1].occupied = true;

                kicks++;
                table1 = false;
            }
            else{
                size_t hash2 = hash_function2(current.key);
                if(!hashtable2[hash2].occupied){
                    hashtable2[hash2] = current;
                    hashtable2[hash2].occupied = true;
                    inserted++;
                    return;
                }
                std::swap(hashtable2[hash2], current);
                hashtable2[hash2].occupied = true;

                kicks++;
                table1 = true;
            }

            if(kicks >= inserted){
                rehash();
                insert(current.key, current.values);
                return;
            }
        }
    }

    std::vector<size_t> find_values(const T& key) const{
        size_t pos1 = hash_function1(key);
        if(hashtable1[pos1].occupied && hashtable1[pos1].key == key) return hashtable1[pos1].values;
        size_t pos2 = hash_function2(key);
        if(hashtable2[pos2].occupied && hashtable2[pos2].key == key) return hashtable2[pos2].values;
        return {};
    }
};