// Testing file for perft testing - now have up to depth 6 testing but not quick enough yet 

#include <iostream>
#include <vector>
#include <stdexcept>
#include <map>
#include "board.h"
#include "move.h"
#include "bitboards.h"
#include "magic.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    try { \
        std::cout << "Running " << #name << "... "; \
        test_##name(); \
        std::cout << "✓ PASS\n"; \
        tests_passed++; \
    } catch (const std::exception &e) { \
        std::cout << "✗ FAIL: " << e.what() << "\n"; \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) throw std::runtime_error("Assertion failed: " #a " != " #b);

// =============================================================
//                      PERFT IMPLEMENTATION
// =============================================================

// One recursive perft step
// Global counters for perft statistics
static uint64_t perft_captures = 0;
static uint64_t perft_checks = 0;
static uint64_t perft_checkmates = 0;
static uint64_t perft_en_passants = 0;
static uint64_t perft_castles = 0;
static uint64_t perft_promotions = 0;

uint64_t perft(Board &b, int depth) {
    if (depth == 0)
        return 1;

    uint64_t nodes = 0;
    std::vector<Move> moves = b.generate_moves();

    for (const Move &m : moves) {
        b.make_move(m);
        nodes += perft(b, depth - 1);
        b.unmake_move(m);
    }
    return nodes;
}

// Perft with detailed statistics
uint64_t perft_detailed(Board &b, int depth) {
    if (depth == 0)
        return 1;

    uint64_t nodes = 0;
    std::vector<Move> moves = b.generate_moves();
    
    // At depth 1, count the move types
    bool is_leaf = (depth == 1);

    for (const Move &m : moves) {
        Bitboards::BoardState bb_snap = Bitboards::snapshot_state();
        Board copy = b;
        copy.make_move(m);
        
        if (is_leaf) {
            // Count captures
            if (m.flag == CAPTURE || m.flag == EN_PASSANT ||
                m.flag == PROMO_CAPTURE_Q || m.flag == PROMO_CAPTURE_R ||
                m.flag == PROMO_CAPTURE_B || m.flag == PROMO_CAPTURE_K) {
                perft_captures++;
            }
            
            // Count en passants
            if (m.flag == EN_PASSANT) {
                perft_en_passants++;
            }
            
            // Count castles
            if (m.flag == CASTLE_KINGSIDE || m.flag == CASTLE_QUEENSIDE) {
                perft_castles++;
            }
            
            // Count promotions
            if (m.flag == PROMO_QUEEN || m.flag == PROMO_ROOK ||
                m.flag == PROMO_BISHOP || m.flag == PROMO_KNIGHT ||
                m.flag == PROMO_CAPTURE_Q || m.flag == PROMO_CAPTURE_R ||
                m.flag == PROMO_CAPTURE_B || m.flag == PROMO_CAPTURE_K) {
                perft_promotions++;
            }
            
            // Count checks
            if (copy.in_check()) {
                perft_checks++;
            }
            
            // Count checkmates (in check and no legal moves)
            if (copy.in_check()) {
                std::vector<Move> opp_moves = copy.generate_moves();
                if (opp_moves.empty()) {
                    perft_checkmates++;
                }
            }
        }
        
        nodes += perft_detailed(copy, depth - 1);
        Bitboards::restore_state(bb_snap);
    }
    return nodes;
}

// Helper: perft with statistics from starting position
void perft_stats(int depth) {
    // Reset counters
    perft_captures = 0;
    perft_checks = 0;
    perft_checkmates = 0;
    perft_en_passants = 0;
    perft_castles = 0;
    perft_promotions = 0;
    
    Board b;
    uint64_t detailed_nodes = perft_detailed(b, depth);
    
    // Also run regular perft to compare
    Board b2;
    uint64_t regular_nodes = perft(b2, depth);
    
    std::cout << "\n=== Perft Statistics at Depth " << depth << " ===\n";
    std::cout << "Nodes (detailed): " << detailed_nodes << "\n";
    std::cout << "Nodes (regular):  " << regular_nodes << "\n";
    std::cout << "Captures:   " << perft_captures << " (expected 1576)\n";
    std::cout << "En Passant: " << perft_en_passants << "\n";
    std::cout << "Castles:    " << perft_castles << "\n";
    std::cout << "Promotions: " << perft_promotions << "\n";
    std::cout << "Checks:     " << perft_checks << " (expected 469)\n";
    std::cout << "Checkmates: " << perft_checkmates << " (expected 8)\n";
    std::cout << "==========================================\n\n";
}

// Helper: perft from starting position
uint64_t perft_startpos(int depth) {
    Board b;
    return perft(b, depth);
}

// Debug: count moves by piece type
void debug_moves_by_piece() {
    using namespace Bitboards;
    
    Board b;
    std::vector<Move> moves = b.generate_moves();
    
    int pawn_moves = 0, knight_moves = 0, bishop_moves = 0;
    int rook_moves = 0, queen_moves = 0, king_moves = 0;
    
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        
        if (white_pawns & from_bit) pawn_moves++;
        else if (white_knights & from_bit) knight_moves++;
        else if (white_bishops & from_bit) bishop_moves++;
        else if (white_rooks & from_bit) rook_moves++;
        else if (white_queens & from_bit) queen_moves++;
        else if (white_kings & from_bit) king_moves++;
    }
    
    std::cout << "\n=== Starting Position Moves by Piece ===\n";
    std::cout << "Pawns:   " << pawn_moves << " (expected 16: 8 single + 8 double)\n";
    std::cout << "Knights: " << knight_moves << " (expected 4)\n";
    std::cout << "Bishops: " << bishop_moves << " (expected 0)\n";
    std::cout << "Rooks:   " << rook_moves << " (expected 0)\n";
    std::cout << "Queens:  " << queen_moves << " (expected 0)\n";
    std::cout << "Kings:   " << king_moves << " (expected 0)\n";
    std::cout << "Total:   " << moves.size() << " (expected 20)\n";
    std::cout << "==========================================\n\n";
}

