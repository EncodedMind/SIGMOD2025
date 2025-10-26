#pragma once
#include <vector>
#include <bit>
#include <functional>

template <typename T>
struct Robinhood{
    int N;

    struct Entry{
        T key;
        std::vector<size_t> values;
        int psl;

        Entry(T k = T{}, const std::vector<size_t>& vec = {}, int p = -1) : key(k), values(vec), psl(p){};
    };

    std::vector<Entry> hashtable;

    Robinhood(size_t size){ // constructor
        N = std::bit_ceil(size);
        hashtable.resize(N);
    }

    size_t hash_function(const auto& key) const{
        return std::hash<T>{}(key) & (N-1);
    }

    void insert(const auto& inputkey, const std::vector<size_t>& inputvalues){

        T key = inputkey;
        size_t pos = hash_function(key);
        int psl = 0;
        std::vector<size_t> values = inputvalues;

        while(1){
            if(hashtable[pos].psl == -1){ // empty spot
                hashtable[pos] = Entry(key, values, psl);
                break;
            }
            else{
                if(hashtable[pos].key == key){ // key already exists, just add the value
                    for(auto& v : values) hashtable[pos].values.push_back(v);
                    break;
                }
                if(psl > hashtable[pos].psl){
                    Entry temp = hashtable[pos];
                    hashtable[pos] = Entry(key, values, psl);
                    key = temp.key;
                    values = temp.values;
                    psl = temp.psl;
                }
                psl++;
                if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
            }
        }
    }

    int find(const auto& key) const{
        size_t pos = hash_function(key);
        int psl = 0;

        while(1){
            if(hashtable[pos].key == key) return pos; // found
            if(hashtable[pos].psl == -1) return -1; // empty spot
            if(psl > hashtable[pos].psl) return -1; // if the psl of the key we are looking for is greater than the psl of the current element, then it means that the key is not in the table
            psl++;
            if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
        }
    }
};