// Project: Chesster - Simple Chess Engine
// File: magic.h
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-01

#pragma once
#include "bitboards.h"
#include <array>

namespace Magic {

// Magic bitboard structures for sliding pieces
struct MagicEntry {
    Bitboards::Bitboard mask;      // Relevant occupancy mask
    Bitboards::Bitboard magic;     // Magic number
    Bitboards::Bitboard* attacks;  // Pointer to attack table
    int shift;                     // Shift for magic index
};

// Attack tables
extern std::array<MagicEntry, 64> rook_magics;
extern std::array<MagicEntry, 64> bishop_magics;

// Attack table storage
extern Bitboards::Bitboard rook_attack_table[102400];   // Total size needed for all rook squares
extern Bitboards::Bitboard bishop_attack_table[5248];   // Total size needed for all bishop squares

// Initialize magic bitboards
void init_magics();

// Get attacks for sliding pieces
Bitboards::Bitboard get_rook_attacks(int square, Bitboards::Bitboard occupied);
Bitboards::Bitboard get_bishop_attacks(int square, Bitboards::Bitboard occupied);
Bitboards::Bitboard get_queen_attacks(int square, Bitboards::Bitboard occupied);

// Helper functions for magic bitboard generation
Bitboards::Bitboard generate_rook_mask(int square);
Bitboards::Bitboard generate_bishop_mask(int square);
Bitboards::Bitboard generate_rook_attacks_slow(int square, Bitboards::Bitboard occupied);
Bitboards::Bitboard generate_bishop_attacks_slow(int square, Bitboards::Bitboard occupied);

} // namespace Magic