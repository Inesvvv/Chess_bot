#pragma once
#include "board.h"
#include "move.h"
#include <cstdint>

struct SearchResult
{
    Move bestMove;
    int score;
    SearchResult(const Move& m, int s) : bestMove(m), score(s) {}
};

SearchResult find_best_move(Board &board, int maxDepth);

// Clear the transposition table (call between games)
void clear_transposition_table();
// Compute zobrist hash for a board position (used for repetition detection)
uint64_t compute_hash(const Board &board);
