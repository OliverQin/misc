#pragma once

#define RANDOM_SHUFFLER_MAX_ROUND 2

#include <cstdint>

template <uint64_t len>
class BitShuffler {
    
    //uint64_t status_;
    uint64_t mask_;
    
    uint64_t rounds_[RANDOM_SHUFFLER_MAX_ROUND];
    static const uint64_t multis_[RANDOM_SHUFFLER_MAX_ROUND]; 

public:
    BitShuffler(uint64_t seed=0) {
        static_assert( len <= 64, "BitShuffler only supports 0 < len <= 64." );
        static_assert( len  >  0, "BitShuffler only supports 0 < len <= 64." );
        
        mask_ = 0;
        for (uint64_t i = 0; i < len; ++ i)
            mask_ |= (1ull << i);
        
        for (int i = 0; i < RANDOM_SHUFFLER_MAX_ROUND; ++ i) {
            seed = (seed + 0x9e3ae8b180f0e2b7ull) * 0xad5750dce9df6249ull;
            rounds_[i] = seed;
        }
    }
    
    uint64_t getNumber(uint64_t pos) {
        for (int i = 0; i < RANDOM_SHUFFLER_MAX_ROUND; ++ i) {
            pos = ((pos * multis_[i]) + rounds_[i]) & mask_;
            pos ^= (pos >> (len/2) );
        }
        return pos;
    }
};

template <uint64_t len>
const uint64_t BitShuffler<len>::multis_[RANDOM_SHUFFLER_MAX_ROUND] = {
    0xc457cbec9f271879ull, 0xdef07fb892b9a8dbull}; //, 0x9016389eb2fbbf7full, 0xc9b841162b4a78f1ull};
