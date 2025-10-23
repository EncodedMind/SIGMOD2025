// Να κάνω load factor και resize
// -------------------------------
// Robin Hood Hashing
#include <iostream>
#include <vector>
#include <bit>
using namespace std;

struct Robinhood{
    int N;

    struct Entry{
        int key;
        vector<int> values;
        int psl;

        Entry(int k = -1, const vector<int>& vec = {}, int p = -1) : key(k), values(vec), psl(p){};
    };

    vector<Entry> hashtable;

    Robinhood(int size){ // constructor
        N = bit_ceil(static_cast<unsigned>(size));
        hashtable.resize(N);
    }

    int hash_function(int key) const{
        return key & (N-1);
    }

    void insert(int key, const vector<int>& inputvalues){
        int pos = hash_function(key);
        int psl = 0;
        vector<int> values = inputvalues;

        while(1){
            if(hashtable[pos].key == -1){ // empty spot
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

    bool find(int key) const{
        int pos = hash_function(key);
        int psl = 0;

        while(1){
            if(hashtable[pos].key == key) return true; // found
            if(hashtable[pos].key == -1) return false; // empty spot
            if(psl > hashtable[pos].psl) return false; // if the psl of the key we are looking for is greater than the psl of the current element, then it means that the key is not in the table
            psl++;
            if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
        }
    }

    void print() const{
        cout << "Final table: ";
        for(int i=0; i<N; ++i){
            const auto& [key, values, psl] = hashtable[i];
            cout << "Bucket " << i << ": ";
            if(key == -1) cout << "{Empty} ";
            else{
                cout << "{" << key << ": [";
                for(auto v : values) cout << v << " ";
                cout << "]} ";
            }
            cout << "- Psl: " << psl << '\n';
        }
    }
};

int main(){
    
    Robinhood hashtable(10);
    vector<pair<int, int>> data = {
        {10, 100},   // Base case, hashes to 10
        {26, 260},   // Hashes to 10, forces collision
        {42, 420},   // Hashes to 10, forces second collision
        {9, 900},    // Hashes to 9, will wrap around
        {19, 190},   // Hashes to 9, forces wrap-around collision
        {10, 101},   // Duplicate key with new value
        {20, 201},   // Another duplicate key
        {40, 400},   // Hashes to 0, creates maximum PSL
        {50, 500}    // Should trigger multiple Robin Hood steals
    };

    for(auto [key, value] : data){
        hashtable.insert(key, {value});
    }

    hashtable.print();

    cout << (hashtable.find(9) ? "Found 9\n" : "9 not found\n");
    cout << (hashtable.find(100) ? "Found 100\n" : "100 not found\n");

    return 0;
}

// g++ -std=c++20 robinhood.cpp -o robinhood && ./robinhood