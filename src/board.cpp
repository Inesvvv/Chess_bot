// Project: Chesster - Simple Chess Engine
// File: board.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "board.h"
#include "magic.h"
#include <cstdlib>
#include <iostream>

// Memo:
// rank = rows (1 to 8)
// file = columns (a to h)

using namespace Bitboards;

Board::Board()
{
    // Inititalize bitboards, attacks and masks (later we don't have that yet but don't forget to uncomment)
    Bitboards::init_bitboards();
    Bitboards::init_attack_tables();
    // Bitboards::init_masks();
    Magic::init_magics(); // init magic bitboards for slider attacks
}

// utility fuction that pops least significant and returns index (= gets all positions of our pieces)
int Board::pop_lsb(Bitboards::Bitboard &b) const
{
    if (b == 0)
        return -1;                     // no pieces left or we have iterated through all
    int position = __builtin_ctzll(b); // built in fucntion that counts the trailign zeros before the first 1 (= returns the position of the first piece)
    b &= b - 1;                        // removes the least significant bit (= the piece we just found)
    return position;
}

// Checks if a square is attacked by a given color (opponent)
bool Board::is_square_attacked(int square, bool by_white) const
{
    Bitboard sq_bit = bit(square);

    if (by_white)
    {
        // white pawn attacks
        if (pawn_attacks_black[square] & white_pawns)
            return true;

        // white knight attacks
        if (knight_attacks[square] & white_knights)
            return true;

        // white king attacks
        if (king_attacks[square] & white_kings)
            return true;

        // white bishop and queen attacks (diagonal)
        Bitboard bishop_attackers = Magic::get_bishop_attacks(square, occupied);
        if (bishop_attackers & (white_bishops | white_queens))
            return true;

        // white rook and queen attacks (straight)
        Bitboard rook_attackers = Magic::get_rook_attacks(square, occupied);
        if (rook_attackers & (white_rooks | white_queens))
            return true;
    }
    else
    {
        // black pawn attacks
        if (pawn_attacks_white[square] & black_pawns)
            return true;

        // black knight attacks
        if (knight_attacks[square] & black_knights)
            return true;

        // black king attacks
        if (king_attacks[square] & black_kings)
            return true;

        // black bishop and queen attacks (diagonal)
        Bitboard bishop_attackers = Magic::get_bishop_attacks(square, occupied);
        if (bishop_attackers & (black_bishops | black_queens))
            return true;

        // black rook and queen attacks (straight)
        Bitboard rook_attackers = Magic::get_rook_attacks(square, occupied);
        if (rook_attackers & (black_rooks | black_queens))
            return true;
    }

    return false;
}

// helper function to find king's square
int Board::find_king(bool by_white) const
{
    Bitboard king = by_white ? white_kings : black_kings;

    if (king == 0)
    {
        return -1;
    }

    // check whether king bitboard has a bit on a square i
    for (int i = 0; i < 64; i++)
    {
        if (king & bit(i))
        {
            return i;
        }
    }

    return -1; // fallback but shouldn't happen
}

