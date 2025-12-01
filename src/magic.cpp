// Project: Chesster - Simple Chess Engine
// File: magic.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12.01

#include "magic.h"
#include <iostream>

using namespace Bitboards;

namespace Magic {

// Attack table storage
Bitboard rook_attack_table[102400];
Bitboard bishop_attack_table[5248];

std::array<MagicEntry, 64> rook_magics;
std::array<MagicEntry, 64> bishop_magics;

// Pre-computed magic numbers (know values that I got from the internet used for magic number multiplication after masking)
// Rook magics
const Bitboard rook_magic_numbers[64] = {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010000800ULL, 0x0000808008000400ULL,
    0x0000808004000200ULL, 0x0000808002000100ULL, 0x0000808001000100ULL, 0x0000408000800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100080008080ULL, 0x0000080080008080ULL,
    0x0000040080008080ULL, 0x0000020080008080ULL, 0x0000010080008080ULL, 0x0000008080008080ULL,
    0x0000802000800100ULL, 0x0000800800400080ULL, 0x0000800400200080ULL, 0x0000800200100080ULL,
    0x0000800100080080ULL, 0x0000800040800100ULL, 0x0000800020400080ULL, 0x0000800010200040ULL,
    0x0000802000400080ULL, 0x0000800800200080ULL, 0x0000800400100080ULL, 0x0000800200080080ULL,
    0x0000800100040080ULL, 0x0000800040800100ULL, 0x0000800020400080ULL, 0x0000800010200040ULL,
    0x0000801000200080ULL, 0x0000800800100080ULL, 0x0000800400080080ULL, 0x0000800200040080ULL,
    0x0000800100020080ULL, 0x0000800040800100ULL, 0x0000800020400080ULL, 0x0000800010200040ULL,
    0x0000400080008080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000008001008080ULL
};

// Bishop magics
const Bitboard bishop_magic_numbers[64] = {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};

// Generate occupancy mask for rook (excludes edges)
Bitboard generate_rook_mask(int square) {
    Bitboard mask = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    // North (exclude top edge)
    for (int r = rank + 1; r < 7; r++) {
        mask |= bit(sq(file, r));
    }
    // South (exclude bottom edge)
    for (int r = rank - 1; r > 0; r--) {
        mask |= bit(sq(file, r));
    }
    // East (exclude right edge)
    for (int f = file + 1; f < 7; f++) {
        mask |= bit(sq(f, rank));
    }
    // West (exclude left edge)
    for (int f = file - 1; f > 0; f--) {
        mask |= bit(sq(f, rank));
    }

    return mask;
}

// Generate occupancy mask for bishop (excludes edges)
Bitboard generate_bishop_mask(int square) {
    Bitboard mask = 0ULL;
    int rank = square / 8;
    int file = square % 8;

    // NE diagonal
    for (int r = rank + 1, f = file + 1; r < 7 && f < 7; r++, f++) {
        mask |= bit(sq(f, r));
    }
    // NW diagonal
    for (int r = rank + 1, f = file - 1; r < 7 && f > 0; r++, f--) {
        mask |= bit(sq(f, r));
    }
    // SE diagonal
    for (int r = rank - 1, f = file + 1; r > 0 && f < 7; r--, f++) {
        mask |= bit(sq(f, r));
    }
    // SW diagonal
    for (int r = rank - 1, f = file - 1; r > 0 && f > 0; r--, f--) {
        mask |= bit(sq(f, r));
    }

    return mask;
}

// Generate rook attacks using bitboard shifts 
Bitboard generate_rook_attacks_slow(int square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    Bitboard piece_bb = bit(square);

    // North
    Bitboard temp = piece_bb;
    for (int i = 0; i < 7 - rank; i++) {
        temp <<= 8;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // South
    temp = piece_bb;
    for (int i = 0; i < rank; i++) {
        temp >>= 8;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // East
    temp = piece_bb;
    for (int i = 0; i < 7 - file; i++) {
        temp = (temp << 1) & ~0x0101010101010101ULL;  // Clear file A to prevent wrapping
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // West
    temp = piece_bb;
    for (int i = 0; i < file; i++) {
        temp = (temp >> 1) & ~0x8080808080808080ULL;  // Clear file H to prevent wrapping
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }

    return attacks;
}

// Generate bishop attacks using bitboard shifts 
Bitboard generate_bishop_attacks_slow(int square, Bitboard occupied) {
    Bitboard attacks = 0ULL;
    int rank = square / 8;
    int file = square % 8;
    Bitboard piece_bb = bit(square);

    // NE diagonal (+9)
    Bitboard temp = piece_bb;
    for (int i = 0; i < std::min(7 - rank, 7 - file); i++) {
        temp = (temp << 9) & ~0x0101010101010101ULL; //mask A file
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // NW diagonal (+7)
    temp = piece_bb;
    for (int i = 0; i < std::min(7 - rank, file); i++) {
        temp = (temp << 7) & ~0x8080808080808080ULL;
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // SE diagonal (-7)
    temp = piece_bb;
    for (int i = 0; i < std::min(rank, 7 - file); i++) {
        temp = (temp >> 7) & ~0x0101010101010101ULL;
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }
    // SW diagonal (-9)
    temp = piece_bb;
    for (int i = 0; i < std::min(rank, file); i++) {
        temp = (temp >> 9) & ~0x8080808080808080ULL;
        if (!temp) break;
        attacks |= temp;
        if (temp & occupied) break;
    }

    return attacks;
}

// Count bits in a bitboard
int popcount(Bitboard b) {
    return __builtin_popcountll(b);
}

// Generate all occupancy variations for a mask
void generate_occupancies(Bitboard mask, std::vector<Bitboard>& occupancies) {
    int bits[64];
    int count = 0;
    
    // Extract bit positions
    Bitboard temp = mask;
    while (temp) {
        int pos = __builtin_ctzll(temp);
        bits[count++] = pos;
        temp &= temp - 1;
    }
    
    // Generate all combinations
    int num_combinations = 1 << count;
    occupancies.resize(num_combinations);
    
    for (int i = 0; i < num_combinations; i++) {
        Bitboard occ = 0ULL;
        for (int j = 0; j < count; j++) {
            if (i & (1 << j)) {
                occ |= (1ULL << bits[j]);
            }
        }
        occupancies[i] = occ;
    }
}

// Initialize magic bitboards
void init_magics() {
    int rook_table_index = 0;
    int bishop_table_index = 0;

    // Initialize rook magics
    for (int square = 0; square < 64; square++) {
        Bitboard mask = generate_rook_mask(square);
        int bits = popcount(mask);
        int shift = 64 - bits;
        
        rook_magics[square].mask = mask;
        rook_magics[square].magic = rook_magic_numbers[square];
        rook_magics[square].shift = shift;
        rook_magics[square].attacks = &rook_attack_table[rook_table_index];
        
        // Generate all occupancy variations
        std::vector<Bitboard> occupancies;
        generate_occupancies(mask, occupancies);
        
        // Fill attack table
        for (Bitboard occ : occupancies) {
            int index = (int)((occ * rook_magics[square].magic) >> shift);
            rook_magics[square].attacks[index] = generate_rook_attacks_slow(square, occ);
        }
        
        rook_table_index += (1 << bits);
    }

    // Initialize bishop magics
    for (int square = 0; square < 64; square++) {
        Bitboard mask = generate_bishop_mask(square);
        int bits = popcount(mask);
        int shift = 64 - bits;
        
        bishop_magics[square].mask = mask;
        bishop_magics[square].magic = bishop_magic_numbers[square];
        bishop_magics[square].shift = shift;
        bishop_magics[square].attacks = &bishop_attack_table[bishop_table_index];
        
        // Generate all occupancy variations
        std::vector<Bitboard> occupancies;
        generate_occupancies(mask, occupancies);
        
        // Fill attack table
        for (Bitboard occ : occupancies) {
            int index = (int)((occ * bishop_magics[square].magic) >> shift);
            bishop_magics[square].attacks[index] = generate_bishop_attacks_slow(square, occ);
        }
        
        bishop_table_index += (1 << bits);
    }
}

// Fast lookup functions
Bitboard get_rook_attacks(int square, Bitboard occupied) {
    const MagicEntry& magic = rook_magics[square];
    occupied &= magic.mask;
    int index = (int)((occupied * magic.magic) >> magic.shift);
    return magic.attacks[index];
}

Bitboard get_bishop_attacks(int square, Bitboard occupied) {
    const MagicEntry& magic = bishop_magics[square];
    occupied &= magic.mask;
    int index = (int)((occupied * magic.magic) >> magic.shift);
    return magic.attacks[index];
}

Bitboard get_queen_attacks(int square, Bitboard occupied) {
    return get_rook_attacks(square, occupied) | get_bishop_attacks(square, occupied);
}

} // namespace Magic