// Project: Chesster - Simple Chess Engine
// File: board.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "search.h"
#include "evaluation.h"
#include "bitboards.h"
#include <limits>
#include <vector>
#include <algorithm> // for std::sort
#include <unordered_map>
//#include <chrono> add latter for the time management part in find_best_move

// definite infinite value used later for alpja and beta
static const int INF = std::numeric_limits<int>::max();

// ====== TRANSPOSITION TABLE ========
// Stores previously evaluated positions to avoid repetitive work
// because a position can be reached via different move orders (transposition)

enum TTFlag {
    TT_EXACT,    // Exact score means that we searched through all moves and found this exact score value 
    TT_ALPHA,    // Upper bound, never found a score better than alpha
    TT_BETA      // Lower bound, score greater than beta
};

struct TTEntry {
    uint64_t hash;      // Zobrist hash of position
    int depth;          // Depth of search when stored
    int score;          // Evaluation score
    TTFlag flag;        // Type of score bound
    Move bestMove;      // Best move found (for move ordering)
    
    TTEntry() : hash(0), depth(0), score(0), flag(TT_EXACT), bestMove(-1, -1, QUIET) {}
};

// Transposition table using unordered_map
static std::unordered_map<uint64_t, TTEntry> transposition_table;

// Zobrist hashing for position identification
static uint64_t piece_keys[12][64];  // [piece_type][square]
static uint64_t side_key;            // XOR when black to move
static uint64_t castling_keys[16];   // Castling rights
static uint64_t ep_keys[64];         // En passant file
static bool zobrist_initialized = false;

static void init_zobrist() {
    if (zobrist_initialized) return;
    
    uint64_t seed = 0x123456789ABCDEF0ULL; // arbitrary 64 bit nb 
    auto rand64 = [&seed]() {
        // Xorshift64 -  12,25 and 27 chosen to avoid repetition (found on chess programming wiki)
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return seed * 0x2545F4914F6CDD1DULL; // multiply by huge number to increase randomness, this case constant with many alternating zeros and ones 
    };
    // go through all pieces and squares to assign random keys
    for (int piece = 0; piece < 12; piece++) {
        for (int sq = 0; sq < 64; sq++) {
            piece_keys[piece][sq] = rand64();
        }
    }
    side_key = rand64();

    for (int i = 0; i < 16; i++) {
        castling_keys[i] = rand64();
    }
    for (int i = 0; i < 64; i++) {
        ep_keys[i] = rand64();
    }
    
    zobrist_initialized = true;
}

// Compute hash for current position
static uint64_t compute_hash(const Board& board) {
    init_zobrist();
    uint64_t hash = 0;
    
    // Hash pieces
    auto hash_pieces = [&](Bitboards::Bitboard bb, int piece_index) {
        Bitboards::Bitboard temp = bb;
        while (temp) {
            int sq = __builtin_ctzll(temp);
            temp &= temp - 1;
            hash ^= piece_keys[piece_index][sq];
        }
    };
    // hash the pieces for both sides
    hash_pieces(Bitboards::white_pawns, 0);
    hash_pieces(Bitboards::white_knights, 1);
    hash_pieces(Bitboards::white_bishops, 2);
    hash_pieces(Bitboards::white_rooks, 3);
    hash_pieces(Bitboards::white_queens, 4);
    hash_pieces(Bitboards::white_kings, 5);
    hash_pieces(Bitboards::black_pawns, 6);
    hash_pieces(Bitboards::black_knights, 7);
    hash_pieces(Bitboards::black_bishops, 8);
    hash_pieces(Bitboards::black_rooks, 9);
    hash_pieces(Bitboards::black_queens, 10);
    hash_pieces(Bitboards::black_kings, 11);
    
    // Hash side to move
    if (!board.team_white) {
        hash ^= side_key;
    }
    
    // Hash castling rights
    int castling = (board.white_castle_kingside ? 1 : 0) |
                   (board.white_castle_queenside ? 2 : 0) |
                   (board.black_castle_kingside ? 4 : 0) |
                   (board.black_castle_queenside ? 8 : 0);
    hash ^= castling_keys[castling];
    
    // Hash en passant
    if (board.ep_square >= 0) {
        hash ^= ep_keys[board.ep_square];
    }
    
    // once we've XORed all the hashes together we return the final hash
    return hash;
}

// Probe transposition table
static bool probe_tt(uint64_t hash, int depth, int alpha, int beta, int& score, Move& bestMove) {
    // look up hash in the table and if not found return false because we dont have any info (haven't seen this position before)
    auto it = transposition_table.find(hash);
    if (it == transposition_table.end()) {
        return false;
    }
    
    const TTEntry& entry = it->second; // it -> second gives us the TTentry that we have stored in the table  for this key 
    
    // Verify hash match (handle collisions)
    if (entry.hash != hash) {
        return false;
    }
    
    // We retrieve best move for move ordering
    bestMove = entry.bestMove;
    
    // Only use score if search was at least as deep
    if (entry.depth >= depth) {
        if (entry.flag == TT_EXACT) {
            score = entry.score;
            return true;
        } else if (entry.flag == TT_ALPHA && entry.score <= alpha) {
            score = alpha;
            return true;
        } else if (entry.flag == TT_BETA && entry.score >= beta) {
            score = beta;
            return true;
        }
    }
    
    return false;
}