// Debug: count moves by piece type after specific moves
void debug_moves_after_e4() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    
    std::vector<Move> moves = b.generate_moves();
    
    int pawn_moves = 0, knight_moves = 0, bishop_moves = 0;
    int rook_moves = 0, queen_moves = 0, king_moves = 0;
    
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        
        if (black_pawns & from_bit) pawn_moves++;
        else if (black_knights & from_bit) knight_moves++;
        else if (black_bishops & from_bit) bishop_moves++;
        else if (black_rooks & from_bit) rook_moves++;
        else if (black_queens & from_bit) queen_moves++;
        else if (black_kings & from_bit) king_moves++;
    }
    
    std::cout << "\n=== After 1.e4 - Black's Moves by Piece ===\n";
    std::cout << "Pawns:   " << pawn_moves << " (expected 16)\n";
    std::cout << "Knights: " << knight_moves << " (expected 4)\n";
    std::cout << "Bishops: " << bishop_moves << " (expected 0)\n";
    std::cout << "Rooks:   " << rook_moves << " (expected 0)\n";
    std::cout << "Queens:  " << queen_moves << " (expected 0)\n";
    std::cout << "Kings:   " << king_moves << " (expected 0)\n";
    std::cout << "Total:   " << moves.size() << " (expected 20)\n";
    std::cout << "==========================================\n\n";
}

// Debug: count moves after e4 e5 to test more complex position
void debug_moves_after_e4_e5() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(4, 6), sq(4, 4), DOUBLE_PUSH)); // e7e5
    
    std::vector<Move> moves = b.generate_moves();
    
    int pawn_moves = 0, knight_moves = 0, bishop_moves = 0;
    int rook_moves = 0, queen_moves = 0, king_moves = 0;
    
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        
        if (white_pawns & from_bit) pawn_moves++;
        else if (white_knights & from_bit) knight_moves++;
        else if (white_bishops & from_bit) bishop_moves++;
        else if (white_rooks & from_bit) rook_moves++;
        else if (white_queens & from_bit) queen_moves++;
        else if (white_kings & from_bit) king_moves++;
    }
    
    std::cout << "\n=== After 1.e4 e5 - White's Moves ===\n";
    std::cout << "Pawns:   " << pawn_moves << " (expected 14)\n";
    std::cout << "Knights: " << knight_moves << " (expected 5)\n";
    std::cout << "Bishops: " << bishop_moves << " (expected 5)\n";
    std::cout << "Queens:  " << queen_moves << " (expected 4)\n";
    std::cout << "Kings:   " << king_moves << " (expected 1)\n";
    std::cout << "Total:   " << moves.size() << " (expected 29)\n";
    std::cout << "==========================================\n\n";
}

