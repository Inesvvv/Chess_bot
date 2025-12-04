// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04

#include "evaluation.h"
#include "board.h"
#include "magic.h"

namespace Evaluation {


int evaluate_material(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                     Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                     Bitboards::Bitboard white_queens,
                     Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                     Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                     Bitboards::Bitboard black_queens)
{
    int score = 0;
    
    // White material
    score += popcount(white_pawns) * PAWN_VALUE;
    score += popcount(white_knights) * KNIGHT_VALUE;
    score += popcount(white_bishops) * BISHOP_VALUE;
    score += popcount(white_rooks) * ROOK_VALUE;
    score += popcount(white_queens) * QUEEN_VALUE;
    
    // Black material
    score -= popcount(black_pawns) * PAWN_VALUE;
    score -= popcount(black_knights) * KNIGHT_VALUE;
    score -= popcount(black_bishops) * BISHOP_VALUE;
    score -= popcount(black_rooks) * ROOK_VALUE;
    score -= popcount(black_queens) * QUEEN_VALUE;
    
    // Bishop pair bonus
    if (popcount(white_bishops) >= 2) {
        score += BISHOP_PAIR_BONUS;
    }
    if (popcount(black_bishops) >= 2) {
        score -= BISHOP_PAIR_BONUS;
    }
    
    return score;
}


int evaluate_piece_square_tables(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                                 Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                                 Bitboards::Bitboard white_queens, Bitboards::Bitboard white_kings,
                                 Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                                 Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                                 Bitboards::Bitboard black_queens, Bitboards::Bitboard black_kings,
                                 bool is_endgame)
{
    int score = 0;
    
    // Helper to evaluate a piece type
    auto eval_pieces = [&](Bitboards::Bitboard pieces, const int* table, bool is_white, int multiplier) {
        Bitboards::Bitboard temp = pieces;
        while (temp) {
            int sq = __builtin_ctzll(temp);
            temp &= temp - 1;
            score += multiplier * get_pst_value(table, sq, is_white);
        }
    };
    
    // White pieces
    eval_pieces(white_pawns, PAWN_TABLE, true, 1);
    eval_pieces(white_knights, KNIGHT_TABLE, true, 1);
    eval_pieces(white_bishops, BISHOP_TABLE, true, 1);
    eval_pieces(white_rooks, ROOK_TABLE, true, 1);
    eval_pieces(white_queens, QUEEN_TABLE, true, 1);
    
    // King - choose table based on game phase
    const int* king_table = is_endgame ? KING_ENDGAME_TABLE : KING_MIDDLEGAME_TABLE;
    eval_pieces(white_kings, king_table, true, 1);
    
    // Black pieces (negative contribution)
    eval_pieces(black_pawns, PAWN_TABLE, false, -1);
    eval_pieces(black_knights, KNIGHT_TABLE, false, -1);
    eval_pieces(black_bishops, BISHOP_TABLE, false, -1);
    eval_pieces(black_rooks, ROOK_TABLE, false, -1);
    eval_pieces(black_queens, QUEEN_TABLE, false, -1);
    eval_pieces(black_kings, king_table, false, -1);
    
    return score;
}


int evaluate_pawn_structure(Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                           Bitboards::Bitboard occupied)
{
    int score = 0;
    
    // Evaluate white pawns
    Bitboards::Bitboard temp_white = white_pawns;
    while (temp_white) {
        int sq = __builtin_ctzll(temp_white);
        temp_white &= temp_white - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        // Passed pawn check
        if ((Bitboards::passed_pawn_white[sq] & black_pawns) == 0) {
            score += PASSED_PAWN_BONUS[rank];
        }
        
        // Isolated pawn check
        if ((Bitboards::isolated_pawn_mask[sq] & white_pawns) == 0) {
            score += ISOLATED_PAWN_PENALTY;
        }
        
        // Doubled pawns check
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        if (popcount(white_pawns & file_mask) > 1) {
            score += DOUBLED_PAWN_PENALTY;
        }

        // maybe we will add other known pawn structures later such as pawn chains
    }
    
    // Evaluate black pawns
    Bitboards::Bitboard temp_black = black_pawns;
    while (temp_black) {
        int sq = __builtin_ctzll(temp_black);
        temp_black &= temp_black - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        // Passed pawn check
        if ((Bitboards::passed_pawn_black[sq] & white_pawns) == 0) {
            score -= PASSED_PAWN_BONUS[7 - rank];  // Flip rank for black
        }
        
        // Isolated pawn check
        if ((Bitboards::isolated_pawn_mask[sq] & black_pawns) == 0) {
            score -= ISOLATED_PAWN_PENALTY;
        }
        
        // Doubled pawns check
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        if (popcount(black_pawns & file_mask) > 1) {
            score -= DOUBLED_PAWN_PENALTY;
        }
    }
    
    return score;
}


int evaluate_piece_activity(Bitboards::Bitboard white_rooks, Bitboards::Bitboard black_rooks,
                            Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard occupied)
{
    int score = 0;
    
    // Evaluate white rooks
    Bitboards::Bitboard temp_white_rooks = white_rooks;
    while (temp_white_rooks) {
        int sq = __builtin_ctzll(temp_white_rooks);
        temp_white_rooks &= temp_white_rooks - 1;
        
        int file = sq % 8;
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        
        // Open file (no pawns)
        if ((file_mask & (white_pawns | black_pawns)) == 0) {
            score += ROOK_OPEN_FILE_BONUS;
        }
        // Semi-open file (no friendly pawns)
        else if ((file_mask & white_pawns) == 0) {
            score += ROOK_SEMI_OPEN_FILE_BONUS;
        }
    }
    
    // Evaluate black rooks
    Bitboards::Bitboard temp_black_rooks = black_rooks;
    while (temp_black_rooks) {
        int sq = __builtin_ctzll(temp_black_rooks);
        temp_black_rooks &= temp_black_rooks - 1;
        
        int file = sq % 8;
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        
        // Open file
        if ((file_mask & (white_pawns | black_pawns)) == 0) {
            score -= ROOK_OPEN_FILE_BONUS;
        }
        // Semi-open file
        else if ((file_mask & black_pawns) == 0) {
            score -= ROOK_SEMI_OPEN_FILE_BONUS;
        }
    }
    
    return score;
}

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
             bool white_to_move)
{
    int score = 0;
    
    // 1. Material evaluation
    score += evaluate_material(white_pawns, white_knights, white_bishops, white_rooks, white_queens,
                              black_pawns, black_knights, black_bishops, black_rooks, black_queens);
    
    // Calculate material for endgame detection
    int white_material = popcount(white_knights) * KNIGHT_VALUE +
                        popcount(white_bishops) * BISHOP_VALUE +
                        popcount(white_rooks) * ROOK_VALUE +
                        popcount(white_queens) * QUEEN_VALUE;
    
    int black_material = popcount(black_knights) * KNIGHT_VALUE +
                        popcount(black_bishops) * BISHOP_VALUE +
                        popcount(black_rooks) * ROOK_VALUE +
                        popcount(black_queens) * QUEEN_VALUE;
    
    bool endgame = is_endgame(white_material, black_material);
    
    // 2. Piece-square tables
    score += evaluate_piece_square_tables(white_pawns, white_knights, white_bishops, white_rooks,
                                         white_queens, white_kings,
                                         black_pawns, black_knights, black_bishops, black_rooks,
                                         black_queens, black_kings, endgame);
    
    // 3. Pawn structure
    score += evaluate_pawn_structure(white_pawns, black_pawns, occupied);
    
    // 4. Piece activity
    score += evaluate_piece_activity(white_rooks, black_rooks, white_pawns, black_pawns, occupied);
    
    // 5. Small bonus for side to move (tempo)
    if (white_to_move) {
        score += 10;
    } else {
        score -= 10;
    }
    
    return score;
}


int evaluate_board(const Board& board)
{
    return evaluate(Bitboards::white_pawns,
                   Bitboards::white_knights,
                   Bitboards::white_bishops,
                   Bitboards::white_rooks,
                   Bitboards::white_queens,
                   Bitboards::white_kings,
                   Bitboards::black_pawns,
                   Bitboards::black_knights,
                   Bitboards::black_bishops,
                   Bitboards::black_rooks,
                   Bitboards::black_queens,
                   Bitboards::black_kings,
                   Bitboards::occupied,
                   board.team_white);
}

} // namespace Evaluation