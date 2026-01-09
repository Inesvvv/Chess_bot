// Project: Chesster - Simple Chess Engine
// File: eval_tester.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-21
// Purpose: Test and compare evaluation functions (tapered vs legacy vs Stockfish)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <cstdlib>
#include <array>
#include <memory>
#include <climits>

#include "board.h"
#include "bitboards.h"
#include "magic.h"
#include "evaluation.h"


// ==================== FEN PARSING ====================

// Parse a FEN string and set up the board position
bool setup_from_fen(Board& board, const std::string& fen) {
    using namespace Bitboards;
    
    // Clear all bitboards
    white_pawns = 0ULL;
    white_knights = 0ULL;
    white_bishops = 0ULL;
    white_rooks = 0ULL;
    white_queens = 0ULL;
    white_kings = 0ULL;
    black_pawns = 0ULL;
    black_knights = 0ULL;
    black_bishops = 0ULL;
    black_rooks = 0ULL;
    black_queens = 0ULL;
    black_kings = 0ULL;
    
    std::istringstream iss(fen);
    std::string position, turn, castling, ep, halfmove, fullmove;
    
    iss >> position >> turn >> castling >> ep;
    // halfmove and fullmove are optional
    if (!(iss >> halfmove)) halfmove = "0";
    if (!(iss >> fullmove)) fullmove = "1";
    
    // Parse piece placement
    int rank = 7;
    int file = 0;
    
    for (char c : position) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += (c - '0');
        } else {
            int sq_idx = sq(file, rank);
            Bitboard sq_bit = bit(sq_idx);
            
            switch (c) {
                case 'P': white_pawns |= sq_bit; break;
                case 'N': white_knights |= sq_bit; break;
                case 'B': white_bishops |= sq_bit; break;
                case 'R': white_rooks |= sq_bit; break;
                case 'Q': white_queens |= sq_bit; break;
                case 'K': white_kings |= sq_bit; break;
                case 'p': black_pawns |= sq_bit; break;
                case 'n': black_knights |= sq_bit; break;
                case 'b': black_bishops |= sq_bit; break;
                case 'r': black_rooks |= sq_bit; break;
                case 'q': black_queens |= sq_bit; break;
                case 'k': black_kings |= sq_bit; break;
                default: return false;
            }
            file++;
        }
    }
    
    // Update combined bitboards
    all_white = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_kings;
    occupied = all_white | all_black;
    empty = ~occupied;
    
    // Parse turn
    board.team_white = (turn == "w");
    
    // Parse castling rights
    board.white_castle_kingside = (castling.find('K') != std::string::npos);
    board.white_castle_queenside = (castling.find('Q') != std::string::npos);
    board.black_castle_kingside = (castling.find('k') != std::string::npos);
    board.black_castle_queenside = (castling.find('q') != std::string::npos);
    
    // Parse en passant square
    if (ep == "-") {
        board.ep_square = -1;
    } else {
        int ep_file = ep[0] - 'a';
        int ep_rank = ep[1] - '1';
        board.ep_square = sq(ep_file, ep_rank);
    }
    
    // Parse halfmove clock
    board.halfmove_clock = std::stoi(halfmove);
    
    return true;
}

// Convert centipawns to pawns for display
double cp_to_pawns(int cp) {
    return cp / 100.0;
}

// ==================== STOCKFISH INTERFACE ====================

// Execute Stockfish and get evaluation for a position
// Returns evaluation in centipawns from white's perspective
// Returns INT_MIN if Stockfish is not available or fails
int get_stockfish_eval(const std::string& fen, const std::string& stockfish_path = "stockfish") {
    std::string command = "printf 'uci\\nisready\\nposition fen " + fen + "\\ngo depth 1\\nquit\\n' | " + stockfish_path;
    
    std::array<char, 128> buffer;
    std::string result;
    
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return INT_MIN;
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }
    
    int status = pclose(pipe);
    if (status != 0) {
        return INT_MIN;
    }
    
    // Parse the evaluation from Stockfish output
    // Look for "info depth X ... score cp Y" or "score mate Y"
    std::istringstream iss(result);
    std::string line;
    int last_cp = 0;
    bool found = false;
    
    while (std::getline(iss, line)) {
        if (line.find("info depth") != std::string::npos && 
            line.find("score cp") != std::string::npos) {
            size_t pos = line.find("score cp");
            if (pos != std::string::npos) {
                pos += 9; // Skip "score cp "
                std::istringstream score_stream(line.substr(pos));
                score_stream >> last_cp;
                found = true;
            }
        } else if (line.find("info depth") != std::string::npos && 
                   line.find("score mate") != std::string::npos) {
            size_t pos = line.find("score mate");
            if (pos != std::string::npos) {
                pos += 11; // Skip "score mate "
                int mate_in;
                std::istringstream score_stream(line.substr(pos));
                score_stream >> mate_in;
                // Convert mate to large centipawn value
                last_cp = (mate_in > 0 ? 20000 - mate_in * 100 : -20000 - mate_in * 100);
                found = true;
            }
        }
    }
    
    return found ? last_cp : INT_MIN;
}

