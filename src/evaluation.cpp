// Project: Chesster - Simple Chess Engine
// File: evaluation.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-04

#include "evaluation.h"
#include "board.h"
#include "magic.h"

namespace Evaluation {

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
    
    // 6. Small bonus for side to move (tempo)
    if (white_to_move) {
        score += 10;
    } else {
        score -= 10;
    }
    
    return score;
}

// wrapper that grabs bitboards and side to move then calls evaluate()
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