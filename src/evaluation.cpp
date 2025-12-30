// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04

#include "evaluation.h"
#include "board.h"
#include "magic.h"

namespace Evaluation {

//calculate phase (for now endgame/midgame) for tappered eval
// ==================== PHASE CALCULATION ====================
int calculate_phase(Bitboards::Bitboard white_knights,
                    Bitboards::Bitboard white_bishops,
                    Bitboards::Bitboard white_rooks,
                    Bitboards::Bitboard white_queens,
                    Bitboards::Bitboard black_knights,
                    Bitboards::Bitboard black_bishops,
                    Bitboards::Bitboard black_rooks,
                    Bitboards::Bitboard black_queens) {
    int phase = 0;
    
    phase += popcount(white_knights) * PHASE_KNIGHT;
    phase += popcount(black_knights) * PHASE_KNIGHT;
    phase += popcount(white_bishops) * PHASE_BISHOP;
    phase += popcount(black_bishops) * PHASE_BISHOP;
    phase += popcount(white_rooks) * PHASE_ROOK;
    phase += popcount(black_rooks) * PHASE_ROOK;
    phase += popcount(white_queens) * PHASE_QUEEN;
    phase += popcount(black_queens) * PHASE_QUEEN;
    
    if (phase > TOTAL_PHASE) phase = TOTAL_PHASE;
    if (phase < 0) phase = 0;
    
    return phase;
}

// counts each piece type for both sides, applies standard piece values, adds a bishop pair bonus if a side has two bishops
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

// uses piece square tables to give positional bonuses/penalties based on where pieces are located
// flips the tables for black pieces and switches the king tables in the endgame
int evaluate_piece_square_tables(Bitboards::Bitboard white_pawns, Bitboards::Bitboard white_knights,
                                 Bitboards::Bitboard white_bishops, Bitboards::Bitboard white_rooks,
                                 Bitboards::Bitboard white_queens, Bitboards::Bitboard white_kings,
                                 Bitboards::Bitboard black_pawns, Bitboards::Bitboard black_knights,
                                 Bitboards::Bitboard black_bishops, Bitboards::Bitboard black_rooks,
                                 Bitboards::Bitboard black_queens, Bitboards::Bitboard black_kings,
                                 bool is_endgame)
{
    int score = 0;
    
    // helper to evaluate a piece type
    // sums up positional bonuses/penalties for all pieces of a given type, using piece-square table
    auto eval_pieces = [&](Bitboards::Bitboard pieces, const int* table, bool is_white, int multiplier) {
        Bitboards::Bitboard temp = pieces;
        while (temp) {
            int sq = __builtin_ctzll(temp); // find the index (0-63) of the least significant bit that is set to 1 in the 64-bit integer temp
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
    
    // King: choose table based on game phase (middlegame when must keep safe or endgame when must be active)
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

// looks at pawn features such as passed pawns, isolated pawns, and doubled pawns
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

//focused on rooks, bonuses for open files and semi-open files
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
        int rank = sq / 8;
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        
        // Open file (no pawns)
        if ((file_mask & (white_pawns | black_pawns)) == 0) {
            score += ROOK_OPEN_FILE_BONUS;
        }
        // Semi-open file (no friendly pawns)
        else if ((file_mask & white_pawns) == 0) {
            score += ROOK_SEMI_OPEN_FILE_BONUS;
        }
        // 7th rank rook
        if (rank == 6) {
            score += ROOK_ON_SEVENTH_BONUS;
        }
    }
    
    // Evaluate black rooks
    Bitboards::Bitboard temp_black_rooks = black_rooks;
    while (temp_black_rooks) {
        int sq = __builtin_ctzll(temp_black_rooks);
        temp_black_rooks &= temp_black_rooks - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        
        // Open file
        if ((file_mask & (white_pawns | black_pawns)) == 0) {
            score -= ROOK_OPEN_FILE_BONUS;
        }
        // Semi-open file
        else if ((file_mask & black_pawns) == 0) {
            score -= ROOK_SEMI_OPEN_FILE_BONUS;
        }
        if (rank == 1) {
            score -= ROOK_ON_SEVENTH_BONUS;
        }
    }
    
    return score;
}

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
                         bool is_endgame)
{
    // helper that scores one side's king (positive if well defended)
    auto evaluate_side = [&](bool white_side) -> int {
        Bitboards::Bitboard king_bb = white_side ? white_kings : black_kings;
        if (king_bb == 0)
        {
            return 0;
        }

        // figure out which bitboards belong to the side we are evaluating
        Bitboards::Bitboard friendly_pawns = white_side ? white_pawns : black_pawns;
        Bitboards::Bitboard enemy_pawns = white_side ? black_pawns : white_pawns;
        Bitboards::Bitboard enemy_knights = white_side ? black_knights : white_knights;
        Bitboards::Bitboard enemy_bishops = white_side ? black_bishops : white_bishops;
        Bitboards::Bitboard enemy_rooks = white_side ? black_rooks : white_rooks;
        Bitboards::Bitboard enemy_queens = white_side ? black_queens : white_queens;
        Bitboards::Bitboard enemy_king_bb = white_side ? black_kings : white_kings;

        // locate the king square and direction it moves (forward = towards enemy)
        int king_sq = __builtin_ctzll(king_bb);
        int king_file = king_sq % 8;
        int king_rank = king_sq / 8;
        int forward = white_side ? 1 : -1;

        // utility that pops the least-significant bit and returns that square index
        auto pop_lsb = [](Bitboards::Bitboard &bb) -> int {
            int sq = __builtin_ctzll(bb);
            bb &= (bb - 1);
            return sq;
        };

        // check for pawn shields directly in front of the king (same file +/- 1)
        auto add_shield = [&](int df, int steps, int bonus) -> int {
            int target_file = king_file + df;
            int target_rank = king_rank + forward * steps;
            if (target_file < 0 || target_file >= 8 || target_rank < 0 || target_rank >= 8)
            {
                return 0;
            }
            int sq = Bitboards::sq(target_file, target_rank);
            if (friendly_pawns & Bitboards::bit(sq))
            {
                return bonus;
            }
            return -KING_SHIELD_GAP_PENALTY;
        };

        // reward intact pawn shields, penalize gaps
        int shield_score = 0;
        for (int df = -1; df <= 1; ++df)
        {
            shield_score += add_shield(df, 1, KING_SHIELD_NEAR_BONUS);
            shield_score += add_shield(df, 2, KING_SHIELD_FAR_BONUS);
        }

        // penalize semi-open/open files near the king
        int open_penalty = 0;
        for (int df = -1; df <= 1; ++df)
        {
            int file = king_file + df;
            if (file < 0 || file >= 8)
            {
                continue;
            }
            Bitboards::Bitboard mask = Bitboards::file_mask[file];
            if ((mask & friendly_pawns) == 0)
            {
                open_penalty += KING_SEMI_OPEN_FILE_PENALTY;
                if ((mask & enemy_pawns) == 0)
                {
                    open_penalty += KING_OPEN_FILE_PENALTY;
                }
            }
        }

        // determine squares around the king and collect all enemy attacks hitting them
        Bitboards::Bitboard king_zone = Bitboards::king_attacks[king_sq] | Bitboards::bit(king_sq);
        Bitboards::Bitboard enemy_attacks = 0ULL;

        Bitboards::Bitboard temp = enemy_pawns;
        while (temp)
        {
            int sq = pop_lsb(temp);
            enemy_attacks |= white_side ? Bitboards::pawn_attacks_black[sq]
                                        : Bitboards::pawn_attacks_white[sq];
        }

        temp = enemy_knights;
        while (temp)
        {
            int sq = pop_lsb(temp);
            enemy_attacks |= Bitboards::knight_attacks[sq];
        }

        temp = enemy_bishops;
        while (temp)
        {
            int sq = pop_lsb(temp);
            enemy_attacks |= Magic::get_bishop_attacks(sq, occupied);
        }

        temp = enemy_rooks;
        while (temp)
        {
            int sq = pop_lsb(temp);
            enemy_attacks |= Magic::get_rook_attacks(sq, occupied);
        }

        temp = enemy_queens;
        while (temp)
        {
            int sq = pop_lsb(temp);
            enemy_attacks |= Magic::get_queen_attacks(sq, occupied);
        }

        if (enemy_king_bb)
        {
            int enemy_king_sq = __builtin_ctzll(enemy_king_bb);
            enemy_attacks |= Bitboards::king_attacks[enemy_king_sq];
        }

        // count how many king-zone squares are attacked to quantify danger
        int attacked = popcount(enemy_attacks & king_zone);
        int danger_penalty = attacked * KING_ZONE_ATTACK_PENALTY;

        // combine shield, open-file, and attack pressure; taper in endgame
        int side_score = shield_score - open_penalty - danger_penalty;
        if (is_endgame)
        {
            side_score = side_score * KING_SAFETY_ENDGAME_SCALE / 100;
        }
        return side_score;
    };

    int white_score = evaluate_side(true);
    int black_score = evaluate_side(false);
    return white_score - black_score;
}


//mobility penalties (trapped pieces)
int count_piece_mobility(int square, 
                        Bitboards::Bitboard friendly_pieces,
                        Bitboards::Bitboard enemy_pieces,
                        Bitboards::Bitboard occupied,
                        bool is_knight, bool is_bishop, 
                        bool is_rook, bool is_queen)
    {
    Bitboards::Bitboard attacks = 0ULL;
    
    if (is_knight) {
        attacks = Bitboards::knight_attacks[square];
    }
    else if (is_bishop) {
        attacks = Magic::get_bishop_attacks(square, occupied);
    }
    else if (is_rook) {
        attacks = Magic::get_rook_attacks(square, occupied);
    }
    else if (is_queen) {
        attacks = Magic::get_queen_attacks(square, occupied);
    }
    
    // Remove squares occupied by friendly pieces
    attacks &= ~friendly_pieces;
   
    return popcount(attacks);
}

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
                            Bitboards::Bitboard occupied)
{
    int score = 0;
    
    auto evaluate_side = [&](Bitboards::Bitboard knights, Bitboards::Bitboard bishops,
                            Bitboards::Bitboard rooks, Bitboards::Bitboard queens,
                            Bitboards::Bitboard friendly, Bitboards::Bitboard enemy,
                            Bitboards::Bitboard friendly_pawns,
                            int multiplier) -> int {
        int side_penalty = 0;
        
        // ==== KNIGHTS ====
        // Normal knight has 6-8 moves, trapped has 0-2
        Bitboards::Bitboard temp_knights = knights;
        while (temp_knights) {
            int sq = __builtin_ctzll(temp_knights);
            temp_knights &= temp_knights - 1;
            
            int mobility = count_piece_mobility(sq, friendly, enemy, occupied,
                                               true, false, false, false);
            
            // Apply penalties based on mobility thresholds
            if (mobility <= 1) {
                // Completely trapped (0-1 moves)
                side_penalty += TRAPPED_KNIGHT_SEVERE;
                if (is_corner(sq)) {
                    side_penalty += CORNER_TRAP_PENALTY;
                }
            }
            else if (mobility <= 3) {
                // Very limited (2-3 moves)
                side_penalty += TRAPPED_KNIGHT_MODERATE;
                if (is_edge(sq)) {
                    side_penalty += EDGE_TRAP_PENALTY;
                }
            }
            else if (mobility <= 5) {
                // Somewhat restricted (4-5 moves)
                side_penalty += TRAPPED_KNIGHT_LIGHT;
            }
            // 6+ moves = normal knight, no penalty
            
            // Check if blocked by own pawns
            Bitboards::Bitboard knight_moves = Bitboards::knight_attacks[sq];
            if (popcount(knight_moves & friendly_pawns) >= 3) {
                side_penalty += BLOCKED_BY_OWN_PENALTY;
            }
        }
        
        // ==== BISHOPS ====
        // Normal bishop has 7-13 moves, trapped has 0-4
        Bitboards::Bitboard temp_bishops = bishops;
        while (temp_bishops) {
            int sq = __builtin_ctzll(temp_bishops);
            temp_bishops &= temp_bishops - 1;
            
            int mobility = count_piece_mobility(sq, friendly, enemy, occupied,
                                               false, true, false, false);
            
            if (mobility <= 2) {
                // Severely trapped (0-2 moves)
                side_penalty += TRAPPED_BISHOP_SEVERE;
                if (is_corner(sq)) {
                    side_penalty += CORNER_TRAP_PENALTY;
                }
            }
            else if (mobility <= 4) {
                // Moderately trapped (3-4 moves)
                side_penalty += TRAPPED_BISHOP_MODERATE;
            }
            else if (mobility <= 6) {
                // Slightly limited (5-6 moves)
                side_penalty += TRAPPED_BISHOP_LIGHT;
            }
            // 7+ moves = normal bishop, no penalty
            
            // Bad bishop: blocked by own pawns on same color squares
            Bitboards::Bitboard diagonal_attacks = Magic::get_bishop_attacks(sq, occupied);
            if (popcount(diagonal_attacks & friendly_pawns) >= 4) {
                side_penalty += BAD_BISHOP_PENALTY;
            }
        }
        
        // ==== ROOKS ====
        // Normal rook has 10-14 moves, trapped has 0-6
        Bitboards::Bitboard temp_rooks = rooks;
        while (temp_rooks) {
            int sq = __builtin_ctzll(temp_rooks);
            temp_rooks &= temp_rooks - 1;
            
            int mobility = count_piece_mobility(sq, friendly, enemy, occupied,
                                               false, false, true, false);
            
            if (mobility <= 3) {
                // Severely trapped (0-3 moves)
                side_penalty += TRAPPED_ROOK_SEVERE;
            }
            else if (mobility <= 6) {
                // Moderately trapped (4-6 moves)
                side_penalty += TRAPPED_ROOK_MODERATE;
            }
            else if (mobility <= 9) {
                // Slightly limited (7-9 moves)
                side_penalty += TRAPPED_ROOK_LIGHT;
            }
            // 10+ moves = normal rook, no penalty
            
            // Rook trapped behind own pawns
            int file = sq % 8;
            Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
            if (popcount(file_mask & friendly_pawns) >= 2) {
                side_penalty += BLOCKED_BY_OWN_PENALTY;
            }
        }
        
        // ==== QUEENS ====
        // Normal queen has 20-27 moves, trapped has 0-9
        Bitboards::Bitboard temp_queens = queens;
        while (temp_queens) {
            int sq = __builtin_ctzll(temp_queens);
            temp_queens &= temp_queens - 1;
            
            int mobility = count_piece_mobility(sq, friendly, enemy, occupied,
                                               false, false, false, true);
            
            if (mobility <= 4) {
                // Severely trapped (0-4 moves)
                side_penalty += TRAPPED_QUEEN_SEVERE;
                if (is_corner(sq) || is_edge(sq)) {
                    side_penalty += CORNER_TRAP_PENALTY;
                }
            }
            else if (mobility <= 9) {
                // Moderately trapped (5-9 moves)
                side_penalty += TRAPPED_QUEEN_MODERATE;
            }
            else if (mobility <= 14) {
                // Slightly limited (10-14 moves)
                side_penalty += TRAPPED_QUEEN_LIGHT;
            }
            // 15+ moves = normal queen, no penalty
        }
        
        return side_penalty * multiplier;
    };
    
    // Evaluate white pieces
    score += evaluate_side(white_knights, white_bishops, white_rooks, white_queens,
                          all_white, all_black, white_pawns, -1);
    
    // Evaluate black pieces
    score += evaluate_side(black_knights, black_bishops, black_rooks, black_queens,
                          all_black, all_white, black_pawns, 1);
    
    return score;
}