// ==================== 50 TEST POSITIONS ====================

struct TestPosition {
    std::string fen;
    std::string description;
    int expected_sign;  // 1 = white better, -1 = black better, 0 = equal
};

std::vector<TestPosition> get_test_positions() {
    return {
        // ============ STARTING & OPENING POSITIONS (1-10) ============
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "1. Starting position", 0},
        {"rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
         "2. After 1.e4", 0},
        {"rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
         "3. After 1.e4 e5", 0},
        {"r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
         "4. After Nf3 Nc6", 0},
        {"r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
         "5. Italian Game", 0},
        {"r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
         "6. Ruy Lopez", 0},
        {"rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
         "7. Scandinavian Defense", 0},
        {"rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2",
         "8. Sicilian Defense", 0},
        {"rnbqkbnr/ppp1pppp/8/3p4/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
         "9. Queen's Gambit", 0},
        {"rnbqkb1r/pppppppp/5n2/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
         "10. Indian Defense", 0},
        
        // ============ MATERIAL IMBALANCES (11-20) ============
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKB1R w KQkq - 0 1",
         "11. White missing knight", -1},
        {"r1bqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "12. Black missing knight", 1},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq - 0 1",
         "13. White missing bishop", -1},
        {"rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "14. Black missing bishop", 1},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/1NBQKBNR w Kkq - 0 1",
         "15. White missing rook", -1},
        {"1nbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQk - 0 1",
         "16. Black missing rook", 1},
        {"rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "17. Black missing queen", 1},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1",
         "18. White missing queen", -1},
        {"rnbqkbnr/ppp1pppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
         "19. Black missing pawn", 1},
        {"rnbqkbnr/pppppppp/8/8/8/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1",
         "20. White missing pawn", -1},
        
        // ============ PAWN STRUCTURE (21-30) ============
        {"8/pppppppp/8/8/8/8/PPPPPPPP/8 w - - 0 1",
         "21. All pawns only", 0},
        {"8/ppp2ppp/8/8/8/8/PPP2PPP/8 w - - 0 1",
         "22. Symmetric pawn structure", 0},
        {"8/pppp1ppp/8/8/4P3/8/PPPP1PPP/8 w - - 0 1",
         "23. White isolated e-pawn", 0},
        {"8/pppppppp/8/8/3PP3/8/PPP2PPP/8 w - - 0 1",
         "24. White pawn duo", 1},
        {"8/ppp2ppp/3p4/8/3PP3/8/PPP2PPP/8 w - - 0 1",
         "25. Closed pawn center", 0},
        {"8/8/8/8/4P3/8/8/8 w - - 0 1",
         "26. White passed pawn e4", 1},
        {"8/8/4P3/8/8/8/8/8 w - - 0 1",
         "27. White passed pawn e6 (advanced)", 1},
        {"8/4P3/8/8/8/8/8/8 w - - 0 1",
         "28. White passed pawn e7 (promotion square)", 1},
        {"8/8/8/4p3/8/8/8/8 w - - 0 1",
         "29. Black passed pawn e5", -1},
        {"8/8/8/8/8/8/4p3/8 w - - 0 1",
         "30. Black passed pawn e2 (advanced)", -1},
        
        // ============ KING SAFETY (31-35) ============
        {"r3k2r/ppp2ppp/8/8/8/8/PPP2PPP/R3K2R w KQkq - 0 1",
         "31. Both can castle", 0},
        {"r3k2r/ppp2ppp/8/8/8/8/PPP2PPP/R4RK1 w kq - 0 1",
         "32. White castled kingside", 1},
        {"r4rk1/ppp2ppp/8/8/8/8/PPP2PPP/R3K2R w KQ - 0 1",
         "33. Black castled kingside", -1},
        {"r3k2r/pppq1ppp/8/8/8/8/PPPQ1PPP/R3K2R w KQkq - 0 1",
         "34. Queens on board, both uncastled", 0},
        {"6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1",
         "35. Kingside pawn shield both sides", 0},
        
        // ============ ENDGAME POSITIONS (36-45) ============
        {"8/8/4k3/8/8/4K3/4P3/8 w - - 0 1",
         "36. K+P vs K", 1},
        {"8/4p3/4k3/8/8/4K3/8/8 w - - 0 1",
         "37. K vs K+P", -1},
        {"8/8/4k3/8/8/4K3/8/8 w - - 0 1",
         "38. K vs K (draw)", 0},
        {"8/8/8/3k4/8/3K4/3P4/8 w - - 0 1",
         "39. K+P vs K (opposition)", 1},
        {"8/8/3k4/8/3P4/3K4/8/8 w - - 0 1",
         "40. K+P vs K (pawn blocked)", 1},
        {"8/8/8/8/8/k7/r7/K7 w - - 0 1",
         "41. K vs K+R (mate threat)", -1},
        {"8/8/8/4k3/8/4K3/4PP2/8 w - - 0 1",
         "42. K+2P vs K", 1},
        {"8/4pp2/4k3/8/8/4K3/8/8 w - - 0 1",
         "43. K vs K+2P", -1},
        {"8/8/4k3/4p3/4P3/4K3/8/8 w - - 0 1",
         "44. K+P vs K+P (blocked pawns)", 0},
        {"4k3/8/8/8/8/8/4R3/4K3 w - - 0 1",
         "45. K+R vs K", 1},
        
        // ============ COMPLEX MIDDLEGAME (46-50) ============
        {"r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 8",
         "46. Typical Italian middlegame", 0},
        {"r2q1rk1/ppp2ppp/2npbn2/2b1p3/2B1P3/2NP1N2/PPPQ1PPP/R1B2RK1 w - - 0 10",
         "47. Middlegame with queens", 0},
        {"r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4",
         "48. Scholar's mate threat", 1},
        {"r1bqr1k1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NPBN2/PPP2PPP/R2Q1RK1 w - - 0 9",
         "49. Closed position", 0},
        {"r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQK2R w KQkq - 0 5",
         "50. Open position", 0},
    };
}