// detect check and double check
int Board::count_checkers(bool white_to_move) const
{

    // find the king's sqaure
    int king_square = find_king(white_to_move);
    int king_rank = king_square / 8;
    int king_file = king_square % 8;

    bool by_white = !white_to_move;

    int attackers = 0;

    // pawn check
    if (by_white)
    {
        if (king_rank > 0)
        {
            if (king_file < 7)
            {
                int from = (king_rank - 1) * 8 + (king_file + 1);
                if (white_pawns & bit(from))
                {
                    attackers += 1;
                }
            }
            if (king_file > 0)
            {
                int from = (king_rank - 1) * 8 + (king_file - 1);
                if (white_pawns & bit(from))
                {
                    attackers += 1;
                }
            }
        }
    }
    else
    {
        if (king_rank < 7)
        {
            if (king_file > 0)
            {
                int from = (king_rank + 1) * 8 + (king_file - 1);
                if (black_pawns & bit(from))
                {
                    attackers += 1;
                }
            }
            if (king_file < 7)
            {
                int from = (king_rank + 1) * 8 + (king_file + 1);
                if (black_pawns & bit(from))
                {
                    attackers += 1;
                }
            }
        }
    }

    // knight check
    Bitboard knight_mask = knight_attacks[king_square];
    Bitboard knight_attackers = knight_mask & (by_white ? white_knights : black_knights);

    for (int i = 0; i < 64; i++)
    {
        if (knight_attackers & bit(i))
        {
            attackers += 1;
        }
    }

    // king check
    Bitboard king_mask = king_attacks[king_square];
    Bitboard king_attackers = king_mask & (by_white ? white_kings : black_kings);

    for (int i = 0; i < 64; i++)
    {
        if (king_attackers & bit(i))
        {
            attackers += 1;
        }
    }

    // bishop and queen diagonal check
    Bitboard bishop_rays = Magic::get_bishop_attacks(king_square, occupied);
    Bitboard diagonal_attackers = bishop_rays & ((by_white ? white_bishops : black_bishops) | (by_white ? white_queens : black_queens));

    for (int i = 0; i < 64; i++)
    {
        if (diagonal_attackers & bit(i))
        {
            attackers += 1;
        }
    }

    // rook and queen straight check
    Bitboard rook_rays = Magic::get_rook_attacks(king_square, occupied);
    Bitboard straight_attackers = rook_rays & ((by_white ? white_rooks : black_rooks) | (by_white ? white_queens : black_queens));

    for (int i = 0; i < 64; i++)
    {
        if (straight_attackers & bit(i))
        {
            attackers += 1;
        }
    }

    return attackers;
}

