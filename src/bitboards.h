// Project: Chesster - Simple Chess Engine
// File: bitboards.h
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#pragma once
#include <cstdint>
#include <array>

namespace Bitboards
{

    // --------------- Types ----------------
    using Bitboard = uint64_t;

    // --------------- Square helpers ----------------
    inline int sq(int file, int rank)
    {
        return rank * 8 + file;
    }

    inline Bitboard bit(int sq_index)
    {
        if (sq_index < 0 || sq_index >= 64)
        {
            // Out-of-bounds square; return empty bitboard to avoid issues
            return 0ULL;
        }
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
    extern std::array<Bitboard, 8> file_mask;                              // file A-H
    extern std::array<Bitboard, 8> rank_mask;                              // rank 1-8
    extern std::array<Bitboard, 64> knight_attacks;                        // precomputed knight attack masks
    extern std::array<Bitboard, 64> king_attacks;                          // precomputed king attack masks
    extern std::array<Bitboard, 64> pawn_attacks_white;                    // precomputed white pawn attack masks
    extern std::array<Bitboard, 64> pawn_attacks_black;                    // precomputed black pawn attack masks
    extern std::array<std::array<Bitboard, 64>, 64> pawn_ep_attacks_white; // array because en passant depends on both pawn position and en passant square
    extern std::array<std::array<Bitboard, 64>, 64> pawn_ep_attacks_black;
    // Masks used in evaluation later on:
    extern std::array<Bitboard, 64> passed_pawn_white;
    extern std::array<Bitboard, 64> passed_pawn_black;
    extern std::array<Bitboard, 64> isolated_pawn_mask;
    // --------------- Functions ----------------
    void init_bitboards();     // Initialize piece positions
    void init_masks();         // Initialize file/rank masks
    void init_attack_tables(); // Precompute knight/king/pawn attacks

    // restore helpers for global bitboards 
    struct BoardState {
        Bitboard white_pawns;
        Bitboard white_knights;
        Bitboard white_bishops;
        Bitboard white_rooks;
        Bitboard white_queens;
        Bitboard white_kings;

        Bitboard black_pawns;
        Bitboard black_knights;
        Bitboard black_bishops;
        Bitboard black_rooks;
        Bitboard black_queens;
        Bitboard black_kings;

        Bitboard all_white;
        Bitboard all_black;
        Bitboard occupied;
        Bitboard empty;
        
        // Board member variables for full state synchronization
        bool team_white;
        int ep_square;
        bool white_castle_kingside;
        bool white_castle_queenside;
        bool black_castle_kingside;
        bool black_castle_queenside;
        int halfmove_clock;
    };

    BoardState snapshot_state();
    void restore_state(const BoardState &s);

} // namespace Bitboards