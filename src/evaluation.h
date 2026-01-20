// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04


#pragma once
#include "bitboards.h"


class Board;

namespace Evaluation {
    //representing values of pieces, reference 1 pawn = 100
    // add midgame/endgame specific piece eval for tapered eval

    // basic values
    const int PAWN_VALUE = 100;
    const int KNIGHT_VALUE = 320;
    const int BISHOP_VALUE = 330;
    const int ROOK_VALUE = 500;
    const int QUEEN_VALUE = 900;
    const int KING_VALUE = 20000;

    // midgame
    const int PAWN_VALUE_MG = 100;
    const int KNIGHT_VALUE_MG = 315;
    const int BISHOP_VALUE_MG = 330;
    const int ROOK_VALUE_MG = 500;
    const int QUEEN_VALUE_MG = 900;
    const int KING_VALUE_MG = 20000;

    // endgame
    const int PAWN_VALUE_EG = 120;
    const int KNIGHT_VALUE_EG = 300;
    const int BISHOP_VALUE_EG = 340;
    const int ROOK_VALUE_EG = 520;
    const int QUEEN_VALUE_EG = 950;
    const int KING_VALUE_EG = 20000;
    
    //phase calculation
    const int PHASE_KNIGHT = 1;
    const int PHASE_BISHOP = 1;
    const int PHASE_ROOK = 2;
    const int PHASE_QUEEN = 4;
    const int TOTAL_PHASE = 24; 


    // we got all the following tables from YouTube videos, Internet forums and AI
    // increase of value of pieces if they are on particular squares

    //without tapered eval
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

