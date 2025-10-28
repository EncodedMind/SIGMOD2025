#pragma once
#include <vector>
#include <bit>
#include <functional>
#include <bitset>
#include <stdexcept>
#include <iostream>

template <typename T>
struct Hopscotch{
    static constexpr int H = 64; // Neighborhood size (test with 32 or 64)
    size_t N;

    struct Entry{
        T key;
        std::vector<size_t> values;
        std::bitset<H> bitmap;
        bool occupied;

        Entry(T k = T{}, const std::vector<size_t>& vec = {}, bool occ = false) : key(k), values(vec), bitmap(0), occupied(occ){};
    };

    std::vector<Entry> hashtable;

    Hopscotch(size_t size){ // constructor
        N = std::bit_ceil(size);
        hashtable.resize(N); // hashtable.resize(N, Entry()); grgr
    }

    size_t hash_function(const T& key) const{
        return std::hash<T>{}(key) & (N-1);
    }

    void rehash(){
        std::vector<Entry> old = hashtable;

        N *= 2;

        hashtable.clear();
        hashtable.resize(N);

        for(auto& e : old){
            if(e.occupied) insert(e.key, e.values);
        }
    }

    void insert(const T& inputkey, const std::vector<size_t>& inputvalues){
        T key = inputkey;
        std::vector<size_t> values = inputvalues;
        size_t i = hash_function(key);

        // if key already exists, check both tables and append the value
        for(int offset = 0; offset < H; ++offset){
            size_t pos = (i + offset) & (N-1);
            if(hashtable[pos].occupied && hashtable[pos].key == key){
                hashtable[pos].values.insert(hashtable[pos].values.end(), values.begin(), values.end());
                return;
            }
        }

        if(hashtable[i].bitmap.all()){
            rehash();
            insert(key, values);
            return;
        }

        size_t j = i;
        while(hashtable[j].occupied){ // Find next free spot
            j = (j+1) & (N-1);
            // if(j == i){ // Full table - Will never happen
            //     cout << "RESIZE NEEDED-2\n";
            //     exit(2);
            // }
        }
        // now j is the free spot

        while(((j-i+N) & (N-1)) >= H){
            int y = -1;

            for(int offset = H-1; offset > 0; --offset){
                int k = ((j - offset + N) & (N-1));
                if(!hashtable[k].occupied) continue;

                size_t hash = hash_function(hashtable[k].key);
                if(((k-hash+N)&(N-1)) >= H) continue;

                if(((j-hash+N)&(N-1)) < H && hashtable[hash].bitmap.test((k-hash+N)&(N-1))){
                    y = k;
                    break;
                }
            }

            if(y == -1){  // y not found
                rehash();
                insert(key, values);
                return;
            }

            size_t hashy = hash_function(hashtable[y].key);
            hashtable[j].key = hashtable[y].key;
            hashtable[j].values = hashtable[y].values;
            hashtable[j].occupied = true;
            hashtable[y].occupied = false;
            hashtable[y].values.clear();

            hashtable[hashy].bitmap.reset((y-hashy+N)&(N-1));
            hashtable[hashy].bitmap.set((j-hashy+N)&(N-1));
            j = y; // New free spot
        }

        hashtable[j].key = key;
        hashtable[j].values = values;
        hashtable[j].occupied = true;
        hashtable[i].bitmap.set((j-i+N)&(N-1));
    }

    std::vector<size_t> find_values(const T& key) const{
        size_t i = hash_function(key);

        for(int offset = 0; offset < H; ++offset){
            if(hashtable[i].bitmap.test(offset)){
                size_t pos = (i+offset)&(N-1);
                if(hashtable[pos].occupied && hashtable[pos].key == key) return hashtable[pos].values;
            }
        }
        return {};
    }
};
