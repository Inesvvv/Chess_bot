#include "bitboards.h"
#pragma once
#include <cstdint>
#include <vector>
#include <random>

// AIMS of this file:
// - determine who is to move 
// - constructor that initializes the position 
// - move generatiion 
// - choosing the random move ( for now we will use the evaluation map later )

struct Move {
    int from;
    int to;
};

class Board {
    // for now we just have this but later need to add undo_move, add_move.. 
    public:
        bool team_white = true; // determines which team white or black 
        Board();

        std::vector<Move> generate_moves() const; // returns a vector of all possible moves of our team 
        Move get_random_move() const; // for now, return a random move from all moves

        void make_move(const Move& move); // applies a given move to the board and updates who the current player is

    private:
        int pop_lsb(Bitboards::Bitboard &b) const; // utilty fucntion tat iterats through all positions to find where the pieces are 
};

