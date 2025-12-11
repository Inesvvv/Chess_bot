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
//#include "perft.cpp"
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
    
    //Perft split runner: ./chesster split 4
    // if (argc >= 3 && std::string(argv[1]) == "split")
    // {
    //     int depth = std::stoi(argv[2]);
    //     std::cout << "Perft split for depth " << depth << ":\n";
    //     perft_split(depth);
    //     return 0;
    // }

    //     // Debug path runner: ./chesster debugpath d2d3,e7e5,c1g5
    //     if (argc >= 3 && std::string(argv[1]) == "debugpath")
    //     {
    //         std::string moves_arg = argv[2];
    //         std::vector<std::string> moves;
    //         size_t pos = 0;
    //         while (pos < moves_arg.size()) {
    //             size_t comma = moves_arg.find(',', pos);
    //             if (comma == std::string::npos) comma = moves_arg.size();
    //             moves.push_back(moves_arg.substr(pos, comma - pos));
    //             pos = comma + 1;
    //         }
    //         Board dbg;
    //         std::cout << "Applying debug path:";
    //         for (auto &m : moves) std::cout << " " << m;
    //         std::cout << "\n";
    //         for (auto &u : moves)
    //         {
    //             Move mv = dbg.parse_uci_move(u);
    //             if (mv.from < 0) { std::cerr << "Invalid move in debug path: " << u << "\n"; return 1; }
    //             std::cout << "Applying: " << u << " flag=" << int(mv.flag) << "\n";
    //             dbg.make_move(mv);
    //         }
    //         dbg.print_board();
    //     std::cout << "ep_square = " << dbg.ep_square << "\n";
    //         std::cout << "In check: " << (dbg.in_check() ? "YES" : "NO") << "\n";
    //         auto legal = dbg.generate_moves();
    //         std::cout << "Legal moves count: " << legal.size() << "\n";
    //         for (const Move &mv : legal)
    //         {
    //             std::cout << mv.from << "->" << mv.to << " flag=" << int(mv.flag) << "\n";
    //         }
    //         return 0;
    //     }

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

    // // ======== TESTING PERFT ===========
    // std::cout << "════════════════════════════════════════════\n";
    // std::cout << "        Chesster — PERFT Test Suite\n";
    // std::cout << "════════════════════════════════════════════\n\n";

    // RUN_TEST(perft_depth1);
    // RUN_TEST(perft_depth2);
    // RUN_TEST(perft_depth3);
    // RUN_TEST(perft_depth4);
    // RUN_TEST(perft_depth5);
    // RUN_TEST(perft_depth6);
    // //RUN_TEST(perft_depth7);  // DO NOT UNCOMMENT - TEST 7 passes but its way too long so we still have optimization to do 

    // // ======== TESTING FEATURES ===========
    // std::cout << "\n════════════════════════════════════════════\n";
    // std::cout << "     Feature Validation Matrix\n";
    // std::cout << "════════════════════════════════════════════\n\n";
    
    // struct FeatureTest { std::string name; void (*test)(); };
    // FeatureTest features[] = {
    //     {"Pawn moves", test_feature_pawn_moves},
    //     {"Pawn double push", test_feature_pawn_double_push},
    //     {"Pawn captures", test_feature_pawn_captures},
    //     {"En passant", test_feature_en_passant},
    //     {"Promotions", test_feature_promotions},
    //     {"Knight moves", test_feature_knight_moves},
    //     {"Bishop moves", test_feature_bishop_moves},
    //     {"Rook moves", test_feature_rook_moves},
    //     {"Queen moves", test_feature_queen_moves},
    //     {"King moves", test_feature_king_moves},
    //     {"Castling", test_feature_castling},
    //     {"Check detection", test_feature_check_detection},
    //     {"Pin detection", test_feature_pin_detection},
    //     {"Legal move filtering", test_feature_legal_move_filtering},
    //     {"Board state tracking", test_feature_board_state_tracking},
    //     {"Move parsing", test_feature_move_parsing}
    // };
    
    // // Display header
    // std::cout << "| Feature                  | Status     |\n";
    // std::cout << "|--------------------------|------------|\n";
    
    // int feature_passed = 0;
    // int feature_total = sizeof(features) / sizeof(features[0]);
    
    // for (const auto &f : features) {
    //     try {
    //         f.test();
    //         std::cout << "| " << std::setw(24) << std::left << f.name << " | ✓ PASS     |\n";
    //         feature_passed++;
    //     } catch (const std::exception &e) {
    //         std::cout << "| " << std::setw(24) << std::left << f.name << " | ✗ FAIL     |\n";
    //     }
    // }
    
    // std::cout << "|--------------------------|------------|\n";

    // std::cout << "\n════════════════════════════════════════════\n";
    // std::cout << " Test Summary\n";
    // std::cout << "════════════════════════════════════════════\n";
    // std::cout << " PERFT Tests Passed: " << tests_passed << "/4\n";
    // std::cout << " Feature Tests Passed: " << feature_passed << "/" << feature_total << "\n";
    // std::cout << " Total Passed: " << (tests_passed + feature_passed) << "/" << (4 + feature_total) << "\n";
    // std::cout << "════════════════════════════════════════════\n";

    // return (tests_failed == 0 && feature_passed == feature_total) ? 0 : 1;

    return 0;
}
