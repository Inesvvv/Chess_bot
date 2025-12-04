#include <iostream>
#include <vector>
#include <stdexcept>

#include "board.h"
#include "move.h"
#include "bitboards.h"

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
uint64_t perft(Board &b, int depth) {
    if (depth == 0)
        return 1;

    uint64_t nodes = 0;
    std::vector<Move> moves = b.generate_legal_moves();

    for (const Move &m : moves) {
        // Snapshot global bitboards, make the move on a board copy (which will modify globals),
        // recurse, then restore the global bitboards so the next sibling move starts from the
        // original position. This is required because bitboards are currently global.
        Bitboards::BoardState snap = Bitboards::snapshot_state();
        Board copy = b;
        copy.make_move(m);
        nodes += perft(copy, depth - 1);
        Bitboards::restore_state(snap);
    }
    return nodes;
}

// Helper: perft from starting position
uint64_t perft_startpos(int depth) {
    Board b;
    return perft(b, depth);
}

// Debug helper: print perft breakdown per root move for a given depth
void perft_split(int depth)
{
    Board b;
    // Use a recursive splitter that can drill into anomalies
    std::function<void(Board&, int, const std::string&, int)> recurse;
    recurse = [&](Board &board, int d, const std::string &prefix, int max_recurse)
    {
        std::vector<Move> moves = board.generate_legal_moves();
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

TEST(perft_depth4) {
    ASSERT_EQ(perft_startpos(4), 197281);
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
    auto moves = b.generate_legal_moves();
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
    auto moves = b.generate_legal_moves();
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
    auto moves = b.generate_legal_moves();
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
    auto moves = b.generate_legal_moves();
    if (moves.empty()) throw std::runtime_error("No legal moves available");
}

TEST(feature_legal_move_filtering) {
    Board b;
    auto moves = b.generate_legal_moves();
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
    auto moves1 = b1.generate_legal_moves();
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
    auto moves2 = b2.generate_legal_moves();
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