std::vector<Move> Board::generate_moves() const
{
    std::vector<Move> moves; // create our vector that will contain all the possible moves
    Bitboard pieces;
    Bitboard enemy_pieces;
    if (team_white)
    {
        pieces = all_white; // our bitboard is the one with all the white pieces
        enemy_pieces = all_black;
    }
    else
    {
        pieces = all_black;
        enemy_pieces = all_white;
    }

    // ==== KNIGHT MOVES =======
    Bitboard knights;
    Bitboard enemy_knights;
    if (team_white)
    {
        knights = white_knights;
        enemy_knights = black_knights;
    }
    else
    {
        knights = black_knights;
        enemy_knights = white_knights;
    }
    // So now our knights are a just a bitboard but we want the specific positions so use our helper function
    Bitboard temp_knights = knights; // we create temporary bitboard so don't modify  original
    while (temp_knights)
    {
        int from = pop_lsb(temp_knights);
        Bitboard attacks = knight_attacks[from] & ~pieces; // all the spots the knight can move to AND all the spots where our pieces are not

        Bitboard temp_attacks = attacks;
        while (temp_attacks)
        {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to}); // found possible move so add to the move vector
        }
    }

    // ======= KINSG MOVES ===========
    Bitboard king;
    Bitboard enemy_king;
    if (team_white)
    {
        king = white_kings;
        enemy_king = black_kings;
    }
    else
    {
        king = black_kings;
        enemy_king = white_kings;
    }
    Bitboard temp_king = king;
    while (temp_king)
    {
        int from = pop_lsb(temp_king);
        Bitboard attacks = king_attacks[from] & ~pieces;
        Bitboard temp_attacks = attacks;
        while (temp_attacks)
        {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
        }

        if (team_white)
        {
            // White kingside castling (e1 -> g1)
            if (white_castle_kingside)
            {
                // Check: king on e1, rook on h1, f1 and g1 are empty
                if ((white_kings & bit(sq(4, 0))) && // King on e1
                    (white_rooks & bit(sq(7, 0))) && // Rook on h1
                    !(occupied & bit(sq(5, 0))) &&   // f1 empty
                    !(occupied & bit(sq(6, 0))))
                { // g1 empty
                    // King must not be in check, not pass through check, not land in check
                    if (!is_square_attacked(sq(4, 0), false) && // e1 not attacked
                        !is_square_attacked(sq(5, 0), false) && // f1 not attacked
                        !is_square_attacked(sq(6, 0), false))
                    { // g1 not attacked
                        moves.push_back(Move(sq(4, 0), sq(6, 0), CASTLE_KINGSIDE));
                    }
                }
            }

            // White queenside castling (e1 -> c1)
            if (white_castle_queenside)
            {
                // Check: king on e1, rook on a1, b1, c1, and d1 are empty
                if ((white_kings & bit(sq(4, 0))) && // King on e1
                    (white_rooks & bit(sq(0, 0))) && // Rook on a1
                    !(occupied & bit(sq(1, 0))) &&   // b1 empty
                    !(occupied & bit(sq(2, 0))) &&   // c1 empty
                    !(occupied & bit(sq(3, 0))))
                { // d1 empty
                    // King must not be in check, not pass through check, not land in check
                    if (!is_square_attacked(sq(4, 0), false) && // e1 not attacked
                        !is_square_attacked(sq(3, 0), false) && // d1 not attacked
                        !is_square_attacked(sq(2, 0), false))
                    { // c1 not attacked
                        moves.push_back(Move(sq(4, 0), sq(2, 0), CASTLE_QUEENSIDE));
                    }
                }
            }
        }
        else
        {
            // Black kingside castling (e8 -> g8)
            if (black_castle_kingside)
            {
                if ((black_kings & bit(sq(4, 7))) && // King on e8
                    (black_rooks & bit(sq(7, 7))) && // Rook on h8
                    !(occupied & bit(sq(5, 7))) &&   // f8 empty
                    !(occupied & bit(sq(6, 7))))
                {                                              // g8 empty
                    if (!is_square_attacked(sq(4, 7), true) && // e8 not attacked
                        !is_square_attacked(sq(5, 7), true) && // f8 not attacked
                        !is_square_attacked(sq(6, 7), true))
                    { // g8 not attacked
                        moves.push_back(Move(sq(4, 7), sq(6, 7), CASTLE_KINGSIDE));
                    }
                }
            }

            // Black queenside castling (e8 -> c8)
            if (black_castle_queenside)
            {
                if ((black_kings & bit(sq(4, 7))) && // King on e8
                    (black_rooks & bit(sq(0, 7))) && // Rook on a8
                    !(occupied & bit(sq(1, 7))) &&   // b8 empty
                    !(occupied & bit(sq(2, 7))) &&   // c8 empty
                    !(occupied & bit(sq(3, 7))))
                {                                              // d8 empty
                    if (!is_square_attacked(sq(4, 7), true) && // e8 not attacked
                        !is_square_attacked(sq(3, 7), true) && // d8 not attacked
                        !is_square_attacked(sq(2, 7), true))
                    { // c8 not attacked
                        moves.push_back(Move(sq(4, 7), sq(2, 7), CASTLE_QUEENSIDE));
                    }
                }
            }
        }
    }

    // =========== PAWN MOVES ============
    Bitboard pawns;
    Bitboard enemy_pawns;
    if (team_white)
    {
        pawns = white_pawns;
        enemy_pawns = black_pawns;
    }
    else
    {
        pawns = black_pawns;
        enemy_pawns = white_pawns;
    }

    Bitboard temp_pawns = pawns;
    while (temp_pawns)
    {
        int from = pop_lsb(temp_pawns);
        // FOR PUSHES
        if (team_white)
        { // need to differentiate because white move from bottom to top and black from top to bottom
            int to = from + 8;
            if (to < 64 && !(occupied & bit(to)))
            {
                moves.push_back({from, to});
                // double push (only from rank 2 so index rank = 1)
                if (from / 8 == 1)
                {
                    int to2 = from + 16;
                    if (!(occupied & bit(to2)))
                        moves.push_back(Move(from, to2, DOUBLE_PUSH));
                }
            }
        }
        else
        {
            int to = from - 8;
            if (to >= 0 && !(occupied & bit(to)))
            {
                moves.push_back({from, to});
                // double push (from rank 7)
                if (from / 8 == 6)
                {
                    int to2 = from - 16;
                    if (!(occupied & bit(to2)))
                        moves.push_back(Move(from, to2, DOUBLE_PUSH));
                }
            }
        }
        // ==== capture moves ====
        Bitboard attacks;
        if (team_white)
        {
            attacks = pawn_attacks_white[from];
        }
        else
        {
            attacks = pawn_attacks_black[from];
        }
        Bitboard capture_targets = attacks & enemy_pieces;
        Bitboard temp_caps = capture_targets;
        while (temp_caps)
        {
            int to = pop_lsb(temp_caps);
            moves.push_back(Move(from, to, CAPTURE));
        }
        // En passant capture
        if (ep_square != -1)
        { // havent added the ep_square updating yet so always -1
            Bitboard ep_attack;
            if (team_white)
            {
                ep_attack = pawn_ep_attacks_white[from][ep_square];
            }
            else
            {
                ep_attack = pawn_ep_attacks_black[from][ep_square];
            }

            if (ep_attack)
                moves.push_back(Move(from, ep_square, EN_PASSANT));
        }
    }

    // ========== ROOK MOVES ==============
    Bitboard rooks = team_white ? white_rooks : black_rooks;
    Bitboard temp_rooks = rooks;
    while (temp_rooks)
    {
        int from = pop_lsb(temp_rooks);
        Bitboard attacks = Magic::get_rook_attacks(from, occupied) & ~pieces;

        Bitboard temp_attacks = attacks;
        while (temp_attacks)
        {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
        }
    }

    // ========== BISHOP MOVES ============
    Bitboard bishops = team_white ? white_bishops : black_bishops;
    Bitboard temp_bishops = bishops;
    while (temp_bishops)
    {
        int from = pop_lsb(temp_bishops);
        Bitboard attacks = Magic::get_bishop_attacks(from, occupied) & ~pieces;

        Bitboard temp_attacks = attacks;
        while (temp_attacks)
        {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
        }
    };

    // =========== QUEEN MOVES ============
    // for queen moves we just recycle bishop and rook moves
    Bitboard queens = team_white ? white_queens : black_queens;
    Bitboard temp_queens = queens;
    while (temp_queens)
    {
        int from = pop_lsb(temp_queens);
        Bitboard attacks = Magic::get_queen_attacks(from, occupied) & ~pieces;

        Bitboard temp_attacks = attacks;
        while (temp_attacks)
        {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
        }
    };



    return moves;
}