// Knight outpost evaluation
int evaluate_knight_outposts(Bitboards::Bitboard white_knights,
                             Bitboards::Bitboard black_knights,
                             Bitboards::Bitboard white_pawns,
                             Bitboards::Bitboard black_pawns)
{
    int score = 0;
    
    // Evaluate white knights
    Bitboards::Bitboard temp = white_knights;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int rank = sq / 8;
        int file = sq % 8;
        
        // Must be in enemy territory (5th or 6th rank for white)
        if (rank < 4) continue;
        
        // Check if defended by own pawn
        Bitboards::Bitboard pawn_defense = Bitboards::pawn_attacks_black[sq];
        bool defended_by_pawn = (pawn_defense & white_pawns) != 0;
        if (!defended_by_pawn) continue;
        
        // Check if enemy pawns can attack it
        // Look at files f-1, f, f+1 ahead of the knight
        bool safe_from_enemy_pawns = true;
        for (int df = -1; df <= 1; df++) {
            int check_file = file + df;
            if (check_file < 0 || check_file >= 8) continue;
            
            // Check all ranks ahead of this knight
            for (int check_rank = rank + 1; check_rank < 8; check_rank++) {
                int check_sq = Bitboards::sq(check_file, check_rank);
                if (black_pawns & Bitboards::bit(check_sq)) {
                    safe_from_enemy_pawns = false;
                    break;
                }
            }
            if (!safe_from_enemy_pawns) break;
        }
        
        if (safe_from_enemy_pawns) {
            // Strong outpost in center files (c-f)
            if (file >= 2 && file <= 5) {
                score += KNIGHT_OUTPOST_STRONG_BONUS;
            } else {
                score += KNIGHT_OUTPOST_BONUS;
            }
        }
    }
    
    // Evaluate black knights
    temp = black_knights;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int rank = sq / 8;
        int file = sq % 8;
        
        // Must be in enemy territory (3rd or 4th rank for black)
        if (rank > 3) continue;
        
        Bitboards::Bitboard pawn_defense = Bitboards::pawn_attacks_white[sq];
        bool defended_by_pawn = (pawn_defense & black_pawns) != 0;
        if (!defended_by_pawn) continue;
        
        bool safe_from_enemy_pawns = true;
        for (int df = -1; df <= 1; df++) {
            int check_file = file + df;
            if (check_file < 0 || check_file >= 8) continue;
            
            for (int check_rank = rank - 1; check_rank >= 0; check_rank--) {
                int check_sq = Bitboards::sq(check_file, check_rank);
                if (white_pawns & Bitboards::bit(check_sq)) {
                    safe_from_enemy_pawns = false;
                    break;
                }
            }
            if (!safe_from_enemy_pawns) break;
        }
        
        if (safe_from_enemy_pawns) {
            if (file >= 2 && file <= 5) {
                score -= KNIGHT_OUTPOST_STRONG_BONUS;
            } else {
                score -= KNIGHT_OUTPOST_BONUS;
            }
        }
    }
    
    return score;
}

