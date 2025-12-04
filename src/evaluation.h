// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04


#pragma
#include "bitboards.h"


class Board;

namespace Evaluation {
    //representing values of pieces, reference 1 pawn = 100
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;
    const int KING_VALUE = 20000;


    // we got all the following tables from YouTube videos, Internet forums and AI
    // increase of value of pieces if they are on particular squares
    const int PAWN_TABLE[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
        5,   5,  10,  25,  25,  10,   5,   5,
        0,   0,   0,  20,  20,   0,   0,   0,
        5,  -5, -10,   0,   0, -10,  -5,   5,
        5,  10,  10, -20, -20,  10,  10,   5,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    const int KNIGHT_TABLE[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    const int BISHOP_TABLE[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    const int ROOK_TABLE[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        5,  10,  10,  10,  10,  10,  10,   5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        0,   0,   0,   5,   5,   0,   0,   0
    };

    const int QUEEN_TABLE[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -5,   0,   5,   5,   5,   5,   0,  -5,
         0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    const int KING_MIDDLEGAME_TABLE[64] = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
         20,  20,   0,   0,   0,   0,  20,  20,
        20,  35,  10,   0,   0,  10,  35,  20
    };

    const int KING_ENDGAME_TABLE[64] = {
        -50, -40, -30, -20, -20, -30, -40, -50,
        -30, -20, -10,   0,   0, -10, -20, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -30,   0,   0,   0,   0, -30, -30,
        -50, -30, -30, -30, -30, -30, -30, -50
    };

    //additional bonus/malus for specific cases
    const int BISHOP_PAIR_BONUS = 50;  // having both bishops
    const int ROOK_OPEN_FILE_BONUS = 20;  // rook on open file
    const int ROOK_SEMI_OPEN_FILE_BONUS = 10; // rook on semi-open file
    const int PASSED_PAWN_BONUS[8] = {0, 10, 20, 40, 60, 100, 150, 0}; // by rank
    const int ISOLATED_PAWN_PENALTY = -10; //for isolated pawn
    const int DOUBLED_PAWN_PENALTY = -15;
    const int MOBILITY_BONUS = 5;  // per legal move in middlegame
    

    // flip square index for black's perspective
    inline int flip_square(int square) {
    return square ^ 56;  // flip rank
    }



    // Get piece-square table value
    inline int get_pst_value(const int* table, int square, bool is_white) {
        if (!is_white) {
            square = flip_square(square);
        }
        return table[square];
    }

    // Detect endgame (simple heuristic: no queens or very little material)
    inline bool is_endgame(int white_material, int black_material) {
        int total_material = white_material + black_material;
        // Endgame if total material < 2 rooks + 2 bishops + 2 knights per side
        return total_material < (2 * ROOK_VALUE + 2 * BISHOP_VALUE + 2 * KNIGHT_VALUE) * 2;
    }

    // Count bits in a bitboard
    inline int popcount(Bitboards::Bitboard b) {
        return __builtin_popcountll(b);
    }



    //evaluate the current position from white's perspective
    // Positive = good for white, Negative = good for black
    int evaluate(Bitboards::Bitboard white_pawns,
                Bitboards::Bitboard white_knights,
                Bitboards::Bitboard white_bishops,
                Bitboards::Bitboard white_rooks,
                Bitboards::Bitboard white_queens,
                Bitboards::Bitboard white_kings,
                Bitboards::Bitboard black_pawns,
                Bitboards::Bitboard black_knights,
                Bitboards::Bitboard black_bishops,
                Bitboards::Bitboard black_rooks,
                Bitboards::Bitboard black_queens,
                Bitboards::Bitboard black_kings,
                Bitboards::Bitboard occupied,
                bool white_to_move);

    //evaluate from the current board state
int evaluate_board(const class Board& board);

//individual evaluation components 
int evaluate_material(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                     Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                     Bitboards::Bitboard white_queens,
                     Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                     Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                     Bitboards::Bitboard black_queens);

int evaluate_piece_square_tables(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                                 Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                                 Bitboards::Bitboard white_queens, Bitboards::Bitboard white_kings,
                                 Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                                 Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                                 Bitboards::Bitboard black_queens, Bitboards::Bitboard black_kings,
                                 bool is_endgame);

int evaluate_pawn_structure(Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                           Bitboards::Bitboard occupied);

int evaluate_piece_activity(Bitboards::Bitboard white_rooks, Bitboards::Bitboard black_rooks,
                            Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard occupied);

} // namespace Evaluation


