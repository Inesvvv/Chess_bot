#pragma once
#include "board.h"
#include "move.h"

struct SearchResult
{
    Move bestMove;
    int score;
    SearchResult(const Move& m, int s) : bestMove(m), score(s) {}
};

SearchResult find_best_move(Board &board, int maxDepth);
