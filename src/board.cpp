#include "board.h"
#include <cstdlib>


// Memo: 
// rank = rows (1 to 8)
// file = columns (a to h)

using namespace Bitboards; 

Board::Board(){
    // Inititalize bitboards, attacks and masks (later we don't have that yet but don't forget to uncomment)
    Bitboards::init_bitboards();
    Bitboards::init_attack_tables();
    // Bitboards::init_masks();
}

// utility fuction that pops least significant and returns index (= gets all positions of our pieces)
int Board::pop_lsb(Bitboard &b) const{
    if (b==0) return -1; // no pieces left or we have iterated through all 
    int position = __builtin_ctzll(b); // built in fucntion that counts the trailign zeros before the first 1 (= returns the position of the first piece)
    b &= b - 1; // removes the least significant bit (= the piece we just found)
    return position;
}

// ================ GENERATE ALL THE MOVES ===========================
std::vector<Move> Board::generate_moves() const {
    std::vector<Move> moves; // create our vector that will contain all the possible moves
    Bitboard pieces;
    Bitboard enemy_pieces;
    if (team_white){
        pieces = all_white; // our bitboard is the one with all the white pieces
        enemy_pieces = all_black;
    }
    else {
        pieces = all_black; 
        enemy_pieces = all_white;
    }

    // ==== KNIGHT MOVES ======= 
    Bitboard knights;
    Bitboard enemy_knights;
    if (team_white){
        knights = white_knights; 
        enemy_knights = black_knights;
    }
    else {
        knights = black_knights; 
        enemy_knights = white_knights;
    }
    // So now our knights are a just a bitboard but we want the specific positions so use our helper function
    Bitboard temp_knights = knights; // we create temporary bitboard so don't modify  original 
    while (temp_knights){
        int from = pop_lsb(temp_knights);
        Bitboard attacks = knight_attacks[from] & ~pieces; // all the spots the knight can move to AND all the spots where our pieces are not 

        Bitboard temp_attacks = attacks;
        while (temp_attacks) {
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to}); // found possible move so add to the move vector 
            }
        }
    
    // ======= KINSG MOVES ===========
    Bitboard king; 
    Bitboard enemy_king; 
    if (team_white){
        king = white_kings; 
        enemy_king = black_kings; 
    }
    else{
        king = black_kings; 
        enemy_king = white_kings; 
    }
    Bitboard temp_king = king;
    while (temp_king){
        int from = pop_lsb(temp_king);
        Bitboard attacks = king_attacks[from] & ~pieces;
        Bitboard temp_attacks = attacks; 
        while (temp_attacks){
            int to = pop_lsb(temp_attacks);
            moves.push_back({from, to});
            }
        }
    
    // =========== PAWN MOVES ============
    Bitboard pawns; 
    Bitboard enemy_pawns; 
    if (team_white){
        pawns = white_pawns; 
        enemy_pawns = black_pawns; 
    }
    else{
        pawns = black_pawns; 
        enemy_pawns = white_pawns; 
    }

    Bitboard temp_pawns = pawns;
    while (pawns){
        int from = pop_lsb(temp_pawns);
        int to; 
        if (team_white){ // need to differentiate because white move from bottom to top and black from top to bottom
            to = from + 8;
            if (to < 64 && !(occupied & bit(to))){
                moves.push_back({from,to});
                }
            int left_capture = from + 7; // here left_capture is +7 because the bitboard is from left to right and bottom to top 
            if (left_capture < 64 && (enemy_pawns & bit(left_capture))){
                moves.push_back({from, left_capture});
                }
            int right_capture = from + 9; 
            if (right_capture < 64 && (enemy_pawns & bit(right_capture))){
                moves.push_back({from, right_capture});
                }
            }
        else{
            to = from - 8;
            if (to >= 0 && !(occupied & bit(to))) {
                moves.push_back({from, to});
                }

            int left_capture = from - 9;
            if (left_capture >= 0 && (enemy_pawns & bit(left_capture))) {
                moves.push_back({from, left_capture});
                }

            int right_capture = from - 7;
            if (right_capture >= 0 && (enemy_pawns & bit(right_capture))) {
                moves.push_back({from, right_capture});
                }
            }
        }
    
    
    // =========== QUEEN MOVES ============


    // ========== TOWER MOVES ==============
    Bitboard rooks; 
    Bitboard enemy_rooks; 
    if (team_white){
        rooks = white_rooks; 
        enemy_rooks = black_pawns; 
    }
    else{
        rooks = black_rooks; 
        enemy_rooks = white_rooks; 
    }

    Bitboard temp_rooks = rooks;
    while (temp_rooks){
        int from = pop_lsb(temp_rooks);
        int to;
        const int rank = from / 8;
        const int file = from % 8;
        if (team_white){ // need to differentiate because white move from bottom to top and black from top to bottom
            //increasing in rank
            for (int r = 1; r < 8 - rank; r++) {
                to = from + 8 * r; //increasing in rank for white rook is in steps of (+8) bits in the bitboard
                if (to < 64 && !(all_white & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
            //decreasing in rank
            for (int r = rank; r > 0; r--) {
                to = from - 8 * r; //decreasing in rank for white rook is in steps of (-8) bits in the bitboard
                if (to < 64 && !(all_white & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
             //increasing in file
             for (int r = 1; r < 8 - file; r++) {
                to = from + r; //increasing in file for white rook is in steps of (+1) bits in the bitboard
                if (to / 8 != rank) { 
                    break;  //rank check to prevent wrap arounds
                }
                if (to < 64 && !(all_white & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
            //decreasing in file
            for (int r = file; r > 0; r--) {
                to = from - r; //decreasing in file for white rook is in steps of (-1) bits in the bitboard
                if (to / 8 != rank) { 
                    break;  //rank check to prevent wrap arounds
                }
                if (to < 64 && !(all_white & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            } 
        }          
        else{
            //increasing in rank
            for (int r = 1; r < 8 - rank; r++) {
                to = from - 8 * r; //increasing in rank for black rook is in steps of (-8) bits in the bitboard
                if (to < 64 && !(all_black & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
            //decreasing in rank
            for (int r = rank; r > 0; r--) {
                to = from + 8 * r; //decreasing in rank for black rook is in steps of (+8) bits in the bitboard
                if (to < 64 && !(all_black & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
             //increasing in file
             for (int r = 1; r < 8 - file; r++) {
                to = from - r; //increasing in file for black rook is in steps of (-1) bits in the bitboard
                if (to / 8 != rank) { 
                    break;  //rank check to prevent wrap arounds
                }
                if (to < 64 && !(all_black & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
            //decreasing in file
            for (int r = file; r > 0; r--) {
                to = from + r; //decreasing in file for black rook is in steps of (+1) bits in the bitboard
                if (to / 8 != rank) { 
                    break;  //rank check to prevent wrap arounds
                }
                if (to < 64 && !(all_black & bit(to))){
                    moves.push_back({from,to});
                }
                if (occupied & bit(to)) {  
                    break; //stop sliding rook if collision with another piece
                }
            }
        }
    
    }
    // ========== BISHOP MOVES ============
    Bitboard bishops;
    Bitboard enemy_bishops;
    if (team_white) {
        bishops = white_bishops;
        enemy_bishops = black_bishops;
    } else {
        bishops = black_bishops;
        enemy_bishops = white_bishops;
    }
    Bitboard temp_bishops = bishops;
    while (temp_bishops) {
        int from = pop_lsb(temp_bishops);
        int to;
        int rank = from / 8;
        int file = from % 8;
        // Upper right diagonal 
        for (int step = 1; step < 8; step++) {
            int r = rank + step;
            int f = file + step;
            if (r >= 8 || f >= 8) break;
            to = r * 8 + f;

            if (!(pieces & bit(to))) moves.push_back({from, to});  //handles the cases when enemy pieces are in the position (adds the move if its enemy piece or empty)
            if (occupied & bit(to)) break; // stops the loop when there is a piece on the path 
        };

        // Upper left diagonal 
        for (int step = 1; step < 8; step++) {
            int r = rank + step;
            int f = file - step;
            if (r >= 8 || f < 0 ) break; 
            to = r*8 + f; 
            if (!(pieces & bit(to))) moves.push_back({from,to});
            if (occupied & bit(to)) break; 
        }

        // Lower right diagonal 
        for (int step = 1; step < 8; step++) {
            int r = rank - step;
            int f = file + step;
            if (r < 0 || f >= 8) break;
            to = r * 8 + f;

            if (!(pieces & bit(to))) moves.push_back({from, to});  
            if (occupied & bit(to)) break; 
        };

        // Lower left diagonal 
        for (int step = 1; step < 8; step++) {
            int r = rank - step;
            int f = file - step;
            if (r < 0 || f < 0) break;
            to = r * 8 + f;

            if (!(pieces & bit(to))) moves.push_back({from, to});  
            if (occupied & bit(to)) break;
        };
    }
    return moves;
}

// for now, return a random move from all moves
Move Board::get_random_move() const {
    std::vector<Move> moves = generate_moves();

    // for now this is a temporary output if there are no legal moves; avoids division by zero
    if(moves.empty()){
        return {0, 0};
    }

    // get a random index from moves and return it 
    int index = rand() % moves.size();
    return moves[index];
}

// applies a given move to the board and updates who the current player is
void Board::make_move(const Move& move){
    int from = move.from;
    int to = move.to;

    Bitboard from_bit = bit(from);
    Bitboard to_bit = bit(to);

    // remove captured pieces
    if(team_white){
        black_pawns   &= ~to_bit;
        black_knights &= ~to_bit;
        black_bishops &= ~to_bit;
        black_rooks   &= ~to_bit;
        black_queens  &= ~to_bit;
        black_kings   &= ~to_bit;
    } else {
        white_pawns   &= ~to_bit;
        white_knights &= ~to_bit;
        white_bishops &= ~to_bit;
        white_rooks   &= ~to_bit;
        white_queens  &= ~to_bit;
        white_kings   &= ~to_bit;
    }

    // move the piece (if a piece is on the square from_bit, remove it from from_bit, and put it on to_bit)
    if (team_white) {
        if (white_pawns & from_bit) {
            white_pawns &= ~from_bit;
            white_pawns |=  to_bit;
        } else if (white_knights & from_bit) {
            white_knights &= ~from_bit;
            white_knights |=  to_bit;
        } else if (white_bishops & from_bit) {
            white_bishops &= ~from_bit;
            white_bishops |=  to_bit;
        } else if (white_rooks & from_bit) {
            white_rooks &= ~from_bit;
            white_rooks |=  to_bit;
        } else if (white_queens & from_bit) {
            white_queens &= ~from_bit;
            white_queens |=  to_bit;
        } else if (white_kings & from_bit) {
            white_kings &= ~from_bit;
            white_kings |=  to_bit;
        }
    } else {
        if (black_pawns & from_bit) {
            black_pawns &= ~from_bit;
            black_pawns |=  to_bit;
        } else if (black_knights & from_bit) {
            black_knights &= ~from_bit;
            black_knights |=  to_bit;
        } else if (black_bishops & from_bit) {
            black_bishops &= ~from_bit;
            black_bishops |=  to_bit;
        } else if (black_rooks & from_bit) {
            black_rooks &= ~from_bit;
            black_rooks |=  to_bit;
        } else if (black_queens & from_bit) {
            black_queens &= ~from_bit;
            black_queens |=  to_bit;
        } else if (black_kings & from_bit) {
            black_kings &= ~from_bit;
            black_kings |=  to_bit;
        }
    }

    // update the general bitboards 
    all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
    occupied = all_white | all_black;

    // switch teams
    team_white = !team_white;
}

