// Να κάνω load factor και resize
// -------------------------------
// Robin Hood Hashing
#include <iostream>
#include <vector>
#define N 6 // Size of hash table = size of keys
using namespace std;

struct Entry{
    char key;
    int value;
    int psl;

    Entry(char k = '-', int v = -1, int p = -1) : key(k), value(v), psl(p){}
};

int hash_function(char key){
    if(key == 'a' || key == 'e' || key == 'f') return 0;
    if(key == 'b' || key == 'c') return 1;
    if(key == 'd') return 2;
    return 0;
}

void hashtable_insert(vector<Entry>& hashtable, char key, int value){
    int pos = hash_function(key);
    int psl = 0;
    while(1){
        if(hashtable[pos].key == '-'){ // empty spot
            hashtable[pos] = Entry(key, value, psl);
            break;
        }
        else{
            if(psl > hashtable[pos].psl){
                Entry temp = hashtable[pos];
                hashtable[pos] = Entry(key, value, psl);
                key = temp.key;
                value = temp.value;
                psl = temp.psl;
            }
            psl++;
            if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
        }
    }
}

bool hashtable_find(const vector<Entry>& hashtable, char key){
    int pos = hash_function(key);
    int psl = 0;

    while(1){
        if(hashtable[pos].key == key) return true; // found
        if(hashtable[pos].key == '-') return false; // empty spot
        if(psl > hashtable[pos].psl) return false; // if the psl of the key we are looking for is greater than the psl of the current element, then it means that the key is not in the table
        psl++;
        if(++pos == N) pos = 0; // in case the table ends, we have to go check at the beginning
    }
}

void hashtable_print(const vector<Entry>& hashtable){
    for(auto [key, value, psl] : hashtable){
        cout << key << " {PSL: " << psl << ", Value: " << value << "}" << endl;
    }
}

int main(){
    
    vector<Entry> hashtable(N);
    vector<pair<char, int>> data = { {'a', 10}, {'b', 20}, {'c', 30}, {'d', 40}, {'e', 50}, {'f', 60} };

    for(auto [key, value] : data){
        hashtable_insert(hashtable, key, value);
    }

    hashtable_print(hashtable);

    cout << (hashtable_find(hashtable, 'b') ? "Found b" : "b not found") << endl;
    cout << (hashtable_find(hashtable, 'g') ? "Found g" : "g not found") << endl;

    return 0;
}

// Example:
// a, e, f hash to 0
// b, c to 1
// d to 2
// Then, inserting values in alphabetical order, a, b, c, d, e, f, produces the hash table:
// a {PSL: 0}
// e {PSL: 1}
// f {PSL: 2}
// b {PSL: 2}
// c {PSL: 3}
// d {PSL: 3}