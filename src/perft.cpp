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
    std::vector<Move> moves = b.generate_legal_moves();
    uint64_t total = 0;
    for (const Move &m : moves)
    {
        Bitboards::BoardState snap = Bitboards::snapshot_state();
        Board copy = b;
        copy.make_move(m);
        uint64_t nodes = perft(copy, depth - 1);
        Bitboards::restore_state(snap);
        total += nodes;
        auto idx_to_sq = [](int sq) {
            int file = sq % 8;
            int rank = sq / 8;
            std::string s;
            s.push_back(static_cast<char>('a' + file));
            s.push_back(static_cast<char>('1' + rank));
            return s;
        };
        std::cout << "Move " << idx_to_sq(m.from) << idx_to_sq(m.to) << " : " << nodes << "\n";
    }
    std::cout << "Perft split total for depth " << depth << " = " << total << "\n";
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