// generate legal moves
std::vector<Move> Board::generate_legal_moves() const
{
    std::vector<Move> psuedo_legal = generate_moves();
    std::vector<Move> legal;

    for (int i = 0; i < psuedo_legal.size(); i++)
    {
        Move m = psuedo_legal[i];

        Board copy = *this;
        copy.make_move(m);

        int king_square = copy.find_king(!copy.team_white);
        bool in_check = copy.is_square_attacked(king_square, copy.team_white);

        if (!in_check)
        {
            legal.push_back(m);
        }
    }

    return legal;
}

// for now, return a random move from all moves
Move Board::get_random_move() const
{
    std::vector<Move> moves = generate_legal_moves();

    // for now this is a temporary output if there are no legal moves; avoids division by zero
    if (moves.empty())
    {
        return {0, 0};
    }

    // get a random index from moves and return it
    int index = rand() % moves.size();
    return moves[index];
}

// applies a given move to the board and updates who the current player is
void Board::make_move(const Move &move)
{
    int from = move.from;
    int to = move.to;

    Bitboard from_bit = bit(from);
    Bitboard to_bit = bit(to);

    // ==== Update make move with move types =======
    ep_square = -1; // reset en passant square by default
    bool isPawn = false;
    if (team_white)
        isPawn = (white_pawns & from_bit);
    else
        isPawn = (black_pawns & from_bit);
    if (move.flag == DOUBLE_PUSH)
    {
        if (team_white)
        {
            ep_square = from + 8; // square behind the pawn after double push
        }
        else
        {
            ep_square = from - 8;
        }
    }
    // === En passant capture ==
    if (move.flag == EN_PASSANT)
    {
        int captured_pawn_square;
        if (team_white)
        {
            captured_pawn_square = to - 8;
            black_pawns &= ~bit(captured_pawn_square); // remove the captured pawn
        }
        else
        {
            captured_pawn_square = to + 8;
            white_pawns &= ~bit(captured_pawn_square);
        };
        // move the pawn to to
        if (team_white)
        {
            white_pawns &= ~from_bit;
            white_pawns |= to_bit;
        }
        else
        {
            black_pawns &= ~from_bit;
            black_pawns |= to_bit;
        }
        // Skip to the end of the move handling logic
        // Update the general bitboards and switch teams
        all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
        all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
        occupied = all_white | all_black;

        // Switch teams
        team_white = !team_white;
        return;
    }
    // === Promotion===
    bool is_promotion = (move.flag == PROMO_QUEEN || move.flag == PROMO_ROOK ||
                         move.flag == PROMO_BISHOP || move.flag == PROMO_KNIGHT ||
                         move.flag == PROMO_CAPTURE_Q || move.flag == PROMO_CAPTURE_R ||
                         move.flag == PROMO_CAPTURE_B || move.flag == PROMO_CAPTURE_K);
    if (is_promotion && isPawn)
    {
        // add the promoted piece
        if (team_white)
        {
            white_pawns &= ~from_bit;
            if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q)
                white_queens |= to_bit;
            else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R)
                white_rooks |= to_bit;
            else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B)
                white_bishops |= to_bit;
            else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K)
                white_knights |= to_bit;
        }
        else
        {
            black_pawns &= ~from_bit;
            if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q)
                black_queens |= to_bit;
            else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R)
                black_rooks |= to_bit;
            else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B)
                black_bishops |= to_bit;
            else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K)
                black_knights |= to_bit;
        }
        // If capture type remove the captured piece
        if (move.flag == PROMO_CAPTURE_Q || move.flag == PROMO_CAPTURE_R ||
            move.flag == PROMO_CAPTURE_B || move.flag == PROMO_CAPTURE_K)
        {
            if (team_white)
            {
                black_pawns &= ~to_bit;
                black_knights &= ~to_bit;
                black_bishops &= ~to_bit;
                black_rooks &= ~to_bit;
                black_queens &= ~to_bit;
                black_kings &= ~to_bit;
            }
            else
            {
                white_pawns &= ~to_bit;
                white_knights &= ~to_bit;
                white_bishops &= ~to_bit;
                white_rooks &= ~to_bit;
                white_queens &= ~to_bit;
                white_kings &= ~to_bit;
            }
        }
        // skip to the end and update the bitboards
        all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
        all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
        occupied = all_white | all_black;

        team_white = !team_white;
        return;
    }

    ep_square = -1;

    // ===== CASTLING SPECIAL CASES =====
    if (move.flag == CASTLE_KINGSIDE || move.flag == CASTLE_QUEENSIDE) {
        if (team_white) {
            white_kings &= ~from_bit;
            white_kings |= to_bit;

            if (move.flag == CASTLE_KINGSIDE) {
                Bitboard rook_from = bit(sq(7, 0));
                Bitboard rook_to   = bit(sq(5, 0));
                white_rooks &= ~rook_from;
                white_rooks |= rook_to;
            } else { // CASTLE_QUEENSIDE
                Bitboard rook_from = bit(sq(0, 0));
                Bitboard rook_to   = bit(sq(3, 0));
                white_rooks &= ~rook_from;
                white_rooks |= rook_to;
            }

            white_castle_kingside = false;
            white_castle_queenside = false;
        } else {
            // Black castling
            black_kings &= ~from_bit;
            black_kings |= to_bit;

            if (move.flag == CASTLE_KINGSIDE) {
                Bitboard rook_from = bit(sq(7, 7));
                Bitboard rook_to   = bit(sq(5, 7));
                black_rooks &= ~rook_from;
                black_rooks |= rook_to;
            } else { // CASTLE_QUEENSIDE
                Bitboard rook_from = bit(sq(0, 7));
                Bitboard rook_to   = bit(sq(3, 7));
                black_rooks &= ~rook_from;
                black_rooks |= rook_to;
            }

            black_castle_kingside = false;
            black_castle_queenside = false;
        }

        // Update combined bitboards
        all_white = white_pawns | white_knights | white_bishops |
                    white_rooks | white_queens | white_kings;
        all_black = black_pawns | black_knights | black_bishops |
                    black_rooks | black_queens | black_kings;
        occupied = all_white | all_black;

        // Switch side to move
        team_white = !team_white;
        return;
    }

    // remove captured pieces
    if (team_white)
    {
        black_pawns &= ~to_bit;
        black_knights &= ~to_bit;
        black_bishops &= ~to_bit;
        black_rooks &= ~to_bit;
        black_queens &= ~to_bit;
        black_kings &= ~to_bit;
    }
    else
    {
        white_pawns &= ~to_bit;
        white_knights &= ~to_bit;
        white_bishops &= ~to_bit;
        white_rooks &= ~to_bit;
        white_queens &= ~to_bit;
        white_kings &= ~to_bit;
    }

    // move the piece (if a piece is on the square from_bit, remove it from from_bit, and put it on to_bit)
    if (team_white)
    {
        if (white_pawns & from_bit)
        {
            white_pawns &= ~from_bit;
            white_pawns |= to_bit;
        }
        else if (white_knights & from_bit)
        {
            white_knights &= ~from_bit;
            white_knights |= to_bit;
        }
        else if (white_bishops & from_bit)
        {
            white_bishops &= ~from_bit;
            white_bishops |= to_bit;
        }
        else if (white_rooks & from_bit)
        {
            white_rooks &= ~from_bit;
            white_rooks |= to_bit;

            if (from == sq(0, 0))
                white_castle_queenside = false;
            if (from == sq(7, 0))
                white_castle_kingside = false;
        }
        else if (white_queens & from_bit)
        {
            white_queens &= ~from_bit;
            white_queens |= to_bit;
        }
        else if (white_kings & from_bit)
        {
            white_kings &= ~from_bit;
            white_kings |= to_bit;

            white_castle_kingside = false;
            white_castle_queenside = false;
        }
    }
    else
    {
        if (black_pawns & from_bit)
        {
            black_pawns &= ~from_bit;
            black_pawns |= to_bit;
        }
        else if (black_knights & from_bit)
        {
            black_knights &= ~from_bit;
            black_knights |= to_bit;
        }
        else if (black_bishops & from_bit)
        {
            black_bishops &= ~from_bit;
            black_bishops |= to_bit;
        }
        else if (black_rooks & from_bit)
        {
            black_rooks &= ~from_bit;
            black_rooks |= to_bit;

            if (from == sq(0, 7))
                black_castle_queenside = false;
            if (from == sq(7, 7))
                black_castle_kingside = false;
        }
        else if (black_queens & from_bit)
        {
            black_queens &= ~from_bit;
            black_queens |= to_bit;
        }
        else if (black_kings & from_bit)
        {
            black_kings &= ~from_bit;
            black_kings |= to_bit;

            black_castle_kingside = false;
            black_castle_queenside = false;
        }
    }

    // update the general bitboards
    all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
    occupied = all_white | all_black;

    // switch teams
    team_white = !team_white;
}