// Connected rooks evaluation
int evaluate_connected_rooks(Bitboards::Bitboard white_rooks,
                             Bitboards::Bitboard black_rooks,
                             Bitboards::Bitboard occupied)
{
    int score = 0;
    
    // White rooks
    if (popcount(white_rooks) >= 2) {
        Bitboards::Bitboard temp = white_rooks;
        while (temp) {
            int sq = __builtin_ctzll(temp);
            temp &= temp - 1;
            
            // Check if another rook can "see" this one
            Bitboards::Bitboard rook_attacks = Magic::get_rook_attacks(sq, occupied);
            Bitboards::Bitboard other_rooks = white_rooks & ~Bitboards::bit(sq);
            
            if (rook_attacks & other_rooks) {
                score += CONNECTED_ROOKS_BONUS;
                break; // Only count once per pair
            }
        }
    }
    
    // Black rooks
    if (popcount(black_rooks) >= 2) {
        Bitboards::Bitboard temp = black_rooks;
        while (temp) {
            int sq = __builtin_ctzll(temp);
            temp &= temp - 1;
            
            Bitboards::Bitboard rook_attacks = Magic::get_rook_attacks(sq, occupied);
            Bitboards::Bitboard other_rooks = black_rooks & ~Bitboards::bit(sq);
            
            if (rook_attacks & other_rooks) {
                score -= CONNECTED_ROOKS_BONUS;
                break;
            }
        }
    }
    
    return score;
}

