// Project: Chesster - Simple Chess Engine
// File: board.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "board.h"
#include "magic.h"
#include <cstdlib>

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
    Magic::init_magics(); //init magic bitboards for slider attacks
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

// ================ GENERATE ALL THE MOVES ===========================
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
    while (temp_pawns){
        int from = pop_lsb(temp_pawns);
        // FOR PUSHES 
        if (team_white) { // need to differentiate because white move from bottom to top and black from top to bottom
            int to = from + 8; 
            if (to < 64 && !(occupied & bit(to))) {
                moves.push_back({from, to});
                // double push (only from rank 2 so index rank = 1)
                if (from / 8 == 1) {
                    int to2 = from + 16;
                    if (!(occupied & bit(to2))) moves.push_back(Move(from, to2, DOUBLE_PUSH));
                    }
                }
            }
        else{
            int to = from - 8; 
            if (to >= 0 && !(occupied & bit(to))){
                moves.push_back({from, to});
                // double push (from rank 7)
                if (from / 8 == 6) {
                    int to2 = from - 16;
                    if (!(occupied & bit(to2)))
                        moves.push_back({from, to2});
                    }
                }
        
            }
        // ==== capture moves ==== 
        Bitboard attacks;
        if (team_white){
            Bitboard attacks = pawn_attacks_white[from];
            }
        else{
            Bitboard attacks = pawn_attacks_black[from];
        }
        Bitboard capture_targets = attacks & enemy_pieces;
        Bitboard temp_caps = capture_targets;
        while (temp_caps){
            int to = pop_lsb(temp_caps);
            moves.push_back(Move(from, to, CAPTURE));
            }
        // En passant capture 
        if (ep_square != -1){ // havent added the ep_square updating yet so always -1
            Bitboard ep_attack;
            if (team_white){
                Bitboard attacks = pawn_ep_attacks_white[from][ep_square];
                }
            else{
                Bitboard attacks = pawn_ep_attacks_black[from][ep_square];
                }
            
            if (ep_attack) moves.push_back(Move(from, ep_square, EN_PASSANT));
            }

    
        }

    // ========== ROOK MOVES ==============
    Bitboard rooks = team_white ? white_rooks : black_rooks;
    Bitboard temp_rooks = rooks;
    while (temp_rooks){
        int from = pop_lsb(temp_rooks);
        Bitboard attacks = Magic::get_rook_attacks(from, occupied) & ~pieces;
    
        Bitboard temp_attacks = attacks;
        while (temp_attacks){
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
            }
        }

    // ========== BISHOP MOVES ============
    Bitboard bishops = team_white ? white_bishops : black_bishops;
    Bitboard temp_bishops = bishops;
    while (temp_bishops){
        int from = pop_lsb(temp_bishops);
        Bitboard attacks = Magic::get_bishop_attacks(from, occupied) & ~pieces;
    
        Bitboard temp_attacks = attacks;
        while (temp_attacks){
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
            }
        };
    

    // =========== QUEEN MOVES ============
    // for queen moves we just recycle bishop and rook moves
    Bitboard queens = team_white ? white_queens : black_queens;
    Bitboard temp_queens = queens;
    while (temp_queens){
        int from = pop_lsb(temp_queens);
        Bitboard attacks = Magic::get_queen_attacks(from, occupied) & ~pieces;
    
        Bitboard temp_attacks = attacks;
        while (temp_attacks){
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
            }
        };

    return moves;
}

// for now, return a random move from all moves
Move Board::get_random_move() const
{
    std::vector<Move> moves = generate_moves();

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
void Board::make_move(const Move &move){
    int from = move.from;
    int to = move.to;

    Bitboard from_bit = bit(from);
    Bitboard to_bit = bit(to);

    // ==== Update make move with move types ======= 
    ep_square = -1; // reset en passant square by default
    bool isPawn = false;
    if (team_white) isPawn = (white_pawns & from_bit);
    else isPawn = (black_pawns & from_bit);
    if (move.flag == DOUBLE_PUSH){
        if (team_white) {
            ep_square = from + 8; // square behind the pawn after double push
            }
        else {
            ep_square = from - 8;
            }
        }
    // === En passant capture == 
    if (move.flag == EN_PASSANT){
        int captured_pawn_square;
        if (team_white){
            captured_pawn_square = to - 8; 
            black_pawns &= ~bit(captured_pawn_square); // remove the captured pawn
            }
        else{
            captured_pawn_square = to + 8;
            white_pawns &= ~bit(captured_pawn_square);
            };
        // move the pawn to to 
        if (team_white) {
            white_pawns &= ~from_bit;
            white_pawns |= to_bit;
            } 
        else {
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
    if (is_promotion && isPawn){
        // add the promoted piece 
        if (team_white){
            white_pawns &= ~from_bit;
            if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q) white_queens |= to_bit;
            else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R) white_rooks |= to_bit;
            else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B) white_bishops |= to_bit;
            else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K) white_knights |= to_bit;
            }
        else{
            black_pawns &= ~from_bit;
            if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q) black_queens |= to_bit;
            else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R) black_rooks |= to_bit;
            else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B) black_bishops |= to_bit;
            else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K) black_knights |= to_bit;
            }
        // If capture type remove the captured piece
        if (move.flag == PROMO_CAPTURE_Q || move.flag == PROMO_CAPTURE_R ||
            move.flag == PROMO_CAPTURE_B || move.flag == PROMO_CAPTURE_K){
            if (team_white){
                black_pawns &= ~to_bit;
                black_knights &= ~to_bit;
                black_bishops &= ~to_bit;
                black_rooks &= ~to_bit;
                black_queens &= ~to_bit;
                black_kings &= ~to_bit;
                }
            else{
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
        occupied  = all_white | all_black;

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
        }
    }

    // update the general bitboards
    all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
    occupied = all_white | all_black;

    // switch teams
    team_white = !team_white;
}

