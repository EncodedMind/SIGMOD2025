// Να κάνω resize, load factor και cycle detection
//
// Αν load factor > 0.5 ή αν κύκλος, resize: Διπλασιάζω μέγεθος και επανεισάγω
// Προσοχή: Μπορεί να προκύψει resize μέσα σε resize
// Cycle detection:
// i) Αν συγκρίνουμε στοιχείο που έχουμε ήδη μετακινήσει
// ii) ctr στοιχεία που μετακινήθηκαν και αν ctr=πλήθος στοιχείων στους πίνακες, κύκλος
// --------------------------
// Cuckoo Hashing
#include <iostream>
#include <vector>
// #include <functional> // for std::hash
#define N 10 // Size of hash tables
using namespace std;

struct Entry{
    int key;
    vector<int> values;

    Entry(int k = -1, const vector<int>& vec = {}) : key(k), values(vec){}
};

int hash_function1(int key){
    return key % N;
}

// int hash_function2(int key){
//     std::hash<int> hasher;
//     return hasher(key) % N;
// }

int hash_function2(int key){
    return (key / N) % N;
}

void hashtable_insert(vector<Entry>& hashtable1, vector<Entry>& hashtable2, int key, const vector<int>& inputvalues){
    bool table1 = true; // true for T1, false for T2
    vector<int> values = inputvalues;
    Entry current(key, values);

    while(1){
        if(table1){
            int hash1 = hash_function1(current.key);
            if(hashtable1[hash1].key == -1){ // position in T1 is available
                hashtable1[hash1] = current;
                break;
            }
            else{
                if(hashtable1[hash1].key == current.key){ // key already exists, just add the value
                    for(auto& v : current.values) hashtable1[hash1].values.push_back(v);
                    break;
                }
                swap(hashtable1[hash1], current); // place new key in T1
                table1 = false;
            }
        }
        else{
            int hash2 = hash_function2(current.key);
            if(hashtable2[hash2].key == -1){ // position in T2 is available
                hashtable2[hash2] = current;
                break;
            }
            else{
                if(hashtable2[hash2].key == current.key){ // key already exists, just add the value
                    for(auto& v : current.values) hashtable2[hash2].values.push_back(v);
                    break;
                }
                swap(hashtable2[hash2], current); // place new key in T2
                table1 = true;
            }
        }
    }
}

bool hashtable_find(const vector<Entry>& hashtable1, const vector<Entry>& hashtable2, int key){
    return (hashtable1[hash_function1(key)].key == key || hashtable2[hash_function2(key)].key == key);
}

void hashtable_print(const vector<Entry>& hashtable){
    for(auto [key, values] : hashtable){
        cout << "{" << key << ": [";
        for(auto v : values) cout << v << " ";
        cout << "]} ";
    }
    cout << endl;
}

int main(){

    vector<Entry> T1(N);
    vector<Entry> T2(N);
    vector<pair<int, int>> data = {
        {10, 100},    // hash1=0, hash2=1
        {20, 200},    // hash1=0, hash2=2 - forces first kickout from T1
        {30, 300},    // hash1=0, hash2=3 - forces multiple kickouts
        {110, 110},   // hash1=0, hash2=11%10=1 - creates chain
        {10, 101},    // Tests duplicate key handling
        {55, 550},    // hash1=5, hash2=5 - same position in both tables
        {95, 950},    // hash1=5, hash2=9 - forces long chain of moves
        {75, 750}     // hash1=5, hash2=7 - tests multiple displacements
    };

    for(auto [key, value] : data){
        hashtable_insert(T1, T2, key, {value});
    }

    cout << "Table 1: ";
    hashtable_print(T1);
    
    cout << "Table 2: ";
    hashtable_print(T2);

    cout << (hashtable_find(T1, T2, 30) ? "Found 30" : "30 not found") << endl;
    cout << (hashtable_find(T1, T2, 100) ? "Found 100" : "100 not found") << endl;

    return 0;
}