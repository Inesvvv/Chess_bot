// Project: Chesster - Simple Chess Engine
// File: bitboards.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "bitboards.h"
#include "board.h"

namespace Bitboards
{

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
    std::array<Bitboard, 64> pawn_attacks_white;
    std::array<Bitboard, 64> pawn_attacks_black;
    std::array<std::array<Bitboard, 64>, 64> pawn_ep_attacks_white;
    std::array<std::array<Bitboard, 64>, 64> pawn_ep_attacks_black;
    std::array<Bitboard, 64> passed_pawn_white;
    std::array<Bitboard, 64> passed_pawn_black;
    std::array<Bitboard, 64> isolated_pawn_mask;

    // --------------- Initialize piece positions ----------------
    void init_bitboards()
    {
        // White pieces
        white_pawns = 0x000000000000FF00ULL; // rank 2
        white_rooks = bit(sq(0, 0)) | bit(sq(7, 0));
        white_knights = bit(sq(1, 0)) | bit(sq(6, 0));
        white_bishops = bit(sq(2, 0)) | bit(sq(5, 0));
        white_queens = bit(sq(3, 0));
        white_kings = bit(sq(4, 0));

        // Black pieces
        black_pawns = 0x00FF000000000000ULL; // rank 7
        black_rooks = bit(sq(0, 7)) | bit(sq(7, 7));
        black_knights = bit(sq(1, 7)) | bit(sq(6, 7));
        black_bishops = bit(sq(2, 7)) | bit(sq(5, 7));
        black_queens = bit(sq(3, 7));
        black_kings = bit(sq(4, 7));

        // Combined
        all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
        all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
        occupied = all_white | all_black;
        empty = ~occupied;
    }

    // --------------- Initialize file/rank masks ----------------
    void init_masks()
    {
        for (int f = 0; f < 8; ++f)
        {
            Bitboard file = 0ULL;
            Bitboard rank = 0ULL;
            for (int r = 0; r < 8; ++r)
            {
                file |= bit(sq(f, r));
                rank |= bit(sq(0, r) + f); // rank1 + file offset
            }
            file_mask[f] = file;
            rank_mask[f] = rank; // will fix in practice
        }
    }