// Pawn chain evaluation
int evaluate_pawn_chains(Bitboards::Bitboard white_pawns,
                        Bitboards::Bitboard black_pawns)
{
    int score = 0;
    
    // White pawn chains
    Bitboards::Bitboard temp = white_pawns;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        // Check for diagonal support (pawn behind on adjacent file)
        if (rank > 0) {
            // Check left diagonal
            if (file > 0) {
                int support_sq = Bitboards::sq(file - 1, rank - 1);
                if (white_pawns & Bitboards::bit(support_sq)) {
                    score += PAWN_CHAIN_BONUS;
                }
            }
            // Check right diagonal
            if (file < 7) {
                int support_sq = Bitboards::sq(file + 1, rank - 1);
                if (white_pawns & Bitboards::bit(support_sq)) {
                    score += PAWN_CHAIN_BONUS;
                }
            }
        }
    }
    
    // Black pawn chains
    temp = black_pawns;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        // Check for diagonal support (pawn behind = higher rank for black)
        if (rank < 7) {
            if (file > 0) {
                int support_sq = Bitboards::sq(file - 1, rank + 1);
                if (black_pawns & Bitboards::bit(support_sq)) {
                    score -= PAWN_CHAIN_BONUS;
                }
            }
            if (file < 7) {
                int support_sq = Bitboards::sq(file + 1, rank + 1);
                if (black_pawns & Bitboards::bit(support_sq)) {
                    score -= PAWN_CHAIN_BONUS;
                }
            }
        }
    }
    
    return score;
}

