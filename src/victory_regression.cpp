// Project: Chesster - Simple Chess Engine
// File: victory_regression.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12.09

// Purpose: to play a mini-match vs random moves to test victory rate of our engine against random player

#include <chrono> // to work with time
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "board.h"
#include "search.h"

namespace {

struct Config {
    int games = 10;
    int depth = 6;
    int maxPlies = 200;
    unsigned int seed = 42;
};

std::string outcome_to_string(const std::string &id) {
    return id;
}

Config parse_args(int argc, char **argv) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--games" && i + 1 < argc) {
            cfg.games = std::stoi(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            cfg.depth = std::stoi(argv[++i]);
        } else if (arg == "--max-plies" && i + 1 < argc) {
            cfg.maxPlies = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            cfg.seed = static_cast<unsigned int>(std::stoul(argv[++i]));
        }
    }
    return cfg;
}

// choose a random legal move to simulate the dumb opponent
Move pick_baseline_move(const std::vector<Move> &moves, std::mt19937 &rng) {
    if (moves.empty()) {
        return Move{-1, -1, QUIET};
    }
    std::uniform_int_distribution<std::size_t> dist(0, moves.size() - 1);
    return moves[dist(rng)];
}

enum class Outcome { EngineWin, BaselineWin, Draw };

struct GameStats {
    Outcome outcome = Outcome::Draw;
    int plies = 0;
    int engineMoves = 0;
    double engineTimeMs = 0.0;
    uint64_t totalNodes = 0;
    int maxDepth = 0;
};

// creates a Board, loops up to maxPlies, alternates between chesster and the baseline player
// tracks outcome, plies (= one side making a single move), and chesster think time
GameStats play_single_game(const Config &cfg, bool enginePlaysWhite, std::mt19937 &rng) {
    Board board;
    GameStats stats;

    for (int ply = 0; ply < cfg.maxPlies; ++ply) {
        bool engine_turn = (board.team_white == enginePlaysWhite);
        std::vector<Move> moves = board.generate_moves();
        if (moves.empty()) {
            if (board.in_check()) {
                stats.outcome = engine_turn ? Outcome::BaselineWin : Outcome::EngineWin;
            } else {
                stats.outcome = Outcome::Draw;
            }
            return stats;
        }

        Move chosen(-1, -1, QUIET);
        if (engine_turn) {
            auto start = std::chrono::steady_clock::now();
            SearchResult result = find_best_move(board, cfg.depth);
            auto end = std::chrono::steady_clock::now();
            chosen = result.bestMove;
            stats.engineTimeMs += std::chrono::duration<double, std::milli>(end - start).count();
            stats.engineMoves += 1;
            stats.totalNodes += result.nodes_explored;
            if (result.depth_reached > stats.maxDepth) {
                stats.maxDepth = result.depth_reached;
            }

            if (chosen.from < 0 || chosen.to < 0) {
                stats.outcome = Outcome::BaselineWin;
                return stats;
            }
        } else {
            chosen = pick_baseline_move(moves, rng);
        }

        board.make_move(chosen);
        stats.plies += 1;
    }

    stats.outcome = Outcome::Draw;
    return stats;
}

std::string outcome_to_string(Outcome outcome) {
    switch (outcome) {
    case Outcome::EngineWin:
        return "engine win";
    case Outcome::BaselineWin:
        return "baseline win";
    default:
        return "draw";
    }
}

} // namespace

// run play_single_game() "games" amount of times, alternating colors each games
// gather win/draw counts and timing
// print summary report
int main(int argc, char **argv) {
    Config cfg = parse_args(argc, argv);
    std::mt19937 rng(cfg.seed);

    int engineWins = 0;
    int baselineWins = 0;
    int draws = 0;
    int totalPlies = 0;
    double totalEngineTimeMs = 0.0;
    int totalEngineMoves = 0;
    uint64_t totalNodesExplored = 0;
    int overallMaxDepth = 0;

    for (int game = 0; game < cfg.games; ++game) {
        bool engineWhite = (game % 2 == 0);
        GameStats stats = play_single_game(cfg, engineWhite, rng);

        totalPlies += stats.plies;
        totalEngineTimeMs += stats.engineTimeMs;
        totalEngineMoves += stats.engineMoves;
        totalNodesExplored += stats.totalNodes;
        if (stats.maxDepth > overallMaxDepth) {
            overallMaxDepth = stats.maxDepth;
        }

        switch (stats.outcome) {
        case Outcome::EngineWin:
            engineWins += 1;
            break;
        case Outcome::BaselineWin:
            baselineWins += 1;
            break;
        case Outcome::Draw:
            draws += 1;
            break;
        }

        std::cout << "Game " << (game + 1)
                  << " (" << (engineWhite ? "engine as White" : "engine as Black")
                  << ") -> " << outcome_to_string(stats.outcome)
                  << ", plies: " << stats.plies
                  << ", nodes: " << stats.totalNodes
                  << ", max depth: " << stats.maxDepth << '\n';
    }

    double winRate = (cfg.games > 0) ? (100.0 * engineWins / cfg.games) : 0.0;
    double avgEngineTime = (totalEngineMoves > 0) ? (totalEngineTimeMs / totalEngineMoves) : 0.0;

    std::cout << "\n========= Victory Summary =========\n";
    std::cout << "Games: " << cfg.games << '\n';
    std::cout << "Engine wins: " << engineWins << '\n';
    std::cout << "Baseline wins: " << baselineWins << '\n';
    std::cout << "Draws: " << draws << '\n';
    std::cout << "Win rate: " << winRate << "%\n";
    std::cout << "Average plies: " << (cfg.games ? (totalPlies / cfg.games) : 0) << '\n';
    std::cout << "Avg engine think time: " << avgEngineTime << " ms/move\n";
    std::cout << "Total nodes explored: " << totalNodesExplored << '\n';
    std::cout << "Avg nodes per move: " << (totalEngineMoves > 0 ? totalNodesExplored / totalEngineMoves : 0) << '\n';
    std::cout << "Max depth reached: " << overallMaxDepth << '\n';
    std::cout << "===================================\n";

    return 0;
}
