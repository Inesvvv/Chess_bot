// Project: Chesster - Simple Chess Engine
// File: magic.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12.01

// Currently not using mgic because was making illegal/ or removing legal move but NEED to update later to upgrade speed 

#include "magic.h"
#include <iostream>
#include <vector>

using namespace Bitboards;

namespace Magic {

// Attack table storage
Bitboard rook_attack_table[102400];
Bitboard bishop_attack_table[5248];

std::array<MagicEntry, 64> rook_magics;
std::array<MagicEntry, 64> bishop_magics;

// THE BUG WAS HERE - on avait pas mis les bons numeros pour les magic bitboards donc on avait des illegal moves et des problemes de superposition 
//  CORRECTED : magic numbers found on chess programming wiki website
// Rook magics
const Bitboard rook_magic_numbers[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL,  0x2801880a0017001ULL,  0x100081001000420ULL,
    0x200020010080420ULL,  0x3001c0002010008ULL,  0x8480008002000100ULL, 0x2080088004402900ULL,
    0x800098204000ULL,     0x2024401000200040ULL, 0x100802000801000ULL,  0x120800800801000ULL,
    0x208808088000400ULL,  0x2802200800400ULL,    0x2200800100020080ULL, 0x801000060821100ULL,
    0x80044006422000ULL,   0x100808020004000ULL,  0x12108a0010204200ULL, 0x140848010000802ULL,
    0x481828014002800ULL,  0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL,  0x2040002120081000ULL, 0x21200680100081ULL,   0x20100080080080ULL,
    0x2000a00200410ULL,    0x20080800400ULL,      0x80088400100102ULL,   0x80004600042881ULL,
    0x4040008040800020ULL, 0x440003000200801ULL,  0x4200011004500ULL,    0x188020010100100ULL,
    0x14800401802800ULL,   0x2080040080800200ULL, 0x124080204001001ULL,  0x200046502000484ULL,
    0x480400080088020ULL,  0x1000422010034000ULL, 0x30200100110040ULL,   0x100021010009ULL,
    0x2002080100110004ULL, 0x202008004008002ULL,  0x20020004010100ULL,   0x2048440040820001ULL,
    0x101002200408200ULL,  0x40802000401080ULL,   0x4008142004410100ULL, 0x2060820c0120200ULL,
    0x1001004080100ULL,    0x20c020080040080ULL,  0x2935610830022400ULL, 0x44440041009200ULL,
    0x280001040802101ULL,  0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL,    0x12001008414402ULL,   0x2006104900a0804ULL,  0x1004081002402ULL
};

// Bishop magics
const Bitboard bishop_magic_numbers[64] = {
    0x40040844404084ULL,   0x2004208a004208ULL,   0x10190041080202ULL,   0x108060845042010ULL,
    0x581104180800210ULL,  0x2112080446200010ULL, 0x1080820820060210ULL, 0x3c0808410220200ULL,
    0x4050404440404ULL,    0x21001420088ULL,      0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL,      0x4011002100800ULL,    0x401484104104005ULL,  0x801010402020200ULL,
    0x400210c3880100ULL,   0x404022024108200ULL,  0x810018200204102ULL,  0x4002801a02003ULL,
    0x85040820080400ULL,   0x810102c808880400ULL, 0xe900410884800ULL,    0x8002020480840102ULL,
    0x220200865090201ULL,  0x2010100a02021202ULL, 0x152048408022401ULL,  0x20080002081110ULL,
    0x4001001021004000ULL, 0x800040400a011002ULL, 0xe4004081011002ULL,   0x1c004001012080ULL,
    0x8004200962a00220ULL, 0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL, 0x42008c0340209202ULL,
    0x209188240001000ULL,  0x400408a884001800ULL, 0x110400a6080400ULL,   0x1840060a44020800ULL,
    0x90080104000041ULL,   0x201011000808101ULL,  0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL,  0x180806108200800ULL,  0x4000020e01040044ULL, 0x300000261044000aULL,
    0x802241102020002ULL,  0x20906061210001ULL,   0x5a84841004010310ULL, 0x4010801011c04ULL,
    0xa010109502200ULL,    0x4a02012000ULL,       0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL,   0x6000020202d0240ULL,  0x8918844842082200ULL, 0x4010011029020020ULL
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

// Re-implemented magic bitboard lookup with correct magic numbers so removed slow implementation (just below if needed)
// Fast magic bitboard lookup 
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

// SLOW attack generation kept for debugging
// Bitboard get_rook_attacks_slow(int square, Bitboard occupied) {
//     return generate_rook_attacks_slow(square, occupied);
// }
// Bitboard get_bishop_attacks_slow(int square, Bitboard occupied) {
//     return generate_bishop_attacks_slow(square, occupied);
// }

Bitboard get_queen_attacks(int square, Bitboard occupied) {
    return get_rook_attacks(square, occupied) | get_bishop_attacks(square, occupied);
}

} // namespace Magic