// Center control evaluation
int evaluate_center_control(Bitboards::Bitboard white_pawns,
                            Bitboards::Bitboard black_pawns,
                            Bitboards::Bitboard white_pieces,
                            Bitboards::Bitboard black_pieces)
{
    int score = 0;
    
    // Main center squares: d4(27), e4(28), d5(35), e5(36)
    const int center_squares[4] = {27, 28, 35, 36};
    
    for (int i = 0; i < 4; i++) {
        int sq = center_squares[i];
        Bitboards::Bitboard sq_bit = Bitboards::bit(sq);
        
        // Pawns controlling center are most valuable
        if (white_pawns & sq_bit) {
            score += CENTER_CONTROL_BONUS;
        }
        if (black_pawns & sq_bit) {
            score -= CENTER_CONTROL_BONUS;
        }
        
        // Pieces occupying center also good
        if (white_pieces & sq_bit) {
            score += CENTER_CONTROL_BONUS / 2;
        }
        if (black_pieces & sq_bit) {
            score -= CENTER_CONTROL_BONUS / 2;
        }
    }
    
    // Extended center: c3-f3, c4-f4, c5-f5, c6-f6 (excluding main center)
    const int extended_center[12] = {
        18, 19, 20, 21,  // c3-f3
        26, 29,          // c4, f4 (d4, e4 already counted)
        34, 37,          // c5, f5 (d5, e5 already counted)
        42, 43, 44, 45   // c6-f6
    };
    
    for (int i = 0; i < 12; i++) {
        int sq = extended_center[i];
        Bitboards::Bitboard sq_bit = Bitboards::bit(sq);
        
        if (white_pawns & sq_bit) {
            score += EXTENDED_CENTER_BONUS;
        }
        if (black_pawns & sq_bit) {
            score -= EXTENDED_CENTER_BONUS;
        }
        
        if (white_pieces & sq_bit) {
            score += EXTENDED_CENTER_BONUS / 2;
        }
        if (black_pieces & sq_bit) {
            score -= EXTENDED_CENTER_BONUS / 2;
        }
    }
    
    return score;
}

// Space advantage evaluation
int evaluate_space(Bitboards::Bitboard white_pawns,
                  Bitboards::Bitboard black_pawns,
                  Bitboards::Bitboard white_pieces,
                  Bitboards::Bitboard black_pieces)
{
    int score = 0;
    
    // Count white's space in black's half (ranks 5-8 for white)
    const Bitboards::Bitboard BLACK_HALF = 0xFFFFFFFF00000000ULL; // ranks 5-8
    int white_space = popcount((white_pawns | white_pieces) & BLACK_HALF);
    score += white_space * SPACE_BONUS_PER_SQUARE;
    
    // Count black's space in white's half (ranks 1-4 for black)
    const Bitboards::Bitboard WHITE_HALF = 0x00000000FFFFFFFFULL; // ranks 1-4
    int black_space = popcount((black_pawns | black_pieces) & WHITE_HALF);
    score -= black_space * SPACE_BONUS_PER_SQUARE;
    
    return score;
}