// Debug: test position after e4 e5 Nf3 (black to move)
void debug_after_e4_e5_Nf3() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(4, 6), sq(4, 4), DOUBLE_PUSH)); // e7e5
    b.make_move(Move(sq(6, 0), sq(5, 2), QUIET));       // Nf3
    
    std::vector<Move> moves = b.generate_moves();
    
    int pawn_moves = 0, knight_moves = 0, bishop_moves = 0;
    int rook_moves = 0, queen_moves = 0, king_moves = 0;
    
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        
        if (black_pawns & from_bit) pawn_moves++;
        else if (black_knights & from_bit) knight_moves++;
        else if (black_bishops & from_bit) bishop_moves++;
        else if (black_rooks & from_bit) rook_moves++;
        else if (black_queens & from_bit) queen_moves++;
        else if (black_kings & from_bit) king_moves++;
    }
    
    // After e4 e5 Nf3, black has:
    // Pawns: 14 (a6,a5,b6,b5,c6,c5,d6,d5,f6,f5,g6,g5,h6,h5) - e5 pawn blocked
    // Knights: 5 (Na6,Nc6,Ne7,Nf6,Nh6)
    // Bishops: 5 (Be7,Bd6,Bc5,Bb4,Ba3)
    // Queens: 4 (Qe7,Qf6,Qg5,Qh4)
    // Kings: 1 (Ke7)
    // Total: 29
    
    std::cout << "\n=== After 1.e4 e5 2.Nf3 - Black's Moves ===\n";
    std::cout << "Pawns:   " << pawn_moves << " (expected 14)\n";
    std::cout << "Knights: " << knight_moves << " (expected 5)\n";
    std::cout << "Bishops: " << bishop_moves << " (expected 5)\n";
    std::cout << "Queens:  " << queen_moves << " (expected 4)\n";
    std::cout << "Kings:   " << king_moves << " (expected 1)\n";
    std::cout << "Total:   " << moves.size() << " (expected 29)\n";
    std::cout << "==========================================\n\n";
}

// Debug: test position with capture - after e4 d5 (black can capture)
void debug_after_e4_d5() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(3, 6), sq(3, 4), DOUBLE_PUSH)); // d7d5
    
    std::vector<Move> moves = b.generate_moves();
    
    int captures = 0;
    for (const Move &m : moves) {
        if (m.flag == CAPTURE) captures++;
    }
    
    std::cout << "\n=== After 1.e4 d5 - White's Moves ===\n";
    std::cout << "Total:    " << moves.size() << " (expected 31)\n";
    std::cout << "Captures: " << captures << " (expected 1: exd5)\n";
    std::cout << "==========================================\n\n";
}

// Debug: test position with check - Scholar's Mate setup
void debug_check_position() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    // 1.e4 e5 2.Qh5 (attacks f7)
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(4, 6), sq(4, 4), DOUBLE_PUSH)); // e7e5
    b.make_move(Move(sq(3, 0), sq(7, 4), QUIET));       // Qh5
    
    std::cout << "\n=== After 1.e4 e5 2.Qh5 - Black's Moves ===\n";
    b.print_board();
    
    std::vector<Move> moves = b.generate_moves();
    
    auto sq_to_str = [](int s) {
        std::string str;
        str.push_back(static_cast<char>('a' + (s % 8)));
        str.push_back(static_cast<char>('1' + (s / 8)));
        return str;
    };
    
    // Count by piece type
    int pawn_moves = 0, knight_moves = 0, bishop_moves = 0;
    int queen_moves = 0, king_moves = 0;
    
    std::cout << "Pawn moves: ";
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        if (black_pawns & from_bit) {
            std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
            pawn_moves++;
        }
    }
    std::cout << "\n";
    
    std::cout << "Knight moves: ";
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        if (black_knights & from_bit) {
            std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
            knight_moves++;
        }
    }
    std::cout << "\n";
    
    std::cout << "Bishop moves: ";
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        if (black_bishops & from_bit) {
            std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
            bishop_moves++;
        }
    }
    std::cout << "\n";
    
    std::cout << "Queen moves: ";
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        if (black_queens & from_bit) {
            std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
            queen_moves++;
        }
    }
    std::cout << "\n";
    
    std::cout << "King moves: ";
    for (const Move &m : moves) {
        Bitboard from_bit = bit(m.from);
        if (black_kings & from_bit) {
            std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
            king_moves++;
        }
    }
    std::cout << "\n";
    
    std::cout << "\nCounts: Pawns=" << pawn_moves << " Knights=" << knight_moves 
              << " Bishops=" << bishop_moves << " Queens=" << queen_moves 
              << " Kings=" << king_moves << "\n";
    std::cout << "Total: " << moves.size() << " (expected ~28)\n";
    std::cout << "In check: " << (b.in_check() ? "YES" : "NO") << "\n";
    std::cout << "==========================================\n\n";
}