// ==================== 3-WAY EVALUATION COMPARISON ====================

struct EvalResult {
    std::string fen;
    std::string description;
    int tapered_eval;        // Tapered evaluation (centipawns)
    int legacy_eval;         // Legacy evaluation (centipawns)
    int stockfish_eval;      // Stockfish evaluation (centipawns)
    int expected_sign;
    bool tapered_sign_ok;    // Tapered sign matches Stockfish
    bool legacy_sign_ok;     // Legacy sign matches Stockfish
    int tapered_diff;        // |tapered - stockfish|
    int legacy_diff;         // |legacy - stockfish|
};

EvalResult evaluate_position(const TestPosition& pos, const std::string& stockfish_path) {
    EvalResult result;
    result.fen = pos.fen;
    result.description = pos.description;
    result.expected_sign = pos.expected_sign;
    
    // Initialize board and set up position
    Board board;
    Magic::init_magics();
    Bitboards::init_attack_tables();
    Bitboards::init_masks();
    
    if (!setup_from_fen(board, pos.fen)) {
        result.tapered_eval = 0;
        result.legacy_eval = 0;
        result.stockfish_eval = 0;
        result.tapered_sign_ok = false;
        result.legacy_sign_ok = false;
        result.tapered_diff = 0;
        result.legacy_diff = 0;
        return result;
    }
    
    // Get tapered evaluation (from white's perspective)
    result.tapered_eval = Evaluation::evaluate_tapered(
        Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
        Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
        Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
        Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
        Bitboards::occupied, true);
    
    // Get legacy evaluation (from white's perspective)
    result.legacy_eval = Evaluation::evaluate(
        Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
        Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
        Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
        Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
        Bitboards::occupied, true);
    
    // Get Stockfish evaluation
    result.stockfish_eval = get_stockfish_eval(pos.fen, stockfish_path);
    
    // Compare signs (use 20cp threshold for "equal")
    auto get_sign = [](int eval) -> int {
        if (eval > 20) return 1;
        if (eval < -20) return -1;
        return 0;
    };
    
    int tapered_sign = get_sign(result.tapered_eval);
    int legacy_sign = get_sign(result.legacy_eval);
    int sf_sign = get_sign(result.stockfish_eval);
    
    if (result.stockfish_eval != INT_MIN) {
        result.tapered_sign_ok = (tapered_sign == sf_sign);
        result.legacy_sign_ok = (legacy_sign == sf_sign);
        result.tapered_diff = std::abs(result.tapered_eval - result.stockfish_eval);
        result.legacy_diff = std::abs(result.legacy_eval - result.stockfish_eval);
    } else {
        result.tapered_sign_ok = true;  // Can't compare
        result.legacy_sign_ok = true;
        result.tapered_diff = 0;
        result.legacy_diff = 0;
    }
    
    return result;
}

