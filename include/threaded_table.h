#pragma once
// Unchained hash table implementation based on:
// https://db.in.tum.de/~birler/papers/hashtable.pdf

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

extern const uint16_t tags[1 << 11];

namespace threaded{

// Entry: key + row index + hash
struct HashEntry {
    int32_t key;
    size_t row_idx;
    uint64_t hash;

    static uint64_t compute_hash(int32_t key) {
        uint32_t crc = 0;
        #if defined(__x86_64__) || defined(__i386__)
            crc = __builtin_ia32_crc32si(static_cast<uint32_t>(key), 0);
        #elif defined(__aarch64__)
            crc = __builtin_arm_crc32w(static_cast<uint32_t>(key), 0);
        #else
            crc = static_cast<uint32_t>(key);
        #endif
        return static_cast<uint64_t>(crc) * ((0x8648DBDull << 32) + 1);
    } 
    
    HashEntry() : key(0), row_idx(0), hash(compute_hash(0)) {}
    HashEntry(int32_t k, size_t r) : key(k), row_idx(r), hash(compute_hash(k)) {}
};

struct Block{
    Block* next;
};

struct GlobalAllocator{
    static constexpr size_t LARGE_CHUNK_SIZE = (2u << 20); // 2 MB

    void* allocateLargeChunk(){
        void* chunk = malloc(LARGE_CHUNK_SIZE);
        return chunk;
    }
};

struct BumpAllocL2{
    static constexpr size_t LARGE_CHUNK_SIZE = (2u << 20); // 2 MB
    static constexpr size_t SMALL_CHUNK_SIZE = (64u << 10); // 64 KB

    uint8_t* large_chunk = nullptr;  // where we are in the large chunk
    uint8_t* large_chunk_end = nullptr; // the end of the large chunk

    Block* head = nullptr; // the start of the list of large chunks

    ~BumpAllocL2(){ // to free all allocated chunks
        Block* current = head;
        while(current){
            Block* next = current->next;
            free(current);
            current = next;
        }
    }

    void addSpace(void* chunk){
        Block* new_block = static_cast<Block*>(chunk); // the start of the chunk will be attributed for the list node
        new_block->next = head;
        head = new_block;

        // add chunk to internal storage
        // we must skip the first sizeof(Block) bytes
        large_chunk = static_cast<uint8_t*>(chunk) + sizeof(Block);
        large_chunk_end = large_chunk + LARGE_CHUNK_SIZE - sizeof(Block);
    }

    void* allocateSmallChunk(){
        // take memory from the large chunk and return pointer
        void* chunk = large_chunk; // allocate 64KB from the pointer large_chunk
        large_chunk = large_chunk + SMALL_CHUNK_SIZE; // move pointer large_chunk 64KB ahead
        return chunk;
    }

    size_t freeSpace(){
        if(!large_chunk) return 0;
        return static_cast<size_t>(large_chunk_end - large_chunk);
    }
};

struct BumpAllocL3{
    static constexpr size_t SMALL_CHUNK_SIZE = (64u << 10); // 64 KB

    uint8_t* small_chunk = nullptr;  // where we are in the small chunk
    uint8_t* small_chunk_end = nullptr; // the end of the small chunk

    Block* head = nullptr; // the start of the list of small chunks

    // no destructor needed, all memory will be freed by the higher level allocator

    void addSpace(void* chunk){
        Block* new_block = static_cast<Block*>(chunk); // the start of the chunk will be attributed for the list node
        new_block->next = head;
        head = new_block;

        // add chunk to internal storage
        small_chunk = static_cast<uint8_t*>(chunk) + sizeof(Block);
        small_chunk_end = small_chunk + SMALL_CHUNK_SIZE - sizeof(Block);
    }
    
    HashEntry* allocate(){
        // take memory from the small chunk and return pointer
        HashEntry* entry = reinterpret_cast<HashEntry*>(small_chunk); // allocate sizeof(HashEntry) from the pointer small_chunk
        small_chunk = small_chunk + sizeof(HashEntry); // move pointer small_chunk sizeof(HashEntry) ahead
        return entry;
    }

    size_t freeSpace(){
        if(!small_chunk) return 0;
        return static_cast<size_t>(small_chunk_end - small_chunk);
    }
};

inline size_t log2_pow2(size_t n){
    size_t bits = 0;
    while ((static_cast<size_t>(1) << bits) < n) ++bits;
    return bits;
}

struct UnchainedHashTable {
    size_t numPartitions;
    uint64_t shift;
    GlobalAllocator& level1;
    BumpAllocL2 level2;
    std::vector<BumpAllocL3> level3;
    std::vector<size_t> counts;
    
    UnchainedHashTable(GlobalAllocator& globalAlloc, size_t partitions) : level1(globalAlloc), numPartitions(partitions) {
        level3.resize(numPartitions);
        counts.resize(numPartitions, 0);
        shift = static_cast<uint64_t>(log2_pow2(numPartitions));
    }
    
    UnchainedHashTable(const UnchainedHashTable&) = delete;
    UnchainedHashTable& operator=(const UnchainedHashTable&) = delete;

    void consume(HashEntry tuple){
        uint64_t part = (shift == 0) ? 0ull : (tuple.hash >> (uint64_t)(64u - shift));
        if(level3[part].freeSpace() < sizeof(HashEntry)){
            if(level2.freeSpace() < BumpAllocL2::SMALL_CHUNK_SIZE){
                void* LargeChunk = level1.allocateLargeChunk();
                level2.addSpace(LargeChunk);
            }
            void* SmallChunk = level2.allocateSmallChunk();
            level3[part].addSpace(SmallChunk);
        }
        *level3[part].allocate() = tuple;
        counts[part]++;
    }
    
};

// thread merging stage
// merge all lists of chunks of partition into one list

inline std::vector<Block*> merge_partitions(const std::vector<UnchainedHashTable>& threadTables, size_t numPartitions){

    std::vector<Block*> partition_heads(numPartitions, nullptr);

    for(size_t p = 0; p < numPartitions; ++p){
        Block* link_head = nullptr;
        Block* tail = nullptr;

        for(const auto& threadTable : threadTables){
            // iterate link_head to reach the end
            Block* current = threadTable.level3[p].head;
            if(!current) continue;
            if(!link_head){
                link_head = current;
                tail = link_head;
            }
            else{
                tail->next = current;
            }
            // move tail to the end of the current list
            while(tail->next){
                tail = tail->next;
            }
        }
        partition_heads[p] = link_head;
    }

    return partition_heads;
}

} // namespace threaded