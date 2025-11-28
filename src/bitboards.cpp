#include "bitboards.h"

namespace Bitboards {

// --------------- Piece bitboards ----------------
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

// --------------- Combined bitboards ----------------
Bitboard all_white;
Bitboard all_black;
Bitboard occupied;
Bitboard empty;

// --------------- Masks ----------------
std::array<Bitboard, 8> file_mask;
std::array<Bitboard, 8> rank_mask;
std::array<Bitboard, 64> knight_attacks;
std::array<Bitboard, 64> king_attacks;

// --------------- Initialize piece positions ----------------
void init_bitboards() {
    // White pieces
    white_pawns   = 0x0000000000001100ULL; // rank 2
    white_rooks   = bit(sq(0,0)) | bit(sq(7,0));
    white_knights = bit(sq(1,0)) | bit(sq(6,0));
    white_bishops = bit(sq(2,0)) | bit(sq(5,0));
    white_queens  = bit(sq(3,0));
    white_kings   = bit(sq(4,0));

    // Black pieces
    black_pawns   = 0x0011000000000000ULL; // rank 7
    black_rooks   = bit(sq(0,7)) | bit(sq(7,7));
    black_knights = bit(sq(1,7)) | bit(sq(6,7));
    black_bishops = bit(sq(2,7)) | bit(sq(5,7));
    black_queens  = bit(sq(3,7));
    black_kings   = bit(sq(4,7));

    // Combined
    all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
    occupied  = all_white | all_black;
    empty     = ~occupied;
}

// --------------- Initialize file/rank masks ----------------
void init_masks() {
    for(int f=0; f<8; ++f) {
        Bitboard file = 0ULL;
        Bitboard rank = 0ULL;
        for(int r=0; r<8; ++r) {
            file |= bit(sq(f,r));
            rank |= bit(sq(0,r) + f); // rank1 + file offset
        }
        file_mask[f] = file;
        rank_mask[f] = rank; // will fix in practice
    }
}

// --------------- Initialize attack tables ----------------
void init_attack_tables() {
    for(int sq_index=0; sq_index<64; ++sq_index) {
        // Knight
        Bitboard attacks = 0ULL;
        int r = sq_index / 8;
        int f = sq_index % 8;
        int dr[8] = {2,1,-1,-2,-2,-1,1,2};
        int df[8] = {1,2,2,1,-1,-2,-2,-1};
        for(int i=0;i<8;++i) {
            int nr = r + dr[i];
            int nf = f + df[i];
            if(nr>=0 && nr<8 && nf>=0 && nf<8)
                attacks |= bit(sq(nf,nr));
        }
        knight_attacks[sq_index] = attacks;

        // King
        attacks = 0ULL;
        int drk[8] = {1,1,1,0,0,-1,-1,-1};
        int dfk[8] = {-1,0,1,-1,1,-1,0,1};
        for(int i=0;i<8;++i) {
            int nr = r + drk[i];
            int nf = f + dfk[i];
            if(nr>=0 && nr<8 && nf>=0 && nf<8)
                attacks |= bit(sq(nf,nr));
        }
        king_attacks[sq_index] = attacks;
    }
}

} // namespace Bitboards