// ==================== REPORTING ====================

void print_result(const EvalResult& result, bool verbose) {
    // Truncate description to 30 chars
    std::string desc = result.description.substr(0, 30);
    
    std::cout << std::left << std::setw(32) << desc;
    
    // Tapered evaluation
    std::cout << " | " << std::right << std::setw(6) << std::fixed 
              << std::setprecision(2) << cp_to_pawns(result.tapered_eval);
    
    // Legacy evaluation
    std::cout << " | " << std::setw(6) << std::fixed 
              << std::setprecision(2) << cp_to_pawns(result.legacy_eval);
    
    // Stockfish evaluation
    if (result.stockfish_eval != INT_MIN) {
        std::cout << " | " << std::setw(6) << std::fixed 
                  << std::setprecision(2) << cp_to_pawns(result.stockfish_eval);
        
        // Tapered vs SF difference
        std::cout << " | " << std::setw(5) << std::fixed 
                  << std::setprecision(2) << cp_to_pawns(result.tapered_diff);
        
        // Legacy vs SF difference
        std::cout << " | " << std::setw(5) << std::fixed 
                  << std::setprecision(2) << cp_to_pawns(result.legacy_diff);
        
        // Which is closer?
        if (result.tapered_diff < result.legacy_diff) {
            std::cout << " | TAP";
        } else if (result.legacy_diff < result.tapered_diff) {
            std::cout << " | LEG";
        } else {
            std::cout << " | TIE";
        }
    } else {
        std::cout << " |    N/A |   N/A |   N/A |  -";
    }
    
    std::cout << "\n";
    
    if (verbose) {
        std::cout << "  FEN: " << result.fen << "\n";
    }
}