// Backward pawn evaluation
int evaluate_backward_pawns(Bitboards::Bitboard white_pawns,
                           Bitboards::Bitboard black_pawns)
{
    int score = 0;
    
    // White backward pawns
    Bitboards::Bitboard temp = white_pawns;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        // Check if isolated (handled elsewhere, skip those)
        if ((Bitboards::isolated_pawn_mask[sq] & white_pawns) == 0) {
            continue;
        }
        
        // Check if square in front is attacked by enemy pawn
        if (rank < 7) {
            int forward_sq = sq + 8;
            Bitboards::Bitboard enemy_attacks = Bitboards::pawn_attacks_white[forward_sq];
            bool forward_attacked = (enemy_attacks & black_pawns) != 0;
            
            if (forward_attacked) {
                // Check if friendly pawns on adjacent files can still advance to defend
                bool can_be_defended = false;
                for (int df = -1; df <= 1; df += 2) {
                    int adj_file = file + df;
                    if (adj_file < 0 || adj_file >= 8) continue;
                    
                    Bitboards::Bitboard file_mask = Bitboards::file_mask[adj_file];
                    Bitboards::Bitboard adj_pawns = white_pawns & file_mask;
                    
                    while (adj_pawns) {
                        int pawn_sq = __builtin_ctzll(adj_pawns);
                        adj_pawns &= adj_pawns - 1;
                        int pawn_rank = pawn_sq / 8;
                        
                        if (pawn_rank < rank) {
                            can_be_defended = true;
                            break;
                        }
                    }
                    if (can_be_defended) break;
                }
                
                if (!can_be_defended) {
                    score += BACKWARD_PAWN_PENALTY;
                }
            }
        }
    }
    
    // Black backward pawns
    temp = black_pawns;
    while (temp) {
        int sq = __builtin_ctzll(temp);
        temp &= temp - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        if ((Bitboards::isolated_pawn_mask[sq] & black_pawns) == 0) {
            continue;
        }
        
        if (rank > 0) {
            int forward_sq = sq - 8;
            Bitboards::Bitboard enemy_attacks = Bitboards::pawn_attacks_black[forward_sq];
            bool forward_attacked = (enemy_attacks & white_pawns) != 0;
            
            if (forward_attacked) {
                bool can_be_defended = false;
                for (int df = -1; df <= 1; df += 2) {
                    int adj_file = file + df;
                    if (adj_file < 0 || adj_file >= 8) continue;
                    
                    Bitboards::Bitboard file_mask = Bitboards::file_mask[adj_file];
                    Bitboards::Bitboard adj_pawns = black_pawns & file_mask;
                    
                    while (adj_pawns) {
                        int pawn_sq = __builtin_ctzll(adj_pawns);
                        adj_pawns &= adj_pawns - 1;
                        int pawn_rank = pawn_sq / 8;
                        
                        if (pawn_rank > rank) {
                            can_be_defended = true;
                            break;
                        }
                    }
                    if (can_be_defended) break;
                }
                
                if (!can_be_defended) {
                    score -= BACKWARD_PAWN_PENALTY;
                }
            }
        }
    }
    
    return score;
}


//same functions as above but now implemented with the tapered logic

//material evaluation
TaperedScore evaluate_material_tapered(Bitboards::Bitboard white_pawns, 
                                        Bitboards::Bitboard white_knights,
                                        Bitboards::Bitboard white_bishops, 
                                        Bitboards::Bitboard white_rooks,
                                        Bitboards::Bitboard white_queens,
                                        Bitboards::Bitboard black_pawns, 
                                        Bitboards::Bitboard black_knights,
                                        Bitboards::Bitboard black_bishops, 
                                        Bitboards::Bitboard black_rooks,
                                        Bitboards::Bitboard black_queens) {
    TaperedScore score;
    
    //white material
    score.mg += popcount(white_pawns) * PAWN_VALUE_MG;
    score.mg += popcount(white_knights) * KNIGHT_VALUE_MG;
    score.mg += popcount(white_bishops) * BISHOP_VALUE_MG;
    score.mg += popcount(white_rooks) * ROOK_VALUE_MG;
    score.mg += popcount(white_queens) * QUEEN_VALUE_MG;
    
    score.eg += popcount(white_pawns) * PAWN_VALUE_EG;
    score.eg += popcount(white_knights) * KNIGHT_VALUE_EG;
    score.eg += popcount(white_bishops) * BISHOP_VALUE_EG;
    score.eg += popcount(white_rooks) * ROOK_VALUE_EG;
    score.eg += popcount(white_queens) * QUEEN_VALUE_EG;
    
    //black material
    score.mg -= popcount(black_pawns) * PAWN_VALUE_MG;
    score.mg -= popcount(black_knights) * KNIGHT_VALUE_MG;
    score.mg -= popcount(black_bishops) * BISHOP_VALUE_MG;
    score.mg -= popcount(black_rooks) * ROOK_VALUE_MG;
    score.mg -= popcount(black_queens) * QUEEN_VALUE_MG;
    
    score.eg -= popcount(black_pawns) * PAWN_VALUE_EG;
    score.eg -= popcount(black_knights) * KNIGHT_VALUE_EG;
    score.eg -= popcount(black_bishops) * BISHOP_VALUE_EG;
    score.eg -= popcount(black_rooks) * ROOK_VALUE_EG;
    score.eg -= popcount(black_queens) * QUEEN_VALUE_EG;
    
    //bishop pair bonus
    if (popcount(white_bishops) >= 2) {
        score.mg += BISHOP_PAIR_BONUS;
        score.eg += BISHOP_PAIR_BONUS;
    }
    if (popcount(black_bishops) >= 2) {
        score.mg -= BISHOP_PAIR_BONUS;
        score.eg -= BISHOP_PAIR_BONUS;
    }
    
    return score;
}