// Debug: test position with pin
void debug_pin_position() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    // Setup: 1.e4 e5 2.Bb5 (pins nothing yet but develops bishop)
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(4, 6), sq(4, 4), DOUBLE_PUSH)); // e7e5
    b.make_move(Move(sq(5, 0), sq(1, 4), QUIET));       // Bb5
    // Now black plays Nc6
    b.make_move(Move(sq(1, 7), sq(2, 5), QUIET));       // Nc6
    
    std::cout << "\n=== After 1.e4 e5 2.Bb5 Nc6 - White's Moves ===\n";
    b.print_board();
    
    std::vector<Move> moves = b.generate_moves();
    std::cout << "Total moves: " << moves.size() << "\n";
    
    // Now let's check if black can move the knight after 3.Nf3
    b.make_move(Move(sq(6, 0), sq(5, 2), QUIET));       // Nf3
    
    std::cout << "\n=== After 1.e4 e5 2.Bb5 Nc6 3.Nf3 - Black's Moves ===\n";
    b.print_board();
    
    std::vector<Move> moves2 = b.generate_moves();
    std::cout << "Total moves: " << moves2.size() << "\n";
    std::cout << "==========================================\n\n";
}

// Debug: count moves by type at starting position
void debug_move_counts(int depth) {
    Board b;
    std::vector<Move> moves = b.generate_moves();
    
    int counts[14] = {0}; // One for each MoveFlag
    int total = 0;
    
    for (const Move &m : moves) {
        counts[m.flag]++;
        total++;
    }
    
    std::cout << "\n=== Move Counts at Starting Position (depth " << depth << ") ===\n";
    std::cout << "Total moves: " << total << "\n";
    std::cout << "QUIET: " << counts[QUIET] << "\n";
    std::cout << "CAPTURE: " << counts[CAPTURE] << "\n";
    std::cout << "DOUBLE_PUSH: " << counts[DOUBLE_PUSH] << "\n";
    std::cout << "EN_PASSANT: " << counts[EN_PASSANT] << "\n";
    std::cout << "PROMO_QUEEN: " << counts[PROMO_QUEEN] << "\n";
    std::cout << "CASTLE_KINGSIDE: " << counts[CASTLE_KINGSIDE] << "\n";
    std::cout << "CASTLE_QUEENSIDE: " << counts[CASTLE_QUEENSIDE] << "\n";
    std::cout << "==========================================\n\n";
}

