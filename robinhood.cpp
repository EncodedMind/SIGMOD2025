// Να κάνω load factor και resize
// -------------------------------
// Robin Hood Hashing
#include <iostream>
#include <vector>
#include <functional> // for std::hash
#define N 10 // Size of hash table
using namespace std;

struct Entry{
    int key;
    vector<int> values;
    int psl;

    Entry(int k = -1, const vector<int>& vec = {}, int p = -1) : key(k), values(vec), psl(p){};
};

int hash_function(int key){
    if(key == -1) return -1; // Empty spot
    return key % N;
}

// int hash_function(int key){
//     std::hash<int> hasher;
//     return hasher(key) % N;
// }

void hashtable_insert(vector<Entry>& hashtable, int key, const vector<int>& inputvalues){
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

bool hashtable_find(const vector<Entry>& hashtable, int key){
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

void hashtable_print(const vector<Entry>& hashtable){
    cout << "Final table: ";
    for(auto [key, values, psl] : hashtable){
        cout << "{" << key << ": [";
        for(auto v : values) cout << v << " ";
        cout << "], psl: " << psl << "} ";
    }
    cout << endl;
}

int main(){
    
    vector<Entry> hashtable(N);
    // vector<pair<int, int>> data = { {10,100}, {21,210}, {32,320}, {10,101}, {43,430}, {21,211} };
    // Test data that creates edge cases
    vector<pair<int, int>> data = {
        {10, 100},   // Base case, hashes to 0
        {20, 200},   // Hashes to 0, forces collision
        {30, 300},   // Hashes to 0, forces second collision
        {9, 900},    // Hashes to 9, will wrap around
        {19, 901},   // Hashes to 9, forces wrap-around collision
        {10, 101},   // Duplicate key with new value
        {20, 201},   // Another duplicate key
        {40, 400},   // Hashes to 0, creates maximum PSL
        {50, 500}    // Should trigger multiple Robin Hood steals
    };

    for(auto [key, value] : data){
        vector<int> values = {value};
        hashtable_insert(hashtable, key, values);
    }

    hashtable_print(hashtable);

    cout << (hashtable_find(hashtable, 21) ? "Found 21" : "21 not found") << endl;
    cout << (hashtable_find(hashtable, 100) ? "Found 100" : "100 not found") << endl;

    return 0;
}