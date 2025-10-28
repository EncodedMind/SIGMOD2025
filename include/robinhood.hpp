#ifndef ROBINHOOD_HPP
#define ROBINHOOD_HPP

#include <vector>

struct Robinhood{
    int N;

    struct Entry{
        int key;
        std::vector<int> values;
        int psl;

        Entry(int k = -1, const std::vector<int>& vec = {}, int p = -1) : key(k), values(vec), psl(p){};
    };

    std::vector<Entry> hashtable;

    Robinhood(int size);

    int hash_function(int key) const;

    void insert(int key, const std::vector<int>& inputvalues);

    bool find(int key) const;

    void print() const;

};

#endif