// Debug helper: print perft breakdown per root move for a given depth
void perft_split(int depth)
{
    Board b;
    // Use a recursive splitter that can drill into anomalies
    std::function<void(Board&, int, const std::string&, int)> recurse;
    recurse = [&](Board &board, int d, const std::string &prefix, int max_recurse)
    {
        std::vector<Move> moves = board.generate_moves();
        struct Entry { Move m; uint64_t nodes; };
        std::vector<Entry> entries;
        uint64_t total = 0;
        for (const Move &m : moves)
        {
            Bitboards::BoardState snap = Bitboards::snapshot_state();
            Board copy = board;
            copy.make_move(m);
            uint64_t nodes = perft(copy, d - 1);
            Bitboards::restore_state(snap);
            entries.push_back({m, nodes});
            total += nodes;
        }

        auto idx_to_sq = [](int sq) {
            int file = sq % 8;
            int rank = sq / 8;
            std::string s;
            s.push_back(static_cast<char>('a' + file));
            s.push_back(static_cast<char>('1' + rank));
            return s;
        };

        for (const auto &e : entries)
            std::cout << prefix << idx_to_sq(e.m.from) << idx_to_sq(e.m.to) << " : " << e.nodes << "\n";
        std::cout << prefix << "Perft split total for depth " << d << " = " << total << "\n";

        if (d > 1 && max_recurse > 0)
        {
            // sort descending and drill into top entries
            std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b){ return a.nodes > b.nodes; });
            int to_recurse = std::min((size_t)4, entries.size());
            for (int i = 0; i < to_recurse; ++i)
            {
                const Entry &e = entries[i];
                std::cout << "\n" << prefix << "Drilling into " << idx_to_sq(e.m.from) << idx_to_sq(e.m.to) << " (" << e.nodes << ")\n";
                Bitboards::BoardState snap = Bitboards::snapshot_state();
                Board copy = board;
                copy.make_move(e.m);
                recurse(copy, d - 1, prefix + "  ", max_recurse - 1);
                Bitboards::restore_state(snap);
            }

            // anomaly detection: compare to median and drill small branches
            std::vector<uint64_t> counts;
            for (const auto &e : entries) counts.push_back(e.nodes);
            if (!counts.empty())
            {
                std::sort(counts.begin(), counts.end());
                uint64_t median = counts[counts.size() / 2];
                uint64_t threshold = std::max<uint64_t>(10, static_cast<uint64_t>(median * 60 / 100));
                for (const auto &e : entries)
                {
                    if (e.nodes < threshold)
                    {
                        std::cout << "\nANOMALY: move=" << idx_to_sq(e.m.from) << idx_to_sq(e.m.to) << " nodes=" << e.nodes << " (threshold=" << threshold << ")\n";
                        Bitboards::BoardState snap = Bitboards::snapshot_state();
                        Board copy = board;
                        copy.make_move(e.m);
                        recurse(copy, d - 1, prefix + "  ", max_recurse - 1);
                        Bitboards::restore_state(snap);
                    }
                }
            }
        }
    };

    Board start = b;
    recurse(start, depth, "", 3);
}

// =============================================================
//                          PERFT TESTS
// =============================================================

TEST(perft_depth1) {
    ASSERT_EQ(perft_startpos(1), 20);
}

TEST(perft_depth2) {
    ASSERT_EQ(perft_startpos(2), 400);
}

TEST(perft_depth3) {
    ASSERT_EQ(perft_startpos(3), 8902);
}

// Debug: test en passant position
void debug_en_passant() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    // Setup position where en passant is possible
    // 1.e4 a6 2.e5 d5 (now exd6 e.p. is possible)
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(0, 6), sq(0, 5), QUIET));       // a7a6
    b.make_move(Move(sq(4, 3), sq(4, 4), QUIET));       // e4e5
    b.make_move(Move(sq(3, 6), sq(3, 4), DOUBLE_PUSH)); // d7d5
    
    std::cout << "\n=== En Passant Position ===\n";
    b.print_board();
    std::cout << "EP square: " << b.ep_square << " (should be d6 = " << sq(3, 5) << ")\n";
    
    std::vector<Move> moves = b.generate_moves();
    
    int ep_moves = 0;
    for (const Move &m : moves) {
        if (m.flag == EN_PASSANT) {
            ep_moves++;
            std::cout << "EN PASSANT: from=" << m.from << " to=" << m.to << "\n";
        }
    }
    std::cout << "En passant moves: " << ep_moves << " (expected 1: exd6)\n";
    std::cout << "Total moves: " << moves.size() << "\n";
    std::cout << "==========================================\n\n";
}

