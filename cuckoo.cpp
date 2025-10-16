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
#define N 11 // Size of hash tables = size of keys + 1
using namespace std;

struct Entry{
    int key;
    int value;

    Entry(int k = -1, int v = -1) : key(k), value(v){}
};

int hash_function1(int key){
    return key % N;
}

int hash_function2(int key){
    return (key / N) % N;
}

void hashtable_insert(vector<Entry>& hashtable1, vector<Entry>& hashtable2, int key, int value){
    Entry current(key, value);
    bool table1 = true; // true for T1, false for T2

    while(1){
        if(table1){
            int hash1 = hash_function1(current.key);
            if(hashtable1[hash1].key == -1){ // position in T1 is available
                hashtable1[hash1] = current;
                break;
            }
            else{
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
    for(auto [key, value] : hashtable){
        if(key == -1) cout << "- ";
        else cout << "{" << key << ", " << value << "} ";
    }
    cout << endl;
}

int main(){

    vector<Entry> T1(N);
    vector<Entry> T2(N);
    vector<pair<int, int>> data = { {20, 200}, {50, 500}, {53, 530}, {75, 750}, {100, 1000}, {67, 670}, {105, 1050}, {3, 300}, {36, 360}, {39, 390} };

    for(auto [key, value] : data){
        hashtable_insert(T1, T2, key, value);
    }

    cout << "Table 1: ";
    hashtable_print(T1);
    
    cout << "Table 2: ";
    hashtable_print(T2);

    cout << (hashtable_find(T1, T2, 100) ? "Found 100" : "100 not found") << endl;
    cout << (hashtable_find(T1, T2, 25) ? "Found 25" : "25 not found") << endl;

    return 0;
}


// Example:
// Input keys: {20, 50, 53, 75, 100, 67, 105, 3, 36, 39}
// T1: - 100 - 36 - - 50 - - 75 -
// T2: 3 20 - 39 53 - 67 - - 105 -
// Hash functions:
// h1(x) = x % 11
// h2(x) = (x / 11) % 11