// Να κάνω resize, load factor
// Τι H θα ορίσω;
// --------------------------
// Hopscotch Hashing
#include <iostream>
#include <vector>
#include <bitset>
#define N 10 // Size of hash table = size of keys + 1
using namespace std;

constexpr int H = 3; // Neighborhood length

struct Entry{
    char key;
    int value;
    bitset<H> bitmap;

    Entry(char k = '-', int v = -1) : key(k), value(v), bitmap(0){}
};

int hash_function(char key){
    if(key == '-') return -1; // Empty spot
    return ((int)key) % N;
}

void hashtable_insert(vector<Entry>& hashtable, char key, int value){
    int i = hash_function(key);
    if(hashtable[i].bitmap.all()){
        cout << "REHASH NEEDED-1" << endl;
        exit(1);
    }
    int j = i;
    while(hashtable[j].key != '-'){ // Find next free spot
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
            char element = hashtable[k].key;
            if(element == '-') continue;

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
        hashtable[j].value = hashtable[y].value;
        hashtable[y].key = '-';
        hashtable[y].value = -1;

        hashtable[hashy].bitmap.reset(H-1-(y-hashy+N)%N);
        hashtable[hashy].bitmap.set(H-1-(j-hashy+N)%N);
        j = y; // New free spot
    }

    hashtable[j].key = key;
    hashtable[j].value = value;
    hashtable[i].bitmap.set(H-1-(j+N-i)%N);        
    // cout << "Inserted key " << key << " at position " << j << endl;
}

bool hashtable_find(const vector<Entry>& hashtable, char key){
    int i = hash_function(key);
    if(i == -1) return false; // Empty spot

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
    for(auto [key, value, bitmap] : hashtable){
        cout << "{" << key << ", " << value << "} ";
    }
    cout << endl;

    cout << "Neighborhood bitsets:\n";
    for(int i=0; i<N; ++i){
        cout << "Bucket " << i << ": " << hashtable[i].bitmap << endl;
    }
}

int main(){
    vector<Entry> hashtable(N);
    vector<pair<char, int>> data = { {'a', 10}, {'b', 20}, {'l', 30}, {'d', 40}, {'e', 50}, {'k', 60} };

    for(auto [key, value] : data){
        hashtable_insert(hashtable, key, value);
    }

    hashtable_print(hashtable);

    cout << (hashtable_find(hashtable, 'k') ? "Found k" : "k not found") << endl;
    cout << (hashtable_find(hashtable, 'c') ? "Found c" : "c not found") << endl;

    return 0;
}

// Example:
// Input: a, b, l, d, e, k
// d hashes to 0, e hashes to 1, a and k hash to 7, b and l hash to 8
// Final table: b, e, d, -, -, -, -, a, k, l
// Neighborhood bitsets:
// Bucket 0: 001
// Bucket 1: 100
// Bucket 2: 000
// Bucket 3: 000
// Bucket 4: 000
// Bucket 5: 000
// Bucket 6: 000
// Bucket 7: 110
// Bucket 8: 011
// Bucket 9: 000