// Debug: test castling position
void debug_castling() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    // Setup: Clear path for kingside castling
    // 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 (both sides can castle kingside)
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH)); // e2e4
    b.make_move(Move(sq(4, 6), sq(4, 4), DOUBLE_PUSH)); // e7e5
    b.make_move(Move(sq(6, 0), sq(5, 2), QUIET));       // Nf3
    b.make_move(Move(sq(1, 7), sq(2, 5), QUIET));       // Nc6
    b.make_move(Move(sq(5, 0), sq(2, 3), QUIET));       // Bc4
    b.make_move(Move(sq(5, 7), sq(2, 4), QUIET));       // Bc5
    
    std::cout << "\n=== Castling Position ===\n";
    b.print_board();
    std::cout << "White kingside: " << b.white_castle_kingside << "\n";
    std::cout << "White queenside: " << b.white_castle_queenside << "\n";
    
    std::vector<Move> moves = b.generate_moves();
    
    int castle_moves = 0;
    for (const Move &m : moves) {
        if (m.flag == CASTLE_KINGSIDE || m.flag == CASTLE_QUEENSIDE) {
            castle_moves++;
            std::cout << "CASTLE: " << (m.flag == CASTLE_KINGSIDE ? "kingside" : "queenside") << "\n";
        }
    }
    std::cout << "Castle moves: " << castle_moves << " (expected 1: O-O)\n";
    std::cout << "Total moves: " << moves.size() << "\n";
    std::cout << "==========================================\n\n";
}

// Debug: verify castling rights are preserved correctly
void debug_castling_rights() {
    using namespace Bitboards;
    using Bitboards::sq;
    
    Board b;
    
    std::cout << "\n=== Castling Rights Tracking ===\n";
    std::cout << "Initial: WK=" << b.white_castle_kingside 
              << " WQ=" << b.white_castle_queenside
              << " BK=" << b.black_castle_kingside 
              << " BQ=" << b.black_castle_queenside << "\n";
    
    // 1.a4 - should NOT affect castling
    b.make_move(Move(sq(0, 1), sq(0, 3), DOUBLE_PUSH));
    std::cout << "After a4: WK=" << b.white_castle_kingside 
              << " WQ=" << b.white_castle_queenside << "\n";
    
    b.unmake_move(Move(sq(0, 1), sq(0, 3), DOUBLE_PUSH));
    std::cout << "After undo: WK=" << b.white_castle_kingside 
              << " WQ=" << b.white_castle_queenside << "\n";
    
    // Now try Ra2 (moving the a1 rook should lose queenside castling)
    // First clear the a-pawn
    b.make_move(Move(sq(0, 1), sq(0, 2), QUIET)); // a3
    b.make_move(Move(sq(0, 6), sq(0, 5), QUIET)); // a6
    b.make_move(Move(sq(0, 0), sq(0, 1), QUIET)); // Ra2
    
    std::cout << "After a3 a6 Ra2: WK=" << b.white_castle_kingside 
              << " WQ=" << b.white_castle_queenside << "\n";
    
    std::cout << "==========================================\n\n";
}

// Debug: compare perft at depth 3 starting from different positions
void debug_perft_comparison() {
    using namespace Bitboards;
    Board b;
    
    std::cout << "\n=== Perft Comparison ===\n";
    
    // Depth 3 from start should be 8902
    uint64_t nodes = perft(b, 3);
    std::cout << "Depth 3 from start: " << nodes << " (expected 8902)\n";
    
    // After e4, how many moves does black have?
    b.make_move(Move(sq(4, 1), sq(4, 3), DOUBLE_PUSH));
    
    std::cout << "\n--- After e4 ---\n";
    b.print_board();
    std::cout << "team_white: " << b.team_white << " (should be 0 for black)\n";
    std::cout << "ep_square: " << b.ep_square << " (should be e3 = " << sq(4, 2) << ")\n";
    
    std::vector<Move> moves_after_e4 = b.generate_moves();
    std::cout << "Black's moves: " << moves_after_e4.size() << " (expected 20)\n";
    
    // List all moves
    auto sq_to_str = [](int s) {
        std::string str;
        str.push_back(static_cast<char>('a' + (s % 8)));
        str.push_back(static_cast<char>('1' + (s / 8)));
        return str;
    };
    
    std::cout << "Moves: ";
    for (const Move &m : moves_after_e4) {
        std::cout << sq_to_str(m.from) << sq_to_str(m.to) << " ";
    }
    std::cout << "\n";
    
    // Manual perft to trace the bug
    std::cout << "\n--- Manual Perft 2 after e4 ---\n";
    uint64_t manual_total = 0;
    for (size_t i = 0; i < moves_after_e4.size(); i++) {
        b.make_move(moves_after_e4[i]);
        auto white_resp = b.generate_moves();
        manual_total += white_resp.size();
        if (i < 5) {
            std::cout << sq_to_str(moves_after_e4[i].from) << sq_to_str(moves_after_e4[i].to) 
                      << ": " << white_resp.size() << " white moves\n";
        }
        b.unmake_move(moves_after_e4[i]);
    }
    std::cout << "Manual total: " << manual_total << "\n";
    
    nodes = perft(b, 2);
    std::cout << "Perft(2): " << nodes << " (expected 400)\n";
    
    std::cout << "==========================================\n\n";
}

