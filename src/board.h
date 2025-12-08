// Project: Chesster - Simple Chess Engine
// File: board.h
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

// AIMS of this file:
// - determine who is to move 
// - constructor that initializes the position 
// - move generatiion 
// - choosing the random move ( for now we will use the evaluation map later )

#include "bitboards.h"
#include "move.h"
#include <array>
#pragma once
#include <cstdint>
#include <vector>
#include <random>

class Board {
    // for now we just have this but later need to add undo_move, add_move.. 
    public:
        bool team_white = true; // determines which team white or black 
        int ep_square = -1; // equal to the en passant square if there is one else -1 
        bool white_castle_kingside = true;
        bool white_castle_queenside = true;
        bool black_castle_kingside = true;
        bool black_castle_queenside = true;
        bool is_square_attacked(int square, bool by_white) const; // checks if a square is attacked by opponent
        Board();

        std::vector<Move> generate_moves() const; // returns only LEGAL moves
        std::vector<Move> generate_pseudo_legal_moves() const; // generates all pseudo-legal moves without legality check
        bool is_move_legal(const Move &m) const; // checks if a move is legal (doesn't leave king in check)
        Move get_random_move() const; // for now return a random move from all moves

        void make_move(const Move& move); // applies a given move to the board and updates who the current player is
        void unmake_move(const Move& move); // reverts a given move on the board and updates who the current player is

        bool in_check() const; //checks if the current player is in check

        int find_king(bool white) const; // find the king's square

        int count_checkers(bool white_to_move) const; // detect check and double check

        Bitboards::Bitboard compute_pinned(bool for_white) const; // returns mask of pinned pieces for the given color
        void compute_pin_masks(bool for_white, std::array<Bitboards::Bitboard, 64> &out_masks) const; // per-square allowed move masks for pinned pieces

        Move parse_uci_move(const std::string &uci) const;

        //for testing purposes
        void print_board() const;
        
    private:
        int pop_lsb(Bitboards::Bitboard &b) const; // utilty fucntion tat iterats through all positions to find where the pieces are 
        //stores everything that changes between moves
        struct UndoState {
            Bitboards::BoardState bb_state;
            bool team_white;
            int ep_square;
            bool white_castle_kingside;
            bool white_castle_queenside;
            bool black_castle_kingside;
            bool black_castle_queenside;
        };

        std::vector<UndoState> undo_stack;

    };

