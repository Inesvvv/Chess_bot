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

        std::vector<Move> generate_moves() const; // returns a vector of all possible moves of our team : ONLY PSEUDO-LEGAL MOVES
        std::vector<Move> generate_legal_moves() const; // legal move filtering
        Move get_random_move() const; // for now, return a random move from all moves

        void make_move(const Move& move); // applies a given move to the board and updates who the current player is

        int find_king(bool white) const; // find the king's square

        int count_checkers(bool white_to_move) const; // detect check and double check

        //for testing purposes
        void print_board() const;
    private:
        int pop_lsb(Bitboards::Bitboard &b) const; // utilty fucntion tat iterats through all positions to find where the pieces are 

    };