//tapered piece-square table evaluation
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
                                   Bitboards::Bitboard black_kings) {
    TaperedScore score;
    
    auto eval_pieces = [&](Bitboards::Bitboard pieces, 
                           const int* table_mg, 
                           const int* table_eg,
                           bool is_white, 
                           int multiplier) {
        Bitboards::Bitboard temp = pieces;
        while (temp) {
            int sq = __builtin_ctzll(temp);
            temp &= temp - 1;
            score.mg += multiplier * get_pst_value(table_mg, sq, is_white);
            score.eg += multiplier * get_pst_value(table_eg, sq, is_white);
        }
    };
    
    //white pieces
    eval_pieces(white_pawns, PAWN_TABLE_MG, PAWN_TABLE_EG, true, 1);
    eval_pieces(white_knights, KNIGHT_TABLE_MG, KNIGHT_TABLE_EG, true, 1);
    eval_pieces(white_bishops, BISHOP_TABLE_MG, BISHOP_TABLE_EG, true, 1);
    eval_pieces(white_rooks, ROOK_TABLE_MG, ROOK_TABLE_EG, true, 1);
    eval_pieces(white_queens, QUEEN_TABLE_MG, QUEEN_TABLE_EG, true, 1);
    eval_pieces(white_kings, KING_MIDDLEGAME_TABLE, KING_ENDGAME_TABLE, true, 1);
    
    //black pieces
    eval_pieces(black_pawns, PAWN_TABLE_MG, PAWN_TABLE_EG, false, -1);
    eval_pieces(black_knights, KNIGHT_TABLE_MG, KNIGHT_TABLE_EG, false, -1);
    eval_pieces(black_bishops, BISHOP_TABLE_MG, BISHOP_TABLE_EG, false, -1);
    eval_pieces(black_rooks, ROOK_TABLE_MG, ROOK_TABLE_EG, false, -1);
    eval_pieces(black_queens, QUEEN_TABLE_MG, QUEEN_TABLE_EG, false, -1);
    eval_pieces(black_kings, KING_MIDDLEGAME_TABLE, KING_ENDGAME_TABLE, false, -1);
    
    return score;
}