void print_summary(const std::vector<EvalResult>& results) {
    int total = results.size();
    int sf_available = 0;
    int tapered_sign_ok = 0;
    int legacy_sign_ok = 0;
    int tapered_wins = 0;
    int legacy_wins = 0;
    int ties = 0;
    double total_tapered_diff = 0;
    double total_legacy_diff = 0;
    double max_tapered_diff = 0;
    double max_legacy_diff = 0;
    
    for (const auto& r : results) {
        if (r.stockfish_eval != INT_MIN) {
            sf_available++;
            if (r.tapered_sign_ok) tapered_sign_ok++;
            if (r.legacy_sign_ok) legacy_sign_ok++;
            
            total_tapered_diff += r.tapered_diff;
            total_legacy_diff += r.legacy_diff;
            
            if (r.tapered_diff > max_tapered_diff) max_tapered_diff = r.tapered_diff;
            if (r.legacy_diff > max_legacy_diff) max_legacy_diff = r.legacy_diff;
            
            if (r.tapered_diff < r.legacy_diff) {
                tapered_wins++;
            } else if (r.legacy_diff < r.tapered_diff) {
                legacy_wins++;
            } else {
                ties++;
            }
        }
    }
    
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                           SUMMARY                                  ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
    std::cout << "║ Total positions tested:        " << std::setw(3) << total << "                                 ║\n";
    std::cout << "║ Stockfish comparisons:         " << std::setw(3) << sf_available << "                                 ║\n";
    std::cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
    
    if (sf_available > 0) {
        std::cout << "║                        TAPERED         LEGACY                     ║\n";
        std::cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ Sign agreement with SF:   " 
                  << std::setw(3) << tapered_sign_ok << "/" << std::setw(2) << sf_available 
                  << " (" << std::setw(5) << std::fixed << std::setprecision(1) 
                  << (100.0 * tapered_sign_ok / sf_available) << "%)   "
                  << std::setw(3) << legacy_sign_ok << "/" << std::setw(2) << sf_available 
                  << " (" << std::setw(5) << std::fixed << std::setprecision(1) 
                  << (100.0 * legacy_sign_ok / sf_available) << "%)     ║\n";
        
        std::cout << "║ Average difference (pawns):   " 
                  << std::setw(5) << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(total_tapered_diff / sf_available) << "         "
                  << std::setw(5) << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(total_legacy_diff / sf_available) << "            ║\n";
        
        std::cout << "║ Maximum difference (pawns):   " 
                  << std::setw(5) << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(max_tapered_diff) << "         "
                  << std::setw(5) << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(max_legacy_diff) << "            ║\n";
        
        std::cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
        std::cout << "║ CLOSER TO STOCKFISH:                                              ║\n";
        std::cout << "║   Tapered wins: " << std::setw(3) << tapered_wins 
                  << "    Legacy wins: " << std::setw(3) << legacy_wins 
                  << "    Ties: " << std::setw(3) << ties << "              ║\n";
        
        // Declare winner
        std::cout << "╠═══════════════════════════════════════════════════════════════════╣\n";
        if (tapered_wins > legacy_wins) {
            std::cout << "║ >>> TAPERED EVALUATION PERFORMS BETTER! <<<                       ║\n";
        } else if (legacy_wins > tapered_wins) {
            std::cout << "║ >>> LEGACY EVALUATION PERFORMS BETTER! <<<                        ║\n";
        } else {
            std::cout << "║ >>> BOTH EVALUATIONS PERFORM SIMILARLY <<<                        ║\n";
        }
    }
    
    std::cout << "╚═══════════════════════════════════════════════════════════════════╝\n";
}

// ==================== PHASE TESTING ====================

void test_phase_calculation() {
    std::cout << "\n==================== PHASE TESTING ====================\n";
    
    struct PhaseTest {
        std::string fen;
        std::string description;
    };
    
    std::vector<PhaseTest> phase_tests = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Starting position (full middlegame)"},
        {"r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4", "After some development"},
        {"r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 8", "Typical middlegame"},
        {"r4rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R4RK1 w - - 0 12", "Some pieces traded"},
        {"4rrk1/ppp2ppp/2np4/2b1p3/2B1P3/2NP4/PPP2PPP/4RRK1 w - - 0 16", "Into endgame"},
        {"8/ppp2ppp/8/4p3/4P3/8/PPP2PPP/8 w - - 0 1", "Pawn endgame"},
        {"8/8/4k3/8/8/4K3/4P3/8 w - - 0 1", "K+P vs K"},
    };
    
    Magic::init_magics();
    Bitboards::init_attack_tables();
    Bitboards::init_masks();
    Board board;
    
    std::cout << std::left << std::setw(40) << "Position" 
              << " | Phase (0-24) | MG%  | EG%\n";
    std::cout << std::string(70, '-') << "\n";
    
    for (const auto& test : phase_tests) {
        setup_from_fen(board, test.fen);
        
        int phase = Evaluation::calculate_phase(
            Bitboards::white_knights, Bitboards::white_bishops,
            Bitboards::white_rooks, Bitboards::white_queens,
            Bitboards::black_knights, Bitboards::black_bishops,
            Bitboards::black_rooks, Bitboards::black_queens);
        
        double mg_pct = (100.0 * phase) / Evaluation::TOTAL_PHASE;
        double eg_pct = 100.0 - mg_pct;
        
        std::cout << std::left << std::setw(40) << test.description
                  << " |     " << std::right << std::setw(2) << phase 
                  << "       | " << std::setw(5) << std::fixed << std::setprecision(1) << mg_pct
                  << " | " << std::setw(5) << eg_pct << "\n";
    }
    
    std::cout << "======================================================\n";
}

// ==================== COMPARE TAPERED VS LEGACY ====================

