// Robin Hood Hashing
#include <iostream>
#include <vector>
#include <bit>
#include <functional>
#include <iostream>

#include "../include/MapInterface.h"

template<typename Key, typename Value>
class RobinhoodMap : public MapInterface<Key, Value> {
public:
    struct Entry {
        Key key;
        std::vector<Value> values;
        int psl;
        bool occupied;
        Entry() : key(0), values(), psl(-1), occupied(false) {}
    };

    RobinhoodMap(size_t size = 16) {
        N = std::bit_ceil(static_cast<unsigned>(size));
        hashtable.resize(N);
    }

    ~RobinhoodMap() override = default;

    void insert(const Key& key, const Value& value) override {
        int pos = hash_function(key);
        int psl = 0;

        while (true) {
            if (!hashtable[pos].occupied) { // empty spot
                hashtable[pos] = Entry{key, {value}, psl, true};
                break;
            } else {
                if (hashtable[pos].key == key) { // key already exists, just add the value
                    for(auto& v : hashtable[pos].values) hashtable[pos].values.push_back(v);
                    break;
                }
                if (psl > hashtable[pos].psl) {
                    // Robin Hood swap
                    Entry temp = hashtable[pos];
                    hashtable[pos] = Entry{key, {value}, psl, true};
                    key = temp.key;
                    value_holder = temp.values;
                    // value_holder = std::move(temp.values);
                    psl = temp.psl;
                    // ensure next loop inserts the displaced entry using value_holder.front()
                } else {
                    ++psl;
                    pos = (pos + 1) % N;
                }
            }

        //     // If we have a displaced vector of values to insert, try to re-insert them
        //     if (!value_holder.empty()) {
        //         // insert all values from value_holder for current key (displaced)
        //         // we re-run the outer logic with this key and the first value; remaining values will be appended when the key is found
        //         Key displacedKey = key;
        //         std::vector<Value> displacedValues = std::move(value_holder);
        //         value_holder.clear();

        //         // try to place displacedKey with its values
        //         size_t j = hash_function(displacedKey);
        //         int pslj = 0;
        //         while (true) {
        //             if (!hashtable[j].occupied) {
        //                 hashtable[j].key = displacedKey;
        //                 hashtable[j].values = std::move(displacedValues);
        //                 hashtable[j].psl = pslj;
        //                 hashtable[j].occupied = true;
        //                 break;
        //             }
        //             if (hashtable[j].key == displacedKey) {
        //                 // append all displaced values
        //                 for (int v : displacedValues) hashtable[j].values.push_back(v);
        //                 break;
        //             }
        //             if (pslj > hashtable[j].psl) {
        //                 Entry temp2 = hashtable[j];
        //                 hashtable[j].key = displacedKey;
        //                 hashtable[j].values = std::move(displacedValues);
        //                 hashtable[j].psl = pslj;
        //                 hashtable[j].occupied = true;

        //                 displacedKey = temp2.key;
        //                 displacedValues = std::move(temp2.values);
        //                 pslj = temp2.psl;
        //             } else {
        //                 ++pslj;
        //                 j = (j + 1) % N;
        //             }
        //         }
        //         break;
        //     }
        // }
    }

    int* find(const int& key) override {
        size_t pos = hash_function(key);
        int psl = 0;

        while (true) {
            if (!hashtable[pos].occupied) return nullptr;
            if (hashtable[pos].key == key) {
                if (!hashtable[pos].values.empty())
                    return &hashtable[pos].values[0];
                return nullptr;
            }
            if (psl > hashtable[pos].psl) return nullptr;
            ++psl;
            pos = (pos + 1) % N;
        }
    }

    // // optional debug
    // void print() const {
    //     std::cout << "Final table:\n";
    //     for (size_t i = 0; i < N; ++i) {
    //         const auto& e = hashtable[i];
    //         std::cout << "Bucket " << i << ": ";
    //         if (!e.occupied) std::cout << "{Empty} ";
    //         else {
    //             std::cout << "{" << e.key << ": [";
    //             for (auto v : e.values) std::cout << v << " ";
    //             std::cout << "]} ";
    //         }
    //         std::cout << "- Psl: " << e.psl << '\n';
    //     }
    // }

private:
    size_t N;
    std::vector<Entry> hashtable;
    // temporary holder used during displacement to carry multiple values
    std::vector<int> value_holder;

    size_t hash_function(int key) const {
        return std::hash<int>{}(key) & (N - 1);
    }
};

// Factory specialization for int,int
template<>
MapInterface<int,int>* createMap<int,int>() {
    return new RobinhoodMap();
}