// Store everything in transposition table
static void store_tt(uint64_t hash, int depth, int score, TTFlag flag, const Move& bestMove) {
    TTEntry entry;
    entry.hash = hash;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
    transposition_table[hash] = entry;
}

// Clear transposition table (call between games)
void clear_transposition_table() {
    transposition_table.clear();
}

// ====== PIECE SCORING FOR MOVE ORDERING ========

// Piece values for MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
static const int MVV_LVA_VICTIM_SCORES[7] = {0, 100, 320, 330, 500, 900, 10000}; // None, Pawn, Knight, Bishop, Rook, Queen, King
static const int MVV_LVA_ATTACKER_SCORES[7] = {0, 5, 4, 3, 2, 1, 0}; // Lower score = better attacker (pawn capturing is best)

// Get the piece type on a square (0=none, 1=pawn, 2=knight, 3=bishop, 4=rook, 5=queen, 6=king)
static int get_piece_on_square(int sq, bool for_white) {
    Bitboards::Bitboard sqBit = Bitboards::bit(sq);
    
    if (for_white) {
        if (Bitboards::white_pawns & sqBit) return 1;
        if (Bitboards::white_knights & sqBit) return 2;
        if (Bitboards::white_bishops & sqBit) return 3;
        if (Bitboards::white_rooks & sqBit) return 4;
        if (Bitboards::white_queens & sqBit) return 5;
        if (Bitboards::white_kings & sqBit) return 6;
    } else {
        if (Bitboards::black_pawns & sqBit) return 1;
        if (Bitboards::black_knights & sqBit) return 2;
        if (Bitboards::black_bishops & sqBit) return 3;
        if (Bitboards::black_rooks & sqBit) return 4;
        if (Bitboards::black_queens & sqBit) return 5;
        if (Bitboards::black_kings & sqBit) return 6;
    }
    return 0;
}

// Score a move for ordering (higher = search first)
static int score_move(const Move& move, bool white_to_move) {
    int score = 0;
    
    // Promotions are very valuable
    if (move.flag >= PROMO_QUEEN && move.flag <= PROMO_CAPTURE_K) {
        score += 9000; // Base promotion score
        // Queen promotions are best
        if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q) {
            score += 900;
        }
    }
    
    // Captures: use MVV-LVA
    if (move.flag == CAPTURE || move.flag == EN_PASSANT || 
        // greater and smaller or equal to here is more efficient way of checking each capture promo since ordered in move.h
        (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K)) {
        
        int victim_type;
        if (move.flag == EN_PASSANT) {
            victim_type = 1; // Pawn
        } else {
            // Get the captured piece type (opponent's piece on 'to' square)
            victim_type = get_piece_on_square(move.to, !white_to_move);
        }
        
        // Get attacker piece type
        int attacker_type = get_piece_on_square(move.from, white_to_move);
        
        // MVV-LVA: prioritize capturing high-value pieces with low-value pieces
        score += 10000 + MVV_LVA_VICTIM_SCORES[victim_type] * 10 - MVV_LVA_ATTACKER_SCORES[attacker_type];
    }
    
    // Castling is usually good but not as good as captures and promotions
    if (move.flag == CASTLE_KINGSIDE || move.flag == CASTLE_QUEENSIDE) {
        score += 500;
    }
    
    return score;
}

// ========= SORT MOVES =========
// Sort moves for better alpha-beta pruning, moves with higher scores come first
// ttMove is the best move from transposition table (searched first if valid)
static void order_moves(std::vector<Move>& moves, bool white_to_move, const Move& ttMove = Move(-1, -1, QUIET)) {
    std::sort(moves.begin(), moves.end(), [white_to_move, &ttMove](const Move& a, const Move& b) {
        // always check TT move first
        bool a_is_tt = (a.from == ttMove.from && a.to == ttMove.to && a.flag == ttMove.flag);
        bool b_is_tt = (b.from == ttMove.from && b.to == ttMove.to && b.flag == ttMove.flag);
        if (a_is_tt) return true;
        if (b_is_tt) return false;
        
        return score_move(a, white_to_move) > score_move(b, white_to_move);
    });
}

// ========= CAPTURE DETECTION =========
// Check if a move is a capture : normal capture, en passant and promotion captures)
static bool is_capture(const Move& move) {
    return move.flag == CAPTURE || 
           move.flag == EN_PASSANT ||
           (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K);
}

