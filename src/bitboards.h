
#pragma once
#include <cstdint>
#include <array>

namespace Bitboards {

// --------------- Types ----------------
using Bitboard = uint64_t;

// --------------- Square helpers ----------------
inline int sq(int file, int rank) {
    return rank * 8 + file;
}

inline Bitboard bit(int sq_index) {
    return 1ULL << sq_index;
}

// --------------- Piece bitboards ----------------
extern Bitboard white_pawns;
extern Bitboard white_knights;
extern Bitboard white_bishops;
extern Bitboard white_rooks;
extern Bitboard white_queens;
extern Bitboard white_kings;

extern Bitboard black_pawns;
extern Bitboard black_knights;
extern Bitboard black_bishops;
extern Bitboard black_rooks;
extern Bitboard black_queens;
extern Bitboard black_kings;

// --------------- Combined bitboards ----------------
extern Bitboard all_white;
extern Bitboard all_black;
extern Bitboard occupied;
extern Bitboard empty;

// --------------- Masks ----------------
extern std::array<Bitboard, 8> file_mask; // file A-H
extern std::array<Bitboard, 8> rank_mask; // rank 1-8
extern std::array<Bitboard, 64> knight_attacks; // precomputed knight attack masks
extern std::array<Bitboard, 64> king_attacks;   // precomputed king attack masks

// --------------- Functions ----------------
void init_bitboards();       // Initialize piece positions
void init_masks();           // Initialize file/rank masks
void init_attack_tables();   // Precompute knight/king attacks

} // namespace Bitboards