TEST(perft_depth4) {
    uint64_t result = perft_startpos(4);
    if (result != 197281) {
        std::string msg = "Expected 197281, got " + std::to_string(result);
        throw std::runtime_error(msg);
    }
}

TEST(perft_depth5) {
    uint64_t result = perft_startpos(5);
    std::cout << "Perft(5) = " << result << " (expected 4865609)\n";
    if (result != 4865609) {
        std::string msg = "Expected 4865609, got " + std::to_string(result);
        throw std::runtime_error(msg);
    }
}

TEST(perft_depth6) {
    std::cout << "(this may take a while...)\n";
    uint64_t result = perft_startpos(6);
    std::cout << "Perft(6) = " << result << " (expected 119060324)\n";
    if (result != 119060324) {
        std::string msg = "Expected 119060324, got " + std::to_string(result);
        throw std::runtime_error(msg);
    }
}

// =============================================================
//                   FEATURE VALIDATION TESTS
// =============================================================

// Helper to apply a sequence of moves
static bool apply_moves_seq(Board &b, const std::vector<std::string> &moves) {
    for (const auto &u : moves) {
        Move m = b.parse_uci_move(u);
        if (m.from < 0) return false;
        b.make_move(m);
    }
    return true;
}

// Helper to check if a move exists in legal moves
static bool has_move(const Board &b, const std::string &uci) {
    Move target = b.parse_uci_move(uci);
    if (target.from < 0) return false;
    auto moves = b.generate_moves();
    for (const Move &m : moves) {
        if (m.from == target.from && m.to == target.to) return true;
    }
    return false;
}

TEST(feature_pawn_moves) {
    Board b;
    if (!has_move(b, "e2e3")) throw std::runtime_error("e2e3 not legal");
    if (!has_move(b, "e2e4")) throw std::runtime_error("e2e4 not legal");
}

TEST(feature_pawn_double_push) {
    Board b;
    auto moves = b.generate_moves();
    bool found_double = false;
    for (const auto &m : moves) {
        if (m.flag == DOUBLE_PUSH) found_double = true;
    }
    if (!found_double) throw std::runtime_error("No DOUBLE_PUSH moves found");
}

TEST(feature_pawn_captures) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "d7d5"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "e4d5")) throw std::runtime_error("e4d5 capture not legal");
}

TEST(feature_en_passant) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "a7a6", "e4e5", "d7d5"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "e5d6")) throw std::runtime_error("en passant e5d6 not legal");
}

TEST(feature_promotions) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "a7a6", "e4e5", "a6a5", "e5e6", "a5a4", "e6e7", "h7h6"})) 
        throw std::runtime_error("Promotion setup failed");
    // Check if any promotion move exists from e7
    auto moves = b.generate_moves();
    bool found_promo = false;
    for (const auto &m : moves) {
        if (m.from == 52 && (m.flag >= PROMO_QUEEN && m.flag <= PROMO_CAPTURE_K)) {
            found_promo = true;
            break;
        }
    }
    if (!found_promo) throw std::runtime_error("No promotion moves from e7 found");
}

TEST(feature_knight_moves) {
    Board b;
    if (!has_move(b, "b1a3")) throw std::runtime_error("b1a3 not legal");
    if (!has_move(b, "b1c3")) throw std::runtime_error("b1c3 not legal");
}

TEST(feature_bishop_moves) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "a7a6"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "f1c4")) throw std::runtime_error("f1c4 not legal");
}

TEST(feature_rook_moves) {
    Board b;
    if (!apply_moves_seq(b, {"a2a3", "a7a6"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "a1a2")) throw std::runtime_error("a1a2 not legal");
}

TEST(feature_queen_moves) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "a7a6"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "d1h5")) throw std::runtime_error("d1h5 not legal");
}