//pawn structure evaluation
TaperedScore evaluate_pawn_structure_tapered(Bitboards::Bitboard white_pawns,
                                              Bitboards::Bitboard black_pawns,
                                              Bitboards::Bitboard /* occupied */) {
    TaperedScore score;
    
    //evaluate white pawns
    Bitboards::Bitboard temp_white = white_pawns;
    while (temp_white) {
        int sq = __builtin_ctzll(temp_white);
        temp_white &= temp_white - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        //passed pawn check - more valuable in endgame
        if ((Bitboards::passed_pawn_white[sq] & black_pawns) == 0) {
            score.mg += PASSED_PAWN_BONUS_MG[rank];
            score.eg += PASSED_PAWN_BONUS_EG[rank];
        }
        
        //isolated pawn check
        if ((Bitboards::isolated_pawn_mask[sq] & white_pawns) == 0) {
            score.mg += ISOLATED_PAWN_PENALTY;
            score.eg += ISOLATED_PAWN_PENALTY;
        }
        
        //doubled pawns check
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        if (popcount(white_pawns & file_mask) > 1) {
            score.mg += DOUBLED_PAWN_PENALTY;
            score.eg += DOUBLED_PAWN_PENALTY;
        }
    }
    
    //evaluate black pawns
    Bitboards::Bitboard temp_black = black_pawns;
    while (temp_black) {
        int sq = __builtin_ctzll(temp_black);
        temp_black &= temp_black - 1;
        
        int file = sq % 8;
        int rank = sq / 8;
        
        //passed pawn check
        if ((Bitboards::passed_pawn_black[sq] & white_pawns) == 0) {
            score.mg -= PASSED_PAWN_BONUS_MG[7 - rank];
            score.eg -= PASSED_PAWN_BONUS_EG[7 - rank];
        }
        
        //isolated pawn check
        if ((Bitboards::isolated_pawn_mask[sq] & black_pawns) == 0) {
            score.mg -= ISOLATED_PAWN_PENALTY;
            score.eg -= ISOLATED_PAWN_PENALTY;
        }
        
        //doubled pawns check
        Bitboards::Bitboard file_mask = Bitboards::file_mask[file];
        if (popcount(black_pawns & file_mask) > 1) {
            score.mg -= DOUBLED_PAWN_PENALTY;
            score.eg -= DOUBLED_PAWN_PENALTY;
        }
    }
    
    return score;
}

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
                     bool white_to_move)
{
    //calculate game phase
    int phase = calculate_phase(white_knights, white_bishops, white_rooks, white_queens,
                                black_knights, black_bishops, black_rooks, black_queens);
    
    TaperedScore total_score;
    
    // 1. Material evaluation 
    total_score += evaluate_material_tapered(white_pawns, white_knights, white_bishops,
                                              white_rooks, white_queens,
                                              black_pawns, black_knights, black_bishops,
                                              black_rooks, black_queens);
    
    // 2. Piece-square tables 
    total_score += evaluate_pst_tapered(white_pawns, white_knights, white_bishops,
                                         white_rooks, white_queens, white_kings,
                                         black_pawns, black_knights, black_bishops,
                                         black_rooks, black_queens, black_kings);
    
    // 3. Pawn structure 
    total_score += evaluate_pawn_structure_tapered(white_pawns, black_pawns, occupied);
    
    //interpolate the tapered score based on phase
    int score = total_score.interpolate(phase);
    
    // 4. Piece activity (not yet tapered)
    score += evaluate_piece_activity(white_rooks, black_rooks, white_pawns, black_pawns, occupied);
    
    // 5. King safety (not yet tapered - uses legacy is_endgame)
    int white_material = popcount(white_knights) * KNIGHT_VALUE +
                         popcount(white_bishops) * BISHOP_VALUE +
                         popcount(white_rooks) * ROOK_VALUE +
                         popcount(white_queens) * QUEEN_VALUE;
    int black_material = popcount(black_knights) * KNIGHT_VALUE +
                         popcount(black_bishops) * BISHOP_VALUE +
                         popcount(black_rooks) * ROOK_VALUE +
                         popcount(black_queens) * QUEEN_VALUE;
    bool endgame = is_endgame(white_material, black_material);
    
    score += evaluate_king_safety(white_kings, black_kings,
                                   white_pawns, black_pawns,
                                   white_knights, black_knights,
                                   white_bishops, black_bishops,
                                   white_rooks, black_rooks,
                                   white_queens, black_queens,
                                   occupied, endgame);
    
    // 6. Trapped pieces
    Bitboards::Bitboard all_white = white_pawns | white_knights | white_bishops |
                                     white_rooks | white_queens | white_kings;
    Bitboards::Bitboard all_black = black_pawns | black_knights | black_bishops |
                                     black_rooks | black_queens | black_kings;
    
    score += evaluate_trapped_pieces(white_knights, white_bishops, white_rooks, white_queens,
                                      black_knights, black_bishops, black_rooks, black_queens,
                                      white_pawns, black_pawns, all_white, all_black, occupied);
    
    // 7. Knight outposts
    score += evaluate_knight_outposts(white_knights, black_knights, white_pawns, black_pawns);
    
    // 8. Connected rooks
    score += evaluate_connected_rooks(white_rooks, black_rooks, occupied);
    
    // 9. Pawn chains
    score += evaluate_pawn_chains(white_pawns, black_pawns);
    
    // 10. Center control
    score += evaluate_center_control(white_pawns, black_pawns, all_white, all_black);
    
    // 11. Space advantage
    score += evaluate_space(white_pawns, black_pawns, all_white, all_black);
    
    // 12. Backward pawns
    score += evaluate_backward_pawns(white_pawns, black_pawns);
    
    // 13. Tempo bonus
    if (white_to_move) {
        score += 10;
    } else {
        score -= 10;
    }
    
    return score;
}

// Legacy evaluate function (kept for comparison/testing)
// main function that calls all functions above
// decides if the position is in the endgame, adds everything up, includes tempo bonus
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

    // 5. King safety
    score += evaluate_king_safety(white_kings, black_kings,
                                  white_pawns, black_pawns,
                                  white_knights, black_knights,
                                  white_bishops, black_bishops,
                                  white_rooks, black_rooks,
                                  white_queens, black_queens,
                                  occupied, endgame);

    // 6. Trapped pieces evaluation
    Bitboards::Bitboard all_white = white_pawns | white_knights | white_bishops | 
                                     white_rooks | white_queens | white_kings;
    Bitboards::Bitboard all_black = black_pawns | black_knights | black_bishops | 
                                     black_rooks | black_queens | black_kings;
    
    score += evaluate_trapped_pieces(white_knights, white_bishops, white_rooks, white_queens,
                                     black_knights, black_bishops, black_rooks, black_queens,
                                     white_pawns, black_pawns,
                                     all_white, all_black, occupied);
    

    // 7. Knight outposts
    score += evaluate_knight_outposts(white_knights, black_knights, white_pawns, black_pawns);

    // 8. Connected rooks
    score += evaluate_connected_rooks(white_rooks, black_rooks, occupied);

    // 9. Pawn chains
    score += evaluate_pawn_chains(white_pawns, black_pawns);

    // 10. Center control
    score += evaluate_center_control(white_pawns, black_pawns, all_white, all_black);

    // 11. Space advantage
    score += evaluate_space(white_pawns, black_pawns, all_white, all_black);

    // 12. Backward pawns
    score += evaluate_backward_pawns(white_pawns, black_pawns);

    // 13. Tempo bonus
    if (white_to_move) {
        score += 10;
    } else {
        score -= 10;
    }
    
    return score;
}

// wrapper that grabs bitboards and side to move then calls evaluate_tapered()
// Returns score from the perspective of the team
int evaluate_board(const Board& board)
{
    int score = evaluate_tapered(Bitboards::white_pawns,
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
    
    // evaluate_tapered() returns score from White's perspective, so we negate for Black
    return board.team_white ? score : -score;
}

} // namespace Evaluation