// Forward declarations
int negamax(Board& board, int depth, int alpha, int beta);
int quiescence(Board& board, int alpha, int beta);

// ========= QUIESCENCE SEARCH =========
// Searches only captures until the position is "quiet"
// This avoids the "horizon effect" where we stop searching right before
// a big capture happens (e.g., stopping search when queen can be taken next move)
int quiescence(Board& board, int alpha, int beta) {
    // Stand pat: evaluate the current position
    // This is the score if we choose not to make any capture
    int stand_pat = Evaluation::evaluate_board(board);
    
    // Beta cutoff: position is already too good for opponent
    if (stand_pat >= beta) {
        return beta;
    }
    
    // Update alpha if standing pat is better than current best
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    // Generate all moves and filter for captures only
    std::vector<Move> moves = board.generate_moves();
    
    // Order captures by MVV-LVA
    order_moves(moves, board.team_white);
    
    for (const Move& move : moves) {
        // Only search captures in quiescence
        if (!is_capture(move)) {
            continue;
        }
        
        board.make_move(move);
        int score = -quiescence(board, -beta, -alpha);
        board.unmake_move(move);
        
        // Beta cutoff
        if (score >= beta) {
            return beta;
        }
        
        // Update alpha
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

// Top-level function called to find the best move
// Uses iterative deepening: search depth 1, then 2, then 3... up to maxDepth
// Improvements:
// 1) Better move ordering from previous iterations
// 2) LATER !!! -> implement time management (stop when time runs out so no time out for 10sec)
// 3) Always have a best move ready even if search is interrupted 
SearchResult find_best_move(Board& board, int maxDepth) {
    std::vector<Move> moves = board.generate_moves();

    if (moves.empty()) {
        Move dummyMove = Move(-1, -1, QUIET);
        int score;

        if (board.in_check()) {
            // side to move is checkmated
            // the 1 is to avoid returning INF which could cause issues
            score = -INF + 1;
        } else {
            // draw by stalemate
            score = 0;
        }

        return SearchResult(dummyMove, score);
    }

    Move bestMove = moves[0];
    int bestScore = -INF;

    // ========= ITERATIVE DEEPENING =========
    // Search progressively deeper: depth 1, 2, 3, ... maxDepth
    // Each iteration improves move ordering for the next
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++) {
        int alpha = -INF;
        int beta = INF;
        
        Move iterationBestMove = moves[0];
        int iterationBestScore = -INF;
        
        // Use best move from previous iteration for ordering
        order_moves(moves, board.team_white, bestMove);

        for (const Move& move : moves) {
            board.make_move(move);
            int score = -negamax(board, currentDepth - 1, -beta, -alpha);
            board.unmake_move(move);

            if (score > iterationBestScore) {
                iterationBestScore = score;
                iterationBestMove = move;
            }

            if (score > alpha) {
                alpha = score;
            }
        }
        
        // Update best move/score after completing this depth
        bestMove = iterationBestMove;
        bestScore = iterationBestScore;
        
        // Optional: early exit if we found a forced mate
        if (bestScore > INF - 1000 || bestScore < -INF + 1000) {
            break;
        }
    }

    return SearchResult(bestMove, bestScore);
}


int negamax(Board& board, int depth, int alpha, int beta) {
    // At depth 0, don't just evaluate - search captures until position is quiet
    if (depth == 0) {
        return quiescence(board, alpha, beta);
    }

    // ========= TRANSPOSITION TABLE LOOKUP =========
    // Check if we've seen this position before and returns the score if yes
    uint64_t hash = compute_hash(board);
    int ttScore;
    Move ttMove(-1, -1, QUIET);
    
    if (probe_tt(hash, depth, alpha, beta, ttScore, ttMove)) {
        return ttScore;  // Use cached result
    }

    std::vector<Move> moves = board.generate_moves();

    if (moves.empty()) {
        //no legal moves: checkmate or stalemate
        if (board.in_check()) {
            return -INF + 1; 
        } else {
            return 0;
        }
    }

    // Order moves for better alpha-beta pruning
    // Use TT best move if available
    order_moves(moves, board.team_white, ttMove);

    int bestScore = -INF;
    Move bestMove = moves[0];
    TTFlag ttFlag = TT_ALPHA;  // Assume we won't improve alpha

    for (const Move& move : moves) {
        board.make_move(move);
        int score = -negamax(board, depth - 1, -beta, -alpha);
        board.unmake_move(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha) {
            alpha = score;
            ttFlag = TT_EXACT;  // Found an exact score (PV node)
        }

        if (alpha >= beta) {
            ttFlag = TT_BETA;  // Failed high (cut node)
            break;
        }
    }

    // ========= STORE IN TRANSPOSITION TABLE =========
    store_tt(hash, depth, bestScore, ttFlag, bestMove);

    return bestScore;
}