    // Slightly softened knight centralization to avoid over-prioritising knight maneuvers
    const int KNIGHT_TABLE[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,  -2,  -2,  -2,  -2, -20, -40,
        -30,  -2,   8,  12,  12,   8,  -2, -30,
        -30,   4,  12,  17,  17,  12,   4, -30,
        -30,  -2,  12,  17,  17,  12,  -2, -30,
        -30,   4,   8,  12,  12,   8,   4, -30,
        -40, -20,  -2,   3,   3,  -2, -20, -40,
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

    // added PST tables for midgame/endgame to have distinction for tapered eval

    const int PAWN_TABLE_MG[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
        5,   5,  10,  25,  25,  10,   5,   5,
        0,   0,   0,  20,  20,   0,   0,   0,
        5,  -5, -10,   0,   0, -10,  -5,   5,
        5,  10,  10, -20, -20,  10,  10,   5,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    const int PAWN_TABLE_EG[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        80,  80,  80,  80,  80,  80,  80,  80,
        50,  50,  50,  50,  50,  50,  50,  50,
        30,  30,  30,  30,  30,  30,  30,  30,
        20,  20,  20,  20,  20,  20,  20,  20,
        10,  10,  10,  10,  10,  10,  10,  10,
        10,  10,  10,  10,  10,  10,  10,  10,
        0,   0,   0,   0,   0,   0,   0,   0
    };

    const int KNIGHT_TABLE_MG[64] = { //look at this maybe again later we move knight to the center to faast and just to many knight moves
        -45, -37, -27, -25, -25, -27, -37, -45,
        -37, -18,  -2,  -2,  -2,  -2, -18, -37,
        -27,  -2,   7,  11,  11,   7,  -2, -27,
        -25,   4,  11,  16,  16,  11,   4, -25,
        -25,  -2,  11,  16,  16,  11,  -2, -25,
        -27,   4,   7,  11,  11,   7,   4, -27,
        -37, -18,  -2,   3,   3,  -2, -18, -37,
        -45, -37, -27, -25, -25, -27, -40, -45
    };

    const int KNIGHT_TABLE_EG[64] = { 
        -50, -40, -30, -22, -22, -30, -40, -50,
        -40, -22,  -6,   0,   0,  -6, -22, -40,
        -30,  -6,   0,   9,   9,   0,  -6, -30,
        -22,   0,   9,  14,  14,   9,   0, -22,
        -22,   0,   9,  14,  14,   9,   0, -22,
        -30,  -6,   0,   9,   9,   0,  -6, -30,
        -40, -22,  -6,   0,   0,  -6, -22, -40,
        -50, -40, -30, -22, -22, -30, -40, -50
    };

    const int BISHOP_TABLE_MG[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    const int BISHOP_TABLE_EG[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   5,   5,   0,   0, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   5,  10,  15,  15,  10,   5, -10,
        -10,   5,  10,  15,  15,  10,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   5,   5,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    const int ROOK_TABLE_MG[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        5,  10,  10,  10,  10,  10,  10,   5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        0,   -2,   0,   5,   5,   0,   -2,   0
    };

    const int ROOK_TABLE_EG[64] = {
        0,   0,   0,   0,   0,   0,   0,   0,
        10,  15,  15,  15,  15,  15,  15,  10,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0
    }; 
    //maybe delete or change this since its basically the same as our 7-th rank bonus and otherwise rooks are good in endgames anyways, so PST table not so important

    const int QUEEN_TABLE_MG[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -5,   0,   5,   5,   5,   5,   0,  -5,
         0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    const int QUEEN_TABLE_EG[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -5,   0,   5,  10,  10,   5,   0,  -5,
        -5,   0,   5,  10,  10,   5,   0,  -5,
        -10,   0,   5,   5,   5,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    //we already defined a difference for king MG/EG before tapered eval so no changes here
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
    const int ROOK_OPEN_FILE_BONUS = 22;  // rook on open file
    const int ROOK_SEMI_OPEN_FILE_BONUS = 5; // rook on semi-open file
    const int PASSED_PAWN_BONUS[8] = {0, 10, 20, 40, 60, 100, 150, 0}; //before tapered
    const int PASSED_PAWN_BONUS_MG[8] = {0, 10, 20, 40, 60, 100, 150, 0}; // by rank, MG
    const int PASSED_PAWN_BONUS_EG[8] = {0, 15, 30, 60, 100, 160, 250, 0}; // by rank, EG
    const int ISOLATED_PAWN_PENALTY = -10; //for isolated pawn
    const int DOUBLED_PAWN_PENALTY = -15;
    const int MOBILITY_BONUS = 5;  // per legal move in middlegame
    const int KING_SHIELD_NEAR_BONUS = 22;   // pawn directly in front of king
    const int KING_SHIELD_FAR_BONUS = 12;     // pawn two ranks ahead of king
    const int KING_SHIELD_GAP_PENALTY = 10;   // missing pawn in the shield
    const int KING_SEMI_OPEN_FILE_PENALTY = 12; // king near semi-open file
    const int KING_OPEN_FILE_PENALTY = 6;    // additional penalty if fully open
    const int KING_ZONE_ATTACK_PENALTY = 10; // per enemy attack in king zone
    const int KING_SAFETY_ENDGAME_SCALE = 60; // percent of king safety kept in endgame
    const int ROOK_ON_SEVENTH_BONUS = 25; //active rook - might be to much with pst table
    const int CONNECTED_ROOKS_BONUS = 27; //rooks connected 
    const int KNIGHT_OUTPOST_BONUS = 12; //knight defended by own pawn not aatacked by enemy pawn + enemy ranks
    const int KNIGHT_OUTPOST_STRONG_BONUS = 25;
    const int BAD_BISHOP_PENALTY = -25; //bishop blocked by pieces
    const int BACKWARD_PAWN_PENALTY = -12; //pawn cant push safely + isolated
    const int PAWN_CHAIN_BONUS = 8; //connected pawns
    const int CENTER_CONTROL_BONUS = 15; //center is advantage
    const int EXTENDED_CENTER_BONUS = 5;
    const int SPACE_BONUS_PER_SQUARE = 2; 
    const int PINNED_PIECE_PENALTY = 8;  // small penalty for pinned pieces
    //maybe add penalty for hanging pieces + lower some bonus/malus might have to much impact
    

    //penalties for trapped pieces (3 levels of restriction of moves) look at values again might be a bit to harsh
    //knights 
    const int TRAPPED_KNIGHT_SEVERE = -45;      
    const int TRAPPED_KNIGHT_MODERATE = -26;   
    const int TRAPPED_KNIGHT_LIGHT = -12;       
    
    //bishops 
    const int TRAPPED_BISHOP_SEVERE = -45;     
    const int TRAPPED_BISHOP_MODERATE = -26;    
    const int TRAPPED_BISHOP_LIGHT = -10;       

    //rooks 
    const int TRAPPED_ROOK_SEVERE = -65;      
    const int TRAPPED_ROOK_MODERATE = -32;      
    const int TRAPPED_ROOK_LIGHT = -11;        
    
    //queens 
    const int TRAPPED_QUEEN_SEVERE = -85;      
    const int TRAPPED_QUEEN_MODERATE = -40;     
    const int TRAPPED_QUEEN_LIGHT = -15;       
    
    // Additional situational penalties (smaller, stackable)
    const int CORNER_TRAP_PENALTY = -15;        //Extra penalty for corner traps
    const int EDGE_TRAP_PENALTY = -8;          //Extra penalty for edge pieces with low mobility
    const int BLOCKED_BY_OWN_PENALTY = -6;     //Blocked by own pawns

    //tapred score
    struct TaperedScore {
        int mg;  // Middlegame score
        int eg;  // Endgame score

        TaperedScore() : mg(0), eg(0) {}
        TaperedScore(int m, int e) : mg(m), eg(e) {}

        TaperedScore operator+(const TaperedScore& other) const {
            return TaperedScore(mg + other.mg, eg + other.eg);
        }

        TaperedScore operator-(const TaperedScore& other) const {
            return TaperedScore(mg - other.mg, eg - other.eg);
        }

        TaperedScore& operator+=(const TaperedScore& other) {
            mg += other.mg;
            eg += other.eg;
            return *this;
        }

        TaperedScore& operator-=(const TaperedScore& other) {
            mg -= other.mg;
            eg -= other.eg;
            return *this;
        }

        // Interpolate between MG and EG based on phase
        int interpolate(int phase) const {
            return (mg * phase + eg * (TOTAL_PHASE - phase)) / TOTAL_PHASE;
        }
    };

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

    int evaluate_tapered(Bitboards::Bitboard white_pawns,
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

                     int calculate_phase(Bitboards::Bitboard white_knights,
                        Bitboards::Bitboard white_bishops,
                        Bitboards::Bitboard white_rooks,
                        Bitboards::Bitboard white_queens,
                        Bitboards::Bitboard black_knights,
                        Bitboards::Bitboard black_bishops,
                        Bitboards::Bitboard black_rooks,
                        Bitboards::Bitboard black_queens);
    
    TaperedScore evaluate_material_tapered(Bitboards::Bitboard white_pawns,
                                           Bitboards::Bitboard white_knights,
                                           Bitboards::Bitboard white_bishops,
                                           Bitboards::Bitboard white_rooks,
                                           Bitboards::Bitboard white_queens,
                                           Bitboards::Bitboard black_pawns,
                                           Bitboards::Bitboard black_knights,
                                           Bitboards::Bitboard black_bishops,
                                           Bitboards::Bitboard black_rooks,
                                           Bitboards::Bitboard black_queens);
    
    TaperedScore evaluate_pst_tapered(Bitboards::Bitboard white_pawns,
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
                                      Bitboards::Bitboard black_kings);
    
    TaperedScore evaluate_pawn_structure_tapered(Bitboards::Bitboard white_pawns,
                                                 Bitboards::Bitboard black_pawns,
                                                 Bitboards::Bitboard occupied);

    // pinned pieces evaluation (returns score from White's perspective)
    int evaluate_pins();

} // namespace Evaluation


