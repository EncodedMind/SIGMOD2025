// Να κάνω resize, load factor
// Τι H θα ορίσω;
// --------------------------
// Hopscotch Hashing
#include <iostream>
#include <vector>
#include <bitset>
// #include <functional> // for std::hash
#define N 10 // Size of hash table
using namespace std;

constexpr int H = 4; // Neighborhood size

struct Entry{
    int key;
    vector<int> values;
    bitset<H> bitmap;

    Entry(int k = -1, const vector<int>& vec = {}) : key(k), values(vec), bitmap(0){};
};

int hash_function(int key){
    return key % N;
}

// int hash_function(int key){
//     std::hash<int> hasher;
//     return hasher(key) % N;
// }

void hashtable_insert(vector<Entry>& hashtable, int key, const vector<int>& inputvalues){
    int i = hash_function(key);
    vector<int> values = inputvalues;

    // If key already exists in the neighborhood, just append
    for(int offset = 0; offset < H; ++offset){
        int pos = (i + offset) % N;
        if (hashtable[pos].key == key) {
            for (auto v : values) hashtable[pos].values.push_back(v);
            return;
        }
    }

    if(hashtable[i].bitmap.all()){
        cout << "REHASH NEEDED-1" << endl;
        exit(1);
    }

    int j = i;
    while(hashtable[j].key != -1){ // Find next free spot
        j = (j+1) % N;
        if(j == i){ // Full table
            cout << "RESIZE NEEDED-2" << endl;
            exit(2);
        }
    }
    // now j is the free spot

    while((j+N-i) % N >= H){
        int y = -1;

        for(int offset = H-1; offset > 0; --offset){
            int k = (j - offset + N) % N;
            int element = hashtable[k].key;
            if(element == -1) continue;

            int hash = hash_function(element);
            if((k+N-hash) % N >= H) continue;

            if((j+N-hash) % N < H && hashtable[hash].bitmap.test(H-1-(k-hash+N)%N)){
                y = k;
                break;
            }
        }

        if(y == -1){ // y not found
            cout << "RESIZE NEEDED-3" << endl;
            exit(3);
        }

        int hashy = hash_function(hashtable[y].key);
        hashtable[j].key = hashtable[y].key;
        hashtable[j].values = hashtable[y].values;
        hashtable[y].key = -1;
        hashtable[y].values.clear();

        hashtable[hashy].bitmap.reset(H-1-(y-hashy+N)%N);
        hashtable[hashy].bitmap.set(H-1-(j-hashy+N)%N);
        j = y; // New free spot
    }

    hashtable[j].key = key;
    hashtable[j].values = values;
    hashtable[i].bitmap.set(H-1-(j+N-i)%N);        
    // cout << "Inserted key " << key << " at position " << j << endl;
}

bool hashtable_find(const vector<Entry>& hashtable, int key){
    int i = hash_function(key);

    for(int offset = 0; offset < H; ++offset){
        if(hashtable[i].bitmap.test(H-1-offset)){
            int pos = (i + offset) % N;
            if(hashtable[pos].key == key) return true;
        }
    }
    return false;
}

void hashtable_print(const vector<Entry>& hashtable){
    cout << "Final table: ";
    for(auto [key, values, bitmap] : hashtable){
        cout << "{" << key << ": [";
        for(auto v : values) cout << v << " ";
        cout << "]} ";
    }
    cout << endl;

    cout << "Neighborhood bitsets:\n";
    for(int i=0; i<N; ++i){
        cout << "Bucket " << i << ": " << hashtable[i].bitmap << endl;
    }
}

int main(){
    vector<Entry> hashtable(N);
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
        hashtable_insert(hashtable, key, {value});
    }

    hashtable_print(hashtable);

    cout << (hashtable_find(hashtable, 17) ? "Found 17" : "17 not found") << endl;
    cout << (hashtable_find(hashtable, 100) ? "Found 100" : "100 not found") << endl;

    return 0;
}