void compare_tapered_vs_legacy() {
    std::cout << "\n============= TAPERED VS LEGACY COMPARISON =============\n";
    
    auto positions = get_test_positions();
    
    Magic::init_magics();
    Bitboards::init_attack_tables();
    Bitboards::init_masks();
    Board board;
    
    std::cout << std::left << std::setw(32) << "Position" 
              << " | Tapered | Legacy  | Diff\n";
    std::cout << std::string(60, '-') << "\n";
    
    for (const auto& pos : positions) {
        setup_from_fen(board, pos.fen);
        
        int tapered = Evaluation::evaluate_tapered(
            Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
            Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
            Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
            Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
            Bitboards::occupied, true);
        
        int legacy = Evaluation::evaluate(
            Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
            Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
            Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
            Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
            Bitboards::occupied, true);
        
        int diff = tapered - legacy;
        
        // Truncate description
        std::string desc = pos.description.substr(0, 30);
        
        std::cout << std::left << std::setw(32) << desc
                  << " | " << std::right << std::setw(7) << std::fixed 
                  << std::setprecision(2) << cp_to_pawns(tapered)
                  << " | " << std::setw(7) << cp_to_pawns(legacy)
                  << " | " << std::setw(6) << cp_to_pawns(diff) << "\n";
    }
    
    std::cout << "========================================================\n";
}

// ==================== SINGLE FEN EVALUATION ====================

void evaluate_single_fen(const std::string& fen, const std::string& stockfish_path, bool verbose) {
    Board board;
    Magic::init_magics();
    Bitboards::init_attack_tables();
    Bitboards::init_masks();
    
    if (!setup_from_fen(board, fen)) {
        std::cerr << "Error: Invalid FEN string\n";
        return;
    }
    
    std::cout << "\n==================== SINGLE POSITION EVALUATION ====================\n";
    std::cout << "FEN: " << fen << "\n\n";
    
    // Calculate phase
    int phase = Evaluation::calculate_phase(
        Bitboards::white_knights, Bitboards::white_bishops,
        Bitboards::white_rooks, Bitboards::white_queens,
        Bitboards::black_knights, Bitboards::black_bishops,
        Bitboards::black_rooks, Bitboards::black_queens);
    
    double mg_pct = (100.0 * phase) / Evaluation::TOTAL_PHASE;
    
    std::cout << "Game Phase: " << phase << "/" << Evaluation::TOTAL_PHASE 
              << " (" << std::fixed << std::setprecision(1) << mg_pct << "% middlegame)\n\n";
    
    // Tapered evaluation
    int tapered = Evaluation::evaluate_tapered(
        Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
        Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
        Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
        Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
        Bitboards::occupied, board.team_white);
    
    // Legacy evaluation
    int legacy = Evaluation::evaluate(
        Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
        Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
        Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
        Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings,
        Bitboards::occupied, board.team_white);
    
    std::cout << "Chesster Tapered Eval: " << std::fixed << std::setprecision(2) 
              << cp_to_pawns(tapered) << " pawns (" << tapered << " cp)\n";
    std::cout << "Chesster Legacy Eval:  " << std::fixed << std::setprecision(2) 
              << cp_to_pawns(legacy) << " pawns (" << legacy << " cp)\n";
    
    // Try Stockfish
    int sf_eval = get_stockfish_eval(fen, stockfish_path);
    if (sf_eval != INT_MIN) {
        std::cout << "Stockfish Eval:        " << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(sf_eval) << " pawns (" << sf_eval << " cp)\n";
        std::cout << "\nDifference (Tapered vs SF): " << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(std::abs(tapered - sf_eval)) << " pawns\n";
        std::cout << "Difference (Legacy vs SF):  " << std::fixed << std::setprecision(2) 
                  << cp_to_pawns(std::abs(legacy - sf_eval)) << " pawns\n";
    } else {
        std::cout << "Stockfish: Not available\n";
    }
    
    if (verbose) {
        // Print detailed breakdown
        std::cout << "\n--- Evaluation Breakdown ---\n";
        
        auto mat_score = Evaluation::evaluate_material_tapered(
            Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
            Bitboards::white_rooks, Bitboards::white_queens,
            Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
            Bitboards::black_rooks, Bitboards::black_queens);
        
        auto pst_score = Evaluation::evaluate_pst_tapered(
            Bitboards::white_pawns, Bitboards::white_knights, Bitboards::white_bishops,
            Bitboards::white_rooks, Bitboards::white_queens, Bitboards::white_kings,
            Bitboards::black_pawns, Bitboards::black_knights, Bitboards::black_bishops,
            Bitboards::black_rooks, Bitboards::black_queens, Bitboards::black_kings);
        
        auto pawn_score = Evaluation::evaluate_pawn_structure_tapered(
            Bitboards::white_pawns, Bitboards::black_pawns, Bitboards::occupied);
        
        std::cout << "Material (MG/EG):    " << mat_score.mg << " / " << mat_score.eg 
                  << " -> " << mat_score.interpolate(phase) << "\n";
        std::cout << "PST (MG/EG):         " << pst_score.mg << " / " << pst_score.eg 
                  << " -> " << pst_score.interpolate(phase) << "\n";
        std::cout << "Pawn Structure (MG/EG): " << pawn_score.mg << " / " << pawn_score.eg 
                  << " -> " << pawn_score.interpolate(phase) << "\n";
    }
    
    std::cout << "====================================================================\n";
}

