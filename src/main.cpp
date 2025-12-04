// Project: Chesster - Simple Chess Engine
// File: main.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

#include "board.h"
#include "perft.cpp"
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

// go from the string representing the move to an actual move
// Move string_to_move(const std::string &line)
// {
//     // check if the line is valid (it had at least 4 characters)
//     if (line.size() < 4)
//     {
//         return Move{-1, -1};
//     }

//     // convert from string to index
//     auto parse_sq = [](char fileChar, char rankChar) -> int
//     {
//         int file = fileChar - 'a';
//         int rank = rankChar - '1';
//         if (file < 0 || file > 7 || rank < 0 || rank > 7)
//         {
//             return -1;
//         }
//         return rank * 8 + file;
//     };

//     int from = parse_sq(line[0], line[1]);
//     int to = parse_sq(line[2], line[3]);

//     // return a move
//     return Move{from, to};
// }

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

    // store who's turn it is to move according to the number of previous moves
    //(even is white, odd is black)
    board.team_white = (history_moves.size() % 2 == 0);

    // for testing purposes: print the board after the history moves have been played
    std::cout << "Position after history:\n";
    board.print_board();

    // Choose the next move
    // FOR NOW WE ARE IMPLEMENTING THE NEXT MOVE TO BE A RANDOM MOVE
    
    
    // Move next = board.get_random_move();
    // std::string next_move_str = move_to_string(next);
    // std::cout << "Next move: " << next_move_str << '\n';

    // if (next_move_str.empty())
    // {
    //     std::cerr << "Did not find any legal moves.\n";
    //     return 1;
    // }

    //make move using search and evaluation
    int searchDepth = 4;
    SearchResult result = find_best_move(board, searchDepth);
    Move next = result.bestMove;
    std::string next_move_str = move_to_string(next);

    std::cout << "Next move: " << next_move_str
            << " (score: " << result.score << ")\n";

    if (next_move_str.empty())
    {
        std::cerr << "Did not find any legal moves.\n";
        return 1;
    }

    // Write to the output file
    std::ofstream out(output_file);
    if (!out)
    {
        std::cerr << "ERROR: could not open output file '"
                  << output_file << "' for writing\n";
        return 1;
    }
    // print the next move into the file
    out << next_move_str << '\n';

    // ======== TESTING PERFT ===========
    std::cout << "════════════════════════════════════════════\n";
    std::cout << "        Chesster — PERFT Test Suite\n";
    std::cout << "════════════════════════════════════════════\n\n";

    RUN_TEST(perft_depth1);
    RUN_TEST(perft_depth2);
    RUN_TEST(perft_depth3);
    RUN_TEST(perft_depth4);

    // Debug: print per-root move breakdown for depth 3 to investigate failures
    std::cout << "\nPerft split (depth 3):\n";
    perft_split(3);
    std::cout << "\nPerft split (depth 2):\n";
    perft_split(2);
    std::cout << "\nPerft split (depth 4):\n";
    perft_split(4);

    std::cout << "\n════════════════════════════════════════════\n";
    std::cout << " Test Summary\n";
    std::cout << "════════════════════════════════════════════\n";
    std::cout << " Passed: " << tests_passed << "\n";
    std::cout << " Failed: " << tests_failed << "\n";
    std::cout << "════════════════════════════════════════════\n";

    return tests_failed == 0 ? 0 : 1;

    //return 0;
}
