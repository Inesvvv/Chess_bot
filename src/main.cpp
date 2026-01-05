// Project: Chesster - Simple Chess Engine
// File: main.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>

#include "board.h"
#include "opening_book.h"
#include "search.h"

// HELPER FUNCTIONS TO PASS FROM FILE TO MOVE

// helper to convert from index to position notation depending on the file and rank
std::string index_to_square(int sq)
{
    int file = sq % 8;
    int rank = sq / 8;
    char fileChar = static_cast<char>('a' + file);
    char rankChar = static_cast<char>('1' + rank);
    return std::string() + fileChar + rankChar;
}

// helper function to go from a move to a string representing the move
std::string move_to_string(const Move &m)
{
    if (m.from < 0 || m.to < 0)
    {
        return "";
    }

    std::string s = index_to_square(m.from) + index_to_square(m.to);

    // Append promotion piece for promotions
    switch (m.flag)
    {
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

int main(int argc, char *argv[])
{
    std::string history_file = ""; // file containing all previous moves
    std::string output_file = "";  // file where we will write out next move

    // loop through all arguments passed to program sand store file that comes after "-H" and "-m"
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-H" && i + 1 < argc)
        {
            history_file = argv[++i];
        }
        else if (arg == "-m" && i + 1 < argc)
        {
            output_file = argv[++i];
        }
    }

    // Only needed for the random move generation
    // Use the current time as the seed, so that each run of the program gives a different sequence of random moves
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Create a board in the starting position
    Board board;

    // store all the previous moves in history_moves
    std::vector<Move> history_moves;

    // Read the history file and store all previous moves
    if (!history_file.empty())
    {
        std::ifstream in(history_file);
        if (!in)
        {
            std::cerr << "ERROR: can't open history file '"
                      << history_file << "'\n";
            return 1;
        }

        std::string line;
        while (std::getline(in, line))
        {
            if (line.empty())
            {
                continue;
            }
            // parse each line into a move, store it, and make the actual move onto the board
            Move mv = board.parse_uci_move(line);
            if (mv.from >= 0 && mv.to >= 0)
            {
                board.make_move(mv);
                history_moves.push_back(mv);
            }
        }
    }

    board.team_white = (history_moves.size() % 2 == 0);

    // for testing purposes: print the board after the history moves have been played
    std::cout << "Position after history:\n";
    board.print_board();

    // ========= OPENING BOOK =========

    // Try to load and use the opening book
    Move next = Move(-1, -1, QUIET);
    std::string next_move_str = "";

    // Load the custom opening book (generated with matching zobrist hashing from search.cpp)
    if (g_opening_book.load_book("../books/custom.bin"))
    {
        if (try_opening_book_move(board, next))
        {
            next_move_str = move_to_string(next);
            std::cout << "Using opening book move: " << next_move_str << "\n";
        }
    }

    // Opening book options (in the books folder):
    // custom.bin - tester opening book to verify functionality
    // Human.bin - Balanced, human-like openings (incompatible hash)
    // komodo.bin - Strong computer engine knowledge (incompatible hash)
    // baron30.bin - Alternative high-quality book (incompatible hash)
    // gm2001.bin - Grandmaster games (incompatible hash)
    // rodent.bin - Engine book (incompatible hash)

    // If no opening book move found, use search
    if (next_move_str.empty())
    {
        std::cout << "Position not in opening book, using search...\n";
        int searchDepth = 5; // Search 10 plies (5 full moves) deep
        SearchResult result = find_best_move(board, searchDepth);
        next = result.bestMove;
        next_move_str = move_to_string(next);

        std::cout << "Next move: " << next_move_str
                  << " (score: " << result.score << ")\n";
    }

    if (next_move_str.empty())
    {
        std::cerr << "Did not find any legal moves.\n";
        return 1;
    }

    // Write to the output file (only if one was specified)
    if (!output_file.empty())
    {
        std::ofstream out(output_file);
        if (!out)
        {
            std::cerr << "ERROR: could not open output file '"
                      << output_file << "' for writing\n";
            return 1;
        }
        // print the next move into the file
        out << next_move_str << '\n';
    }
    return 0;
}
