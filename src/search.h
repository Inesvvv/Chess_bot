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
// Configure the soft time limit (milliseconds) used by the iterative deepening loop
void set_time_limit_ms(int milliseconds);

// Clear the transposition table (call between games)
void clear_transposition_table();
// Compute zobrist hash for a board position (used for repetition detection)
uint64_t compute_hash(const Board &board);
