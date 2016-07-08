#pragma once

#define RANDOM_SHUFFLER_MAX_ROUND 4

#include <cstdint>

template <uint32_t len>
class BitShuffler {
    
    //uint32_t status_;
    uint32_t mask_;
    uint32_t rounds_[RANDOM_SHUFFLER_MAX_ROUND];
    
    static const uint32_t multis_[RANDOM_SHUFFLER_MAX_ROUND]; 

public:
    BitShuffler(uint32_t seed=0) {
        static_assert( len <= 32, "BitShuffler only supports 0 < len <= 32." );
        static_assert( len  >  0, "BitShuffler only supports 0 < len <= 32." );
        
        mask_ = 0;
        for (int i = 0; i < len; ++ i)
            mask_ |= (1 << i);
        
        for (int i = 0; i < RANDOM_SHUFFLER_MAX_ROUND; ++ i) {
            seed = (seed + 0x233dead3) * 0x3f998253;
            rounds_[i] = seed;
        }
    }
    
    uint32_t getNumber(uint32_t pos) {
        for (int i = 0; i < RANDOM_SHUFFLER_MAX_ROUND; ++ i) {
            pos = (pos * multis_[i] + rounds_[i]) & mask_;
            pos ^= (pos >> (len/2) );
        }
        return pos;
    }
};

template <uint32_t len>
const uint32_t BitShuffler<len>::multis_[RANDOM_SHUFFLER_MAX_ROUND] = {
    0x963df78b, 0xf45e08b9, 0xa898b30b, 0xc7d071df};