    // attack tables prob pas dans bitboards mais peut etre dans board ou moves
    // --------------- Initialize attack tables ----------------
    void init_attack_tables()
    {
        for (int sq_index = 0; sq_index < 64; ++sq_index)
        {
            // Knight
            Bitboard attacks = 0ULL;
            int r = sq_index / 8;
            int f = sq_index % 8;
            int dr[8] = {2, 1, -1, -2, -2, -1, 1, 2};
            int df[8] = {1, 2, 2, 1, -1, -2, -2, -1};
            for (int i = 0; i < 8; ++i)
            {
                int nr = r + dr[i];
                int nf = f + df[i];
                if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
                    attacks |= bit(sq(nf, nr));
            }
            knight_attacks[sq_index] = attacks;

            // King
            attacks = 0ULL;
            int drk[8] = {1, 1, 1, 0, 0, -1, -1, -1};
            int dfk[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
            for (int i = 0; i < 8; ++i)
            {
                int nr = r + drk[i];
                int nf = f + dfk[i];
                if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8)
                    attacks |= bit(sq(nf, nr));
            }
            king_attacks[sq_index] = attacks;

            // =========== PAWNS ===========
            //Bitboard attacks = 0ULL;
            //int r = sq_index / 8;
            //int f = sq_index % 8;
            // Separate white and black pawn moves because black go down and white go up
            // === WHITE Normal Capture (diagonal)=====
            attacks = 0ULL;
            // capture left
            if (r + 1 < 8 && f - 1 >= 0)
                attacks |= bit(sq(f - 1, r + 1));
            // capture right
            if (r + 1 < 8 && f + 1 < 8)
                attacks |= bit(sq(f + 1, r + 1));
            pawn_attacks_white[sq_index] = attacks;

            // === BLACK Normal Capture (diagonal)=====
            attacks = 0ULL;
            // capture left
            if (r - 1 >= 0 && f - 1 >= 0)
                attacks |= bit(sq(f - 1, r - 1));
            // capture right
            if (r - 1 >= 0 && f + 1 < 8)
                attacks |= bit(sq(f + 1, r - 1));
            pawn_attacks_black[sq_index] = attacks;

            // ======= EN PASSANT Attack Tables ===========
            // En passant only happens when opposite team pawn pushes twice and our pawn moves diagonaly to the position behind it and can thus capture the opposing pawn
            for (int ep = 0; ep < 64; ep++)
            {
                pawn_ep_attacks_white[sq_index][ep] = 0ULL;
                pawn_ep_attacks_black[sq_index][ep] = 0ULL;

                int ep_r = ep / 8;
                int ep_f = ep % 8;
                // == WHITE ===
                if (ep_r == 5 && r == 4)
                {
                    if (ep_f == f - 1)
                        pawn_ep_attacks_white[sq_index][ep] = bit(ep);
                    if (ep_f == f + 1)
                        pawn_ep_attacks_white[sq_index][ep] = bit(ep);
                }

                // ==== BLACK ====
                if (ep_r == 2 && r == 3)
                {
                    if (ep_f == f - 1)
                        pawn_ep_attacks_black[sq_index][ep] = bit(ep);
                    if (ep_f == f + 1)
                        pawn_ep_attacks_black[sq_index][ep] = bit(ep);
                }
            }
            // ==== PASSED MASK (not using it for move generation but evaluation later because a passed pawn is very powerful) ===
            // ======= WHITE PASSED MASK =====
            Bitboard white_mask = 0ULL;
            for (int rr = r + 1; rr < 8; rr++)
            { // all ranks ABOVE the pawn
                // need to check the three surrounding files (f, f-1 and f+1)
                white_mask |= bit(sq(f, rr)); // f
                if (f - 1 >= 0)
                    white_mask |= bit(sq(f - 1, rr)); // f-1
                if (f + 1 < 8)
                    white_mask |= bit(sq(f + 1, rr)); // f+1
            }
            passed_pawn_white[sq_index] = white_mask;

            // ====== BLACK PASSED MASK =======
            Bitboard black_mask = 0ULL;
            for (int rr = 0; rr < r; rr++)
            {
                black_mask |= bit(sq(f, rr));
                if (f - 1 >= 0)
                    black_mask |= bit(sq(f - 1, rr)); // f-1
                if (f + 1 < 8)
                    black_mask |= bit(sq(f + 1, rr)); // f+1
            }
            passed_pawn_black[sq_index] = black_mask;

            // ===== ISOLATED MASK (same for both) ====
            // Isolated pawn is a pawn that has no same team pawn on either side file (so can't be protected = weak)
            // If a same team pawn is in the isolated mask then the current pawn is not isolated
            Bitboard isolated_mask = 0ULL;
            if (f - 1 >= 0)
            {
                for (int rr = 0; rr < 8; rr++)
                {
                    isolated_mask |= bit(sq(f - 1, rr));
                }
            }
            if (f + 1 < 8)
            {
                for (int rr = 0; rr < 8; rr++)
                {
                    isolated_mask |= bit(sq(f + 1, rr));
                }
            }
            isolated_pawn_mask[sq_index] = isolated_mask;
        }
    }

} // namespace Bitboards

// Restore implementation
namespace Bitboards {
    BoardState snapshot_state()
    {
        BoardState s;
        s.white_pawns = white_pawns;
        s.white_knights = white_knights;
        s.white_bishops = white_bishops;
        s.white_rooks = white_rooks;
        s.white_queens = white_queens;
        s.white_kings = white_kings;

        s.black_pawns = black_pawns;
        s.black_knights = black_knights;
        s.black_bishops = black_bishops;
        s.black_rooks = black_rooks;
        s.black_queens = black_queens;
        s.black_kings = black_kings;

        s.all_white = all_white;
        s.all_black = all_black;
        s.occupied = occupied;
        s.empty = empty;

        return s;
    }

    void restore_state(const BoardState &s)
    {
        white_pawns = s.white_pawns;
        white_knights = s.white_knights;
        white_bishops = s.white_bishops;
        white_rooks = s.white_rooks;
        white_queens = s.white_queens;
        white_kings = s.white_kings;

        black_pawns = s.black_pawns;
        black_knights = s.black_knights;
        black_bishops = s.black_bishops;
        black_rooks = s.black_rooks;
        black_queens = s.black_queens;
        black_kings = s.black_kings;

        all_white = s.all_white;
        all_black = s.all_black;
        occupied = s.occupied;
        empty = s.empty;
    }
} // namespace Bitboards
