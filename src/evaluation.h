// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04


#pragma once
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
    const int KING_SHIELD_NEAR_BONUS = 12;   // pawn directly in front of king
    const int KING_SHIELD_FAR_BONUS = 6;     // pawn two ranks ahead of king
    const int KING_SHIELD_GAP_PENALTY = 8;   // missing pawn in the shield
    const int KING_SEMI_OPEN_FILE_PENALTY = 12; // king near semi-open file
    const int KING_OPEN_FILE_PENALTY = 6;    // additional penalty if fully open
    const int KING_ZONE_ATTACK_PENALTY = 10; // per enemy attack in king zone
    const int KING_SAFETY_ENDGAME_SCALE = 60; // percent of king safety kept in endgame
    const int ROOK_ON_SEVENTH_BONUS = 30; //active rook
    const int CONNECTED_ROOKS_BONUS = 25; //rooks connected 
    const int KNIGHT_OUTPOST_BONUS = 30; //knight defended by own pawn not aatacked by enemy pawn + enemy ranks
    const int KNIGHT_OUTPOST_STRONG_BONUS = 50;
    const int BAD_BISHOP_PENALTY = -25; //bishop blocked by pieces
    const int BACKWARD_PAWN_PENALTY = -12; //pawn cant push safely + isolated
    const int PAWN_CHAIN_BONUS = 8; //connected pawns
    const int CENTER_CONTROL_BONUS = 15; //center is advantage
    const int EXTENDED_CENTER_BONUS = 5;
    const int SPACE_BONUS_PER_SQUARE = 2; 
    //maybe add penalty for hanging pieces + lower some bonus/malus might have to much impact
    

    //penalties for trapped pieces (3 levels of restriction of moves) look at values again might be a bit to harsh
    //knights 
    const int TRAPPED_KNIGHT_SEVERE = -80;      
    const int TRAPPED_KNIGHT_MODERATE = -40;   
    const int TRAPPED_KNIGHT_LIGHT = -15;       
    
    //bishops 
    const int TRAPPED_BISHOP_SEVERE = -75;     
    const int TRAPPED_BISHOP_MODERATE = -35;    
    const int TRAPPED_BISHOP_LIGHT = -12;       

    //rooks 
    const int TRAPPED_ROOK_SEVERE = -100;      
    const int TRAPPED_ROOK_MODERATE = -50;      
    const int TRAPPED_ROOK_LIGHT = -20;        
    
    //queens 
    const int TRAPPED_QUEEN_SEVERE = -150;      
    const int TRAPPED_QUEEN_MODERATE = -70;     
    const int TRAPPED_QUEEN_LIGHT = -25;       
    
    // Additional situational penalties (smaller, stackable)
    const int CORNER_TRAP_PENALTY = -25;        //Extra penalty for corner traps
    const int EDGE_TRAP_PENALTY = -12;          //Extra penalty for edge pieces with low mobility
    const int BLOCKED_BY_OWN_PENALTY = -10;     //Blocked by own pawns

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

    // Check if square is in corner
    inline bool is_corner(int square) {
        return (square == 0 || square == 7 || square == 56 || square == 63);
    }

    // Check if square is on edge
    inline bool is_edge(int square) {
        int file = square % 8;
        int rank = square / 8;
        return (file == 0 || file == 7 || rank == 0 || rank == 7);
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

    // uses piece square tables to give positional bonuses/penalties based on where pieces are located
    int evaluate_piece_square_tables(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                                    Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                                    Bitboards::Bitboard white_queens, Bitboards::Bitboard white_kings,
                                    Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                                    Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                                    Bitboards::Bitboard black_queens, Bitboards::Bitboard black_kings,
                                    bool is_endgame);

    // looks at pawn features such as passed pawns, isolated pawns, doubled pawns
    int evaluate_pawn_structure(Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard occupied);

    // focused on rooks, bonuses for open files and semi-open files
    int evaluate_piece_activity(Bitboards::Bitboard white_rooks, Bitboards::Bitboard black_rooks,
                                Bitboards::Bitboard white_pawns, Bitboards::Bitboard black_pawns,
                                Bitboards::Bitboard occupied);

    // inspects pawn-shields in front of each king, penalizes semi/open files, attack pressure, scales the result down in endgames
    int evaluate_king_safety(Bitboards::Bitboard white_kings,
                            Bitboards::Bitboard black_kings,
                            Bitboards::Bitboard white_pawns,
                            Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard white_knights,
                            Bitboards::Bitboard black_knights,
                            Bitboards::Bitboard white_bishops,
                            Bitboards::Bitboard black_bishops,
                            Bitboards::Bitboard white_rooks,
                            Bitboards::Bitboard black_rooks,
                            Bitboards::Bitboard white_queens,
                            Bitboards::Bitboard black_queens,
                            Bitboards::Bitboard occupied,
                            bool is_endgame);
    // Trapped piece evaluation
    int evaluate_trapped_pieces(Bitboards::Bitboard white_knights,
                                Bitboards::Bitboard white_bishops,
                                Bitboards::Bitboard white_rooks,
                                Bitboards::Bitboard white_queens,
                                Bitboards::Bitboard black_knights,
                                Bitboards::Bitboard black_bishops,
                                Bitboards::Bitboard black_rooks,
                                Bitboards::Bitboard black_queens,
                                Bitboards::Bitboard white_pawns,
                                Bitboards::Bitboard black_pawns,
                                Bitboards::Bitboard all_white,
                                Bitboards::Bitboard all_black,
                                Bitboards::Bitboard occupied);

    int count_piece_mobility(int square, 
                            Bitboards::Bitboard friendly_pieces,
                            Bitboards::Bitboard enemy_pieces,
                            Bitboards::Bitboard occupied,
                            bool is_knight, bool is_bishop, 
                            bool is_rook, bool is_queen);

    int evaluate_knight_outposts(Bitboards::Bitboard white_knights,
                                Bitboards::Bitboard black_knights,
                                Bitboards::Bitboard white_pawns,
                                Bitboards::Bitboard black_pawns);   
                            
    int evaluate_connected_rooks(Bitboards::Bitboard white_rooks,
                                Bitboards::Bitboard black_rooks,
                                Bitboards::Bitboard occupied);
    
    int evaluate_pawn_chains(Bitboards::Bitboard white_pawns,
                        Bitboards::Bitboard black_pawns);
                
    int evaluate_center_control(Bitboards::Bitboard white_pawns,
                            Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard white_pieces,
                            Bitboards::Bitboard black_pieces);

    int evaluate_space(Bitboards::Bitboard white_pawns,
                  Bitboards::Bitboard black_pawns,
                  Bitboards::Bitboard white_pieces,
                  Bitboards::Bitboard black_pieces);

    int evaluate_backward_pawns(Bitboards::Bitboard white_pawns,
                           Bitboards::Bitboard black_pawns);

} // namespace Evaluation