TEST(feature_king_moves) {
    Board b;
    if (!apply_moves_seq(b, {"e2e3", "a7a6"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "e1e2")) throw std::runtime_error("e1e2 not legal");
}

TEST(feature_castling) {
    Board b;
    if (!apply_moves_seq(b, {"g1h3", "a7a6", "f1e2", "a6a5"})) throw std::runtime_error("Setup failed");
    if (!has_move(b, "e1g1")) throw std::runtime_error("e1g1 castling not legal");
}

TEST(feature_check_detection) {
    Board b;
    // Fool's mate: f2f4, e7e5, g2g4, d8h4 puts white in check
    if (!apply_moves_seq(b, {"f2f4", "e7e5", "g2g4", "d8h4"})) 
        throw std::runtime_error("Fool's mate setup failed");
    // After d8h4, white should be in check
    if (!b.in_check()) throw std::runtime_error("White should be in check after fool's mate");
}

TEST(feature_pin_detection) {
    Board b;
    if (!apply_moves_seq(b, {"e2e4", "e7e5", "f1c4", "a7a6", "d1h5", "b7b5"})) throw std::runtime_error("Setup failed");
    auto moves = b.generate_moves();
    if (moves.empty()) throw std::runtime_error("No legal moves available");
}

TEST(feature_legal_move_filtering) {
    Board b;
    auto moves = b.generate_moves();
    for (const Move &m : moves) {
        Bitboards::BoardState snap = Bitboards::snapshot_state();
        Board copy = b;
        copy.make_move(m);
        int king_sq = copy.find_king(!copy.team_white);
        bool in_check = copy.is_square_attacked(king_sq, copy.team_white);
        Bitboards::restore_state(snap);
        if (in_check) throw std::runtime_error("Legal move left king in check");
    }
}

TEST(feature_board_state_tracking) {
    // Test 1: single pawn push should not set ep_square
    Board b1;
    auto moves1 = b1.generate_moves();
    bool found_single_push = false;
    for (const auto &m : moves1) {
        if (m.flag == QUIET && m.from == 12) {  // e2 pawn single push
            found_single_push = true;
            Bitboards::BoardState snap = Bitboards::snapshot_state();
            Board b_copy = b1;
            b_copy.make_move(m);
            if (b_copy.ep_square != -1) throw std::runtime_error("ep_square should be -1 after single push");
            Bitboards::restore_state(snap);
            break;
        }
    }
    if (!found_single_push) throw std::runtime_error("No single pawn push found");
    
    // Test 2: double push should set ep_square
    Board b2;
    auto moves2 = b2.generate_moves();
    bool found_double = false;
    for (const auto &m : moves2) {
        if (m.flag == DOUBLE_PUSH) {
            found_double = true;
            Bitboards::BoardState snap = Bitboards::snapshot_state();
            Board b_copy = b2;
            b_copy.make_move(m);
            if (b_copy.ep_square == -1) throw std::runtime_error("ep_square should be set after DOUBLE_PUSH");
            Bitboards::restore_state(snap);
            break;
        }
    }
    if (!found_double) throw std::runtime_error("No DOUBLE_PUSH found");
}

TEST(feature_move_parsing) {
    Board b;
    
    // Test 1: simple move e2e4
    Move m1 = b.parse_uci_move("e2e4");
    if (m1.from != 12) throw std::runtime_error("e2 not parsed as 12");
    if (m1.to != 28) throw std::runtime_error("e4 not parsed as 28");
    
    // Test 2: Setup for promotion test
    Board b2;
    if (!apply_moves_seq(b2, {"e2e4", "a7a6", "e4e5", "a6a5", "e5e6", "a5a4", "e6e7", "h7h6"}))
        throw std::runtime_error("Setup failed for promotion parsing");
    
    // Test 3: parse promotion move
    Move m_promo = b2.parse_uci_move("e7e8q");
    if (m_promo.from != 52) throw std::runtime_error("e7 not parsed as 52");
    if (m_promo.to != 60) throw std::runtime_error("e8 not parsed as 60");
    // Note: flag may vary depending on board state, just ensure move is valid
    if (m_promo.flag < PROMO_QUEEN) throw std::runtime_error("promotion flag not recognized");
}


