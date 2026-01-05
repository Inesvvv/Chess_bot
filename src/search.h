#pragma once
#include "board.h"
#include "move.h"
#include <cstdint>

struct SearchResult
{
    Move bestMove;
    int score;
    uint64_t nodes_explored;
    int depth_reached;
    SearchResult(const Move& m, int s, uint64_t nodes = 0, int depth = 0) 
        : bestMove(m), score(s), nodes_explored(nodes), depth_reached(depth) {}
};

SearchResult find_best_move(Board &board, int maxDepth);

// Clear the transposition table (call between games)
void clear_transposition_table();

// Compute zobrist hash for a board position (used for repetition detection)
uint64_t compute_hash(const Board &board);

// Try to get a move from the opening book. Returns true if found, false otherwise.
// If successful, the book_move will be set to the best book move.
bool try_opening_book_move(Board &board, Move& book_move);
