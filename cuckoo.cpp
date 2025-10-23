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
#include <bit>
using namespace std;

struct Cuckoo{
    int N;

    struct Entry{
        int key;
        vector<int> values;

        Entry(int k = -1, const vector<int>& vec = {}) : key(k), values(vec){}
    };

    vector<Entry> hashtable1, hashtable2;

    Cuckoo(int size){ // constructor
        N = bit_ceil(static_cast<unsigned>(size));
        hashtable1.resize(N);
        hashtable2.resize(N);
    }

    int hash_function1(int key) const{
        return key & (N-1);
    }

    int hash_function2(int key) const{
        return (key / N) & (N-1);
    }

    void insert(int key, const vector<int>& inputvalues){
        bool table1 = true; // true for hashtable1, false for hashtable2
        vector<int> values = inputvalues;
        Entry current(key, values);

        while(1){
            if(table1){
                int hash1 = hash_function1(current.key);
                if(hashtable1[hash1].key == -1){ // position in hashtable1 is available
                    hashtable1[hash1] = current;
                    break;
                }
                else{
                    if(hashtable1[hash1].key == current.key){ // key already exists, just add the value
                        for(auto& v : current.values) hashtable1[hash1].values.push_back(v);
                        break;
                    }
                    swap(hashtable1[hash1], current); // place new key in hashtable1
                    table1 = false;
                }
            }
            else{
                int hash2 = hash_function2(current.key);
                if(hashtable2[hash2].key == -1){ // position in hashtable2 is available
                    hashtable2[hash2] = current;
                    break;
                }
                else{
                    if(hashtable2[hash2].key == current.key){ // key already exists, just add the value
                        for(auto& v : current.values) hashtable2[hash2].values.push_back(v);
                        break;
                    }
                    swap(hashtable2[hash2], current); // place new key in hashtable2
                    table1 = true;
                }
            }
        }
    }

    bool find(int key) const{
        return (hashtable1[hash_function1(key)].key == key || hashtable2[hash_function2(key)].key == key);
    }

    void print() const{
        auto print_table = [](const vector<Entry>& hashtable){
            for(const auto& [key, values] : hashtable){
                if(key == -1) cout << "{Empty} ";
                else{
                    cout << "{" << key << ": [";
                    for(auto v : values) cout << v << " ";
                    cout << "]} ";
                }
            }
            cout << '\n';
        };

        cout << "Table 1: ";
        print_table(hashtable1);
        cout << "Table 2: ";
        print_table(hashtable2);
    }
};

int main(){

    Cuckoo hashtable(10);
    vector<pair<int, int>> data = {
        {10, 100},    // hash1=0, hash2=1
        {20, 200},    // hash1=0, hash2=2 - forces first kickout from hashtable1
        {30, 300},    // hash1=0, hash2=3 - forces multiple kickouts
        {110, 110},   // hash1=0, hash2=11%10=1 - creates chain
        {10, 101},    // Tests duplicate key handling
        {55, 550},    // hash1=5, hash2=5 - same position in both tables
        {95, 950},    // hash1=5, hash2=9 - forces long chain of moves
        {75, 750}     // hash1=5, hash2=7 - tests multiple displacements
    };

    for(auto [key, value] : data){
        hashtable.insert(key, {value});
    }

    hashtable.print();

    cout << (hashtable.find(30) ? "Found 30\n" : "30 not found\n");
    cout << (hashtable.find(100) ? "Found 100\n" : "100 not found\n");

    return 0;
}

// g++ -std=c++20 cuckoo.cpp -o cuckoo && ./cuckoo