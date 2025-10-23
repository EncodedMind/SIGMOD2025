// Να κάνω resize, load factor
// Τι H θα ορίσω;
// --------------------------
// Hopscotch Hashing
#include <iostream>
#include <vector>
#include <bitset>
#include <bit>
using namespace std;

struct Hopscotch{
    static constexpr int H = 4; // Neighborhood size - will be 32 or 64 according to paper
    int N;

    struct Entry{
        int key;
        vector<int> values;
        bitset<H> bitmap;

        Entry(int k = -1, const vector<int>& vec = {}) : key(k), values(vec), bitmap(0){};
    };

    vector<Entry> hashtable;
    
    Hopscotch(int size){ // constructor
        N = bit_ceil(static_cast<unsigned>(size));
        hashtable.resize(N);
    }

    int hash_function(int key) const{
        return (key & (N-1));
    }

    void insert(int key, const vector<int>& inputvalues){
        int i = hash_function(key);
        vector<int> values = inputvalues;

        // If key already exists in the neighborhood, just append
        for(int offset = 0; offset < H; ++offset){
            int pos = ((i + offset) & (N-1));
            if (hashtable[pos].key == key) {
                for (auto v : values) hashtable[pos].values.push_back(v);
                return;
            }
        }

        if(hashtable[i].bitmap.all()){
            cout << "REHASH NEEDED-1\n";
            exit(1);
        }

        int j = i;
        while(hashtable[j].key != -1){ // Find next free spot
            j = ((j+1) & (N-1));
            if(j == i){ // Full table
                cout << "RESIZE NEEDED-2\n";
                exit(2);
            }
        }
        // now j is the free spot

        while(((j-i+N) & (N-1)) >= H){
            int y = -1;

            for(int offset = H-1; offset > 0; --offset){
                int k = ((j - offset + N) & (N-1));
                int element = hashtable[k].key;
                if(element == -1) continue;

                int hash = hash_function(element);
                if(((k-hash+N)&(N-1)) >= H) continue;

                if(((j-hash+N)&(N-1)) < H && hashtable[hash].bitmap.test(H-1-((k-hash+N)&(N-1)))){
                    y = k;
                    break;
                }
            }

            if(y == -1){ // y not found
                cout << "RESIZE NEEDED-3\n";
                exit(3);
            }

            int hashy = hash_function(hashtable[y].key);
            hashtable[j].key = hashtable[y].key;
            hashtable[j].values = hashtable[y].values;
            hashtable[y].key = -1;
            hashtable[y].values.clear();

            hashtable[hashy].bitmap.reset(H-1-((y-hashy+N)&(N-1)));
            hashtable[hashy].bitmap.set(H-1-((j-hashy+N)&(N-1)));
            j = y; // New free spot
        }

        hashtable[j].key = key;
        hashtable[j].values = values;
        hashtable[i].bitmap.set(H-1-((j-i+N)&(N-1)));        
        // cout << "Inserted key " << key << " at position " << j << '\n';
    }

    bool find(int key) const{
        int i = hash_function(key);

        for(int offset = 0; offset < H; ++offset){
            if(hashtable[i].bitmap.test(H-1-offset)){
                int pos = ((i+offset)&(N-1));
                if(hashtable[pos].key == key) return true;
            }
        }
        return false;
    }

    void print() const{
        cout << "Final table: ";
        for(int i=0; i<N; ++i){
            const auto& [key, values, bitmap] = hashtable[i];
            cout << "Bucket " << i << ": ";
            if(key == -1) cout << "{Empty} ";
            else{
                cout << "{" << key << ": [";
                for(auto v : values) cout << v << " ";
                cout << "]} ";
            }
            cout << "- Bitmap: " << bitmap << '\n';
        }
    }
};

int main(){
    Hopscotch hashtable(10);
    vector<pair<int, int>> data = {
    {10, 100},   // Hashes to 0
    {20, 200},   // Hashes to 0, forces first displacement
    {7, 700},    // Hashes to 7
    {17, 170},   // Hashes to 7, forces displacement within H
    {10, 101},   // Tests duplicate key handling
    {27, 270},   // Hashes to 7, but still within H=4
    {40, 400},   // Hashes to 0, tests neighborhood limits
    {8, 800}     // Should cause neighborhood restructuring
    };

    for(auto [key, value] : data){
        hashtable.insert(key, {value});
    }

    hashtable.print();

    cout << (hashtable.find(17) ? "Found 17\n" : "17 not found\n");
    cout << (hashtable.find(100) ? "Found 100\n" : "100 not found\n");

    return 0;
}

// g++ -std=c++20 hopscotch.cpp -o hopscotch && ./hopscotch