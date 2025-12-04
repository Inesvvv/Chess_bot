#include "search.h"
#include "evaluation.h"
#include <limits>
#include <vector>

static const int INF = std::numeric_limits<int>::max();

// Forward declaration of the core recursive search
int negamax(Board& board, int depth, int alpha, int beta);

// Top-level function called by your engine
SearchResult find_best_move(Board& board, int maxDepth) {
    std::vector<Move> moves = board.generate_legal_moves();

    if (moves.empty()) {
        Move dummyMove = Move(-1, -1, QUIET);
        int score;

        if (board.in_check()) {
            // side to move is checkmated
            score = -INF + 1;
        } else {
            // stalemate
            score = 0;
        }

        return SearchResult(dummyMove, score);
    }


    Move bestMove = moves[0];
    int bestScore = -INF;

    int alpha = -INF;
    int beta = INF;

    for (const Move& move : moves) {
        board.make_move(move);
        int score = -negamax(board, maxDepth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return SearchResult(bestMove, bestScore);
}


int negamax(Board& board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluate(board);
    }

    std::vector<Move> moves = board.generate_legal_moves();

    if (moves.empty()) {
        //no legal moves: checkmate or stalemate

        //if currently in check, huge negative score; else 0
        if (board.in_check()) {
            return -INF + 1; 
        } else {
            //stalemate
            return 0;
        }
    }

    int bestScore = -INF;

    for (const Move& move : moves) {
        board.make_move(move);

        int score = -negamax(board, depth - 1, -beta, -alpha);

        board.unmake_move(move);

        if (score > bestScore) {
            bestScore = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break;
        }
    }

    return bestScore;
}
