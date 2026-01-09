// Minimal UCI front-end for Chesster so we can battle Stockfish via cutechess.
// Inspired by our teammate's version, but wired up to Chesster's Board/Search APIs.

#include "board.h"
#include "move.h"
#include "search.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Access time_limit_ms from search.cpp for time management
extern int time_limit_ms;

namespace {

constexpr int kDefaultFixedDepth = 5;
constexpr int kMaxDepthCap = 64;

struct GoParams {
	int depth = 0;
	int wtime = 0;
	int btime = 0;
	int winc = 0;
	int binc = 0;
	int movestogo = 0;
	int movetime = 0;
	bool infinite = false;
};

std::string move_to_string(const Move &m) {
	if (m.from < 0 || m.to < 0) {
		return "";
	}

	auto index_to_square = [](int sq) {
		int file = sq % 8;
		int rank = sq / 8;
		char fileChar = static_cast<char>('a' + file);
		char rankChar = static_cast<char>('1' + rank);
		return std::string() + fileChar + rankChar;
	};

	std::string s = index_to_square(m.from) + index_to_square(m.to);

	switch (m.flag) {
		case PROMO_QUEEN:
		case PROMO_CAPTURE_Q:
			s += 'q';
			break;
		case PROMO_ROOK:
		case PROMO_CAPTURE_R:
			s += 'r';
			break;
		case PROMO_BISHOP:
		case PROMO_CAPTURE_B:
			s += 'b';
			break;
		case PROMO_KNIGHT:
		case PROMO_CAPTURE_K:
			s += 'n';
			break;
		default:
			break;
	}

	return s;
}

Move move_from_uci(Board &board, const std::string &uci) {
	Move parsed = board.parse_uci_move(uci);
	if (parsed.from >= 0 && parsed.to >= 0) {
		return parsed;
	}

	std::vector<Move> legal = board.generate_moves();
	for (const Move &mv : legal) {
		if (move_to_string(mv) == uci) {
			return mv;
		}
	}

	return Move(-1, -1);
}

void apply_moves(Board &board, std::istringstream &iss) {
	std::string moveStr;
	while (iss >> moveStr) {
		Move mv = move_from_uci(board, moveStr);
		if (mv.from < 0) {
			std::cout << "info string invalid move " << moveStr << '\n';
			break;
		}
		board.make_move(mv);
	}
}

void handle_position(Board &board, std::istringstream &iss) {
	std::string token;
	if (!(iss >> token)) {
		return;
	}

	if (token == "startpos") {
		board = Board();
		if (!(iss >> token)) {
			return;
		}
	} else if (token == "fen") {
		// Skip the six FEN fields for now (Board() already builds startpos)
		int skipped = 0;
		while (skipped < 6 && (iss >> token)) {
			++skipped;
		}
		board = Board();
		if (!(iss >> token)) {
			return;
		}
	}

	if (token == "moves") {
		apply_moves(board, iss);
	}
}

GoParams parse_go(std::istringstream &iss) {
	GoParams params;
	std::string token;
	while (iss >> token) {
		if (token == "depth") {
			iss >> params.depth;
		} else if (token == "wtime") {
			iss >> params.wtime;
		} else if (token == "btime") {
			iss >> params.btime;
		} else if (token == "winc") {
			iss >> params.winc;
		} else if (token == "binc") {
			iss >> params.binc;
		} else if (token == "movestogo") {
			iss >> params.movestogo;
		} else if (token == "movetime") {
			iss >> params.movetime;
		} else if (token == "infinite") {
			params.infinite = true;
		} else {
			// Ignore other options (ponder, searchmoves, etc.) for now
		}
	}
	return params;
}

int choose_depth(const GoParams &params) {
	if (params.depth > 0) {
		return std::min(params.depth, kMaxDepthCap);
	}
	if (params.movetime > 0 || params.wtime > 0 || params.btime > 0) {
		// For timed games, let iterative deepening run deep; time limit will cut it off.
		return 32;
	}
	if (params.infinite) {
		return 32;
	}
	return kDefaultFixedDepth;
}

int choose_time_limit_ms(const GoParams &params, bool white_to_move) {
	if (params.movetime > 0) {
		// Fixed time per move: spend nearly everything, tiny buffer for overhead
		return std::max(100, params.movetime - 10);
	}

	if (params.wtime > 0 || params.btime > 0) {
		int ourTime = white_to_move ? params.wtime : params.btime;
		int ourInc = white_to_move ? params.winc : params.binc;
		if (ourTime <= 0) {
			return 500; // emergency
		}

		// Estimate moves remaining in game
		int moves = params.movestogo > 0 ? params.movestogo : 20;
		moves = std::max(1, moves);

		// Base budget: divide remaining time by expected moves, plus increment
		int budget = ourTime / moves + ourInc;

		// Use more time when we have plenty (scale up early/mid-game)
		if (ourTime > 60000) {
			// > 1 min left: can afford to think longer
			budget = std::max(budget, ourTime / 16);
		} else if (ourTime > 20000) {
			// 20s-60s left: moderate
			budget = std::max(budget, ourTime / 22);
		}
		// else: low on time, stick to basic budget

		// Safety margin: never use more than 80% of remaining time in one move
		int maxAllowed = ourTime * 9 / 10;

		// Also keep at least 1 second in reserve (or 5% of time, whichever is larger)
		int reserve = std::max(1000, ourTime / 20);
		maxAllowed = std::min(maxAllowed, ourTime - reserve);

		budget = std::min(budget, maxAllowed);
		budget = std::max(budget, 100); // at least 100ms

		// Cap at 9.5 seconds per move to stay under 10s timeout
		return std::min(budget, 9500);
	}

	if (params.infinite) {
		return 300000; // 5 minutes cap to avoid hangs
	}

	return 5000; // default 5s cap when no info provided
}

void handle_go(Board &board, std::istringstream &iss) {
	GoParams params = parse_go(iss);
	int depth = choose_depth(params);
	depth = std::max(1, depth);

	int time_limit = choose_time_limit_ms(params, board.team_white);
	time_limit_ms = std::min(time_limit, 9500);  // Never exceed 9.5s

	auto start = std::chrono::steady_clock::now();
	SearchResult result = find_best_move(board, depth);
	auto stop = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

	std::string bestMoveStr = move_to_string(result.bestMove);
	if (bestMoveStr.empty()) {
		auto legal = board.generate_moves();
		if (!legal.empty()) {
			bestMoveStr = move_to_string(legal.front());
		}
	}

	std::cout << "info depth " << depth
			  << " score cp " << result.score
			  << " time " << elapsed
			  << " tlim " << time_limit << '\n';

	if (bestMoveStr.empty()) {
		std::cout << "bestmove 0000" << std::endl;
	} else {
		std::cout << "bestmove " << bestMoveStr << std::endl;
	}
}

void uci_loop() {
	Board board;
	std::string line;

	while (std::getline(std::cin, line)) {
		if (line.empty()) {
			continue;
		}

		std::istringstream iss(line);
		std::string token;
		iss >> token;

		if (token == "uci") {
			std::cout << "id name Chesster" << '\n';
			std::cout << "id author Team Chesster" << '\n';
			std::cout << "uciok" << std::endl;
		} else if (token == "isready") {
			std::cout << "readyok" << std::endl;
		} else if (token == "ucinewgame") {
			board = Board();
			clear_transposition_table();
		} else if (token == "position") {
			handle_position(board, iss);
		} else if (token == "go") {
			handle_go(board, iss);
		} else if (token == "quit") {
			break;
		} else if (token == "stop") {
			// Search is synchronous, nothing to cancel
		} else if (token == "setoption") {
			// No configurable options yet
		}
	}
}

} // namespace

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::cout.setf(std::ios::unitbuf);

	clear_transposition_table();
	uci_loop();
	return 0;
}