// for testing purposes
void Board::print_board() const
{
    std::cout << "\n  +-----------------+\n";
    for (int rank = 7; rank >= 0; --rank)
    {
        std::cout << rank + 1 << " | ";
        for (int file = 0; file < 8; ++file)
        {
            int sq_idx = rank * 8 + file;
            Bitboard b = bit(sq_idx);
            char c = '.';

            // White pieces (uppercase)
            if (white_pawns & b)
                c = 'P';
            else if (white_knights & b)
                c = 'N';
            else if (white_bishops & b)
                c = 'B';
            else if (white_rooks & b)
                c = 'R';
            else if (white_queens & b)
                c = 'Q';
            else if (white_kings & b)
                c = 'K';
            // Black pieces (lowercase)
            else if (black_pawns & b)
                c = 'p';
            else if (black_knights & b)
                c = 'n';
            else if (black_bishops & b)
                c = 'b';
            else if (black_rooks & b)
                c = 'r';
            else if (black_queens & b)
                c = 'q';
            else if (black_kings & b)
                c = 'k';

            std::cout << c << ' ';
        }
        std::cout << "|\n";
    }
    std::cout << "  +-----------------+\n";
    std::cout << "    a b c d e f g h\n";
    std::cout << (team_white ? "Side to move: White\n" : "Side to move: Black\n");
}