// ==================== RUN 50 POSITION COMPARISON ====================

void run_50_position_comparison(const std::string& stockfish_path, bool verbose) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║           50-POSITION EVALUATION COMPARISON: TAPERED vs LEGACY vs SF          ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════════════╝\n\n";
    
    std::cout << "Using Stockfish at: " << stockfish_path << "\n";
    std::cout << "(Set with --stockfish PATH if not found)\n\n";
    
    Magic::init_magics();
    Bitboards::init_attack_tables();
    Bitboards::init_masks();
    
    auto positions = get_test_positions();
    std::vector<EvalResult> results;
    
    std::cout << std::left << std::setw(32) << "Position" 
              << " | Tapr  | Legcy |   SF  | T-SF | L-SF | Best\n";
    std::cout << std::string(82, '-') << "\n";
    
    for (const auto& pos : positions) {
        EvalResult result = evaluate_position(pos, stockfish_path);
        results.push_back(result);
        print_result(result, verbose);
    }
    
    print_summary(results);
}

// ==================== MAIN ====================

void print_usage(const char* program) {
    std::cout << "Usage: " << program << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  --stockfish PATH    Path to Stockfish executable (default: stockfish)\n"
              << "  --fen \"FEN\"         Evaluate a single FEN position\n"
              << "  --verbose           Show FEN strings and detailed breakdown\n"
              << "  --phase             Test phase calculation\n"
              << "  --compare           Compare tapered vs legacy evaluation (no SF)\n"
              << "  --50                Run 50-position comparison (tapered vs legacy vs SF)\n"
              << "  --all               Run all tests including 50-position comparison\n"
              << "  --help              Show this help message\n\n"
              << "Examples:\n"
              << "  " << program << " --50                                 # Run 50-position comparison\n"
              << "  " << program << " --50 --stockfish /usr/bin/stockfish  # With custom SF path\n"
              << "  " << program << " --compare                            # Quick tapered vs legacy\n"
              << "  " << program << " --phase                              # Test phase calculation\n"
              << "  " << program << " --fen \"8/8/4k3/8/8/4K3/4P3/8 w - - 0 1\" --verbose\n";
}

int main(int argc, char* argv[]) {
    std::string stockfish_path = "stockfish";
    std::string fen;
    bool verbose = false;
    bool run_phase = false;
    bool run_compare = false;
    bool run_50 = false;
    bool run_all = false;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--stockfish" && i + 1 < argc) {
            stockfish_path = argv[++i];
        } else if (arg == "--fen" && i + 1 < argc) {
            fen = argv[++i];
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--phase") {
            run_phase = true;
        } else if (arg == "--compare") {
            run_compare = true;
        } else if (arg == "--50") {
            run_50 = true;
        } else if (arg == "--all") {
            run_all = true;
        }
    }
    
    // If no options specified, show help
    if (!run_phase && !run_compare && !run_50 && !run_all && fen.empty()) {
        std::cout << "Chesster Evaluation Tester\n";
        std::cout << "==========================\n\n";
        print_usage(argv[0]);
        return 0;
    }
    
    // Evaluate single FEN
    if (!fen.empty()) {
        evaluate_single_fen(fen, stockfish_path, verbose);
        return 0;
    }
    
    // Run phase testing
    if (run_phase || run_all) {
        test_phase_calculation();
    }
    
    // Run tapered vs legacy comparison (no Stockfish)
    if (run_compare || run_all) {
        compare_tapered_vs_legacy();
    }
    
    // Run 50-position comparison
    if (run_50 || run_all) {
        run_50_position_comparison(stockfish_path, verbose);
    }
    
    return 0;
}