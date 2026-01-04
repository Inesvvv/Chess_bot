// Project: Chesster - Simple Chess Engine
// File: board.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "search.h"
#include "evaluation.h"
#include "opening_book.h"
#include "bitboards.h"
#include <limits>
#include <vector>
#include <algorithm> // for std::sort
#include <unordered_map>
#include <chrono>

// definite infinite value used later for alpha and beta
static const int INF = std::numeric_limits<int>::max();

// ====== TIME MANAGEMENT ========
// Global variables to track time and abort search if needed
static std::chrono::steady_clock::time_point search_start_time;
int time_limit_ms = 9000;  // 9 seconds (leave 1s margin before 10s timeout)
static bool search_aborted = false;

// Node counter for periodic time checks (reset at start of each search)
static int node_count = 0;

// Total nodes explored in the search - put now to see if were improving the nb of nodes were looking at 
static uint64_t total_nodes_explored = 0;

// Maximum depth reached during iterative deepening - see if thats 
static int max_depth_reached = 0;

// ====== KILLER MOVES ========
// Killer moves are quiet moves that caused a beta cutoff at a given ply.
// We store 2 killer moves per ply (depth) - cant do more because going to take too much time. 
// In move ordering killer moves are right after captures, as they're going to be good in sibling nodes.
static const int MAX_PLY = 64;  // Maximum search depth - 64 is standard for chess engine but maybe review later 
static const int NUM_KILLERS = 2;  // Store 2 killer moves per ply

// Killer move struct: from, to, flag for each killer
struct KillerEntry {
    int from = -1;
    int to = -1;
    MoveFlag flag = QUIET;
};
static KillerEntry killer_moves[MAX_PLY][NUM_KILLERS];  // [ply][slot]

// Clear killer moves table (call at start of each search)
static void clear_killers() {
    for (int ply = 0; ply < MAX_PLY; ply++) {
        for (int slot = 0; slot < NUM_KILLERS; slot++) {
            killer_moves[ply][slot] = {-1, -1, QUIET};
        }
    }
}

// Store a killer move (quiet move that caused beta cutoff)
static void store_killer(const Move& move, int ply) {
    // Don't store captures as killers (they're already ordered by MVV-LVA)
    if (move.flag == CAPTURE || move.flag == EN_PASSANT ||
        (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K)) {
        return;
    }
    
    if (ply < 0 || ply >= MAX_PLY) return;
    
    // Don't store if it's already the first killer
    if (killer_moves[ply][0].from == move.from &&
        killer_moves[ply][0].to == move.to &&
        killer_moves[ply][0].flag == move.flag) {
        return;
    }
    
    // Shift: move first killer to second slot, store new killer in first
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = {move.from, move.to, move.flag};
}

// Check if a move is a killer at the given ply (returns 1 for first killer, 2 for second, 0 otherwise)
static int get_killer_slot(const Move& move, int ply) {
    if (ply < 0 || ply >= MAX_PLY) return 0;
    
    if (killer_moves[ply][0].from == move.from &&
        killer_moves[ply][0].to == move.to &&
        killer_moves[ply][0].flag == move.flag) {
        return 1;  // First killer
    }
    if (killer_moves[ply][1].from == move.from &&
        killer_moves[ply][1].to == move.to &&
        killer_moves[ply][1].flag == move.flag) {
        return 2;  // Second killer
    }
    return 0;
}

// ====== HISTORY HEURISTIC ========
// History heuristic tracks how often each move causes beta cutoffs across the entire search.
// Unlike killer moves which are ply-specific, history is accumulates over the search.
// Indexed by [from_square][to_square] - rewards moves that consistently cause cutoffs.
static int history_table[64][64];

// Maximum history score to prevent overflow and keep scores bounded
static const int MAX_HISTORY = 6000;

// Clear history table (call at start of each search)
static void clear_history() {
    for (int from = 0; from < 64; from++) {
        for (int to = 0; to < 64; to++) {
            history_table[from][to] = 0;
        }
    }
}

// Update history score for a move that caused beta cutoff
// Bonus is depth^2 - deeper cutoffs are more valuable
static void update_history(const Move& move, int depth) {
    // Only store quiet moves (captures already ordered by MVV-LVA)
    if (move.flag == CAPTURE || move.flag == EN_PASSANT ||
        (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K)) {
        return;
    }
    
    if (move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64) return;
    
    // Add depth^2 bonus 
    int bonus = depth * depth;
    history_table[move.from][move.to] += bonus;
    
    // Cap at MAX_HISTORY to prevent overflow
    if (history_table[move.from][move.to] > MAX_HISTORY) {
        history_table[move.from][move.to] = MAX_HISTORY;
    }
}

// Get history score for a move
static int get_history_score(const Move& move) {
    if (move.from < 0 || move.from >= 64 || move.to < 0 || move.to >= 64) return 0;
    return history_table[move.from][move.to];
}

// Check if we've exceeded the time limit
static bool is_time_up() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - search_start_time).count();
    return elapsed >= time_limit_ms;
}

// Check time periodically to avoid overhead when checking for time out
static bool check_time() {
    total_nodes_explored++;  // Count every node visited
    if (++node_count >= 1024) {  // Check every 1024 nodes for quicker aborts
        node_count = 0;
        if (is_time_up()) {
            search_aborted = true;
            return true;
        }
    }
    return search_aborted;
}

// ====== TRANSPOSITION TABLE ========
// Stores previously evaluated positions to avoid repetitive work
// because a position can be reached via different move orders (transposition)

enum TTFlag
{
    TT_EXACT, // Exact score means that we searched through all moves and found this exact score value
    TT_ALPHA, // Upper bound, never found a score better than alpha
    TT_BETA   // Lower bound, score greater than beta
};

struct TTEntry
{
    uint64_t hash; // Zobrist hash of position
    int depth;     // Depth of search when stored
    int score;     // Evaluation score
    TTFlag flag;   // Type of score bound
    Move bestMove; // Best move found (for move ordering)

    TTEntry() : hash(0), depth(0), score(0), flag(TT_EXACT), bestMove(-1, -1, QUIET) {}
};

// Transposition table using unordered_map
static std::unordered_map<uint64_t, TTEntry> transposition_table;

// Zobrist hashing for position identification
static uint64_t piece_keys[12][64]; // [piece_type][square]
static uint64_t side_key;           // XOR when black to move
static uint64_t castling_keys[16];  // Castling rights
static uint64_t ep_keys[64];        // En passant file
static bool zobrist_initialized = false;

static void init_zobrist()
{
    if (zobrist_initialized)
        return;

    uint64_t seed = 0x123456789ABCDEF0ULL; // arbitrary 64 bit nb
    auto rand64 = [&seed]()
    {
        // Xorshift64 -  12,25 and 27 chosen to avoid repetition (found on chess programming wiki)
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return seed * 0x2545F4914F6CDD1DULL; // multiply by huge number to increase randomness, this case constant with many alternating zeros and ones
    };
    // go through all pieces and squares to assign random keys
    for (int piece = 0; piece < 12; piece++)
    {
        for (int sq = 0; sq < 64; sq++)
        {
            piece_keys[piece][sq] = rand64();
        }
    }
    side_key = rand64();

    for (int i = 0; i < 16; i++)
    {
        castling_keys[i] = rand64();
    }
    for (int i = 0; i < 64; i++)
    {
        ep_keys[i] = rand64();
    }

    zobrist_initialized = true;
}

// Compute hash for current position
uint64_t compute_hash(const Board &board)
{
    init_zobrist();
    uint64_t hash = 0;

    // Hash pieces
    auto hash_pieces = [&](Bitboards::Bitboard bb, int piece_index)
    {
        Bitboards::Bitboard temp = bb;
        while (temp)
        {
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
    if (!board.team_white)
    {
        hash ^= side_key;
    }

    // Hash castling rights
    int castling = (board.white_castle_kingside ? 1 : 0) |
                   (board.white_castle_queenside ? 2 : 0) |
                   (board.black_castle_kingside ? 4 : 0) |
                   (board.black_castle_queenside ? 8 : 0);
    hash ^= castling_keys[castling];

    // Hash en passant
    if (board.ep_square >= 0)
    {
        hash ^= ep_keys[board.ep_square];
    }

    // once we've XORed all the hashes together we return the final hash
    return hash;
}

// Probe transposition table
static bool probe_tt(uint64_t hash, int depth, int alpha, int beta, int &score, Move &bestMove)
{
    // look up hash in the table and if not found return false because we dont have any info (haven't seen this position before)
    auto it = transposition_table.find(hash);
    if (it == transposition_table.end())
    {
        return false;
    }

    const TTEntry &entry = it->second; // it -> second gives us the TTentry that we have stored in the table  for this key

    // Verify hash match (handle collisions)
    if (entry.hash != hash)
    {
        return false;
    }

    // We retrieve best move for move ordering
    bestMove = entry.bestMove;

    // Only use score if search was at least as deep
    if (entry.depth >= depth)
    {
        if (entry.flag == TT_EXACT)
        {
            score = entry.score;
            return true;
        }
        else if (entry.flag == TT_ALPHA && entry.score <= alpha)
        {
            score = alpha;
            return true;
        }
        else if (entry.flag == TT_BETA && entry.score >= beta)
        {
            score = beta;
            return true;
        }
    }

    return false;
}

// Store everything in transposition table
static void store_tt(uint64_t hash, int depth, int score, TTFlag flag, const Move &bestMove)
{
    TTEntry entry;
    entry.hash = hash;
    entry.depth = depth;
    entry.score = score;
    entry.flag = flag;
    entry.bestMove = bestMove;
    transposition_table[hash] = entry;
}

// Clear transposition table (call between games)
void clear_transposition_table()
{
    transposition_table.clear();
}

// ====== PIECE SCORING FOR MOVE ORDERING ========

// Piece values for MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
static const int MVV_LVA_VICTIM_SCORES[7] = {0, 100, 320, 330, 500, 900, 10000}; // None, Pawn, Knight, Bishop, Rook, Queen, King
static const int MVV_LVA_ATTACKER_SCORES[7] = {0, 5, 4, 3, 2, 1, 0};             // Lower score = better attacker (pawn capturing is best)

// Get the piece type on a square (0=none, 1=pawn, 2=knight, 3=bishop, 4=rook, 5=queen, 6=king)
static int get_piece_on_square(int sq, bool for_white)
{
    Bitboards::Bitboard sqBit = Bitboards::bit(sq);

    if (for_white)
    {
        if (Bitboards::white_pawns & sqBit)
            return 1;
        if (Bitboards::white_knights & sqBit)
            return 2;
        if (Bitboards::white_bishops & sqBit)
            return 3;
        if (Bitboards::white_rooks & sqBit)
            return 4;
        if (Bitboards::white_queens & sqBit)
            return 5;
        if (Bitboards::white_kings & sqBit)
            return 6;
    }
    else
    {
        if (Bitboards::black_pawns & sqBit)
            return 1;
        if (Bitboards::black_knights & sqBit)
            return 2;
        if (Bitboards::black_bishops & sqBit)
            return 3;
        if (Bitboards::black_rooks & sqBit)
            return 4;
        if (Bitboards::black_queens & sqBit)
            return 5;
        if (Bitboards::black_kings & sqBit)
            return 6;
    }
    return 0;
}

// Score a move for ordering (higher = search first)
// Added ply parameter, used to check killer moves at the current depth
static int score_move(const Move &move, bool white_to_move, int ply = -1)
{
    int score = 0;

    // Promotions are very valuable - queen promotions are the best moves
    if (move.flag >= PROMO_QUEEN && move.flag <= PROMO_CAPTURE_K)
    {
        score += 20000; // Base promotion score (above captures)
        // Queen promotions are best
        if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q)
        {
            score += 900;
        }
        // Add capture bonus if it's a promotion-capture
        if (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K) {
            int victim_type = get_piece_on_square(move.to, !white_to_move);
            score += MVV_LVA_VICTIM_SCORES[victim_type] * 10;
        }
        return score; // Promotions are always most imporant
    }

    // Captures: use MVV-LVA
    if (move.flag == CAPTURE || move.flag == EN_PASSANT)
    {
        int victim_type;
        if (move.flag == EN_PASSANT)
        {
            victim_type = 1; // Pawn
        }
        else
        {
            // Get the captured piece type (opponent's piece on 'to' square)
            victim_type = get_piece_on_square(move.to, !white_to_move);
        }

        // Get attacker piece type
        int attacker_type = get_piece_on_square(move.from, white_to_move);

        // MVV-LVA: prioritize capturing high-value pieces with low-value pieces
        score += 10000 + MVV_LVA_VICTIM_SCORES[victim_type] * 10 - MVV_LVA_ATTACKER_SCORES[attacker_type];
        return score; // Captures are second priority
    }

    // Killer moves: quiet moves that caused beta cutoff at this ply
    // Scored below captures but above other quiet moves
    int killer_slot = get_killer_slot(move, ply);
    if (killer_slot == 1) {
        score += 9000;  // First killer (just below captures)
        return score;
    } else if (killer_slot == 2) {
        score += 8000;  // Second killer
        return score;
    }

    // Castling is usually good - score above history moves
    if (move.flag == CASTLE_KINGSIDE || move.flag == CASTLE_QUEENSIDE)
    {
        score += 7000;
        return score;
    }

    // History heuristic: add accumulated score for quiet moves
    // Max history is 6000, so this comes after castling (changed castling because before was 500 which was too low)
    score += get_history_score(move);

    return score;
}

// ========= SORT MOVES =========
// Sort moves for better alpha-beta pruning, moves with higher scores come first
// ttMove is the best move from transposition table (searched first if valid)
// ply is used to check killer moves at the current depth
static void order_moves(std::vector<Move> &moves, bool white_to_move, const Move &ttMove = Move(-1, -1, QUIET), int ply = -1)
{
    std::sort(moves.begin(), moves.end(), [white_to_move, &ttMove, ply](const Move &a, const Move &b)
              {
        // always check TT move first
        bool a_is_tt = (a.from == ttMove.from && a.to == ttMove.to && a.flag == ttMove.flag);
        bool b_is_tt = (b.from == ttMove.from && b.to == ttMove.to && b.flag == ttMove.flag);
        if (a_is_tt) return true;
        if (b_is_tt) return false;
        
        return score_move(a, white_to_move, ply) > score_move(b, white_to_move, ply); });
}

// ========= CAPTURE DETECTION =========
// Check if a move is a capture : normal capture, en passant and promotion captures)
static bool is_capture(const Move &move)
{
    return move.flag == CAPTURE ||
           move.flag == EN_PASSANT ||
           (move.flag >= PROMO_CAPTURE_Q && move.flag <= PROMO_CAPTURE_K);
}

// Forward declarations
// Added ply which tracks how deep we are from the root (for killer moves)
int negamax(Board &board, int depth, int alpha, int beta, int ply);
int quiescence(Board &board, int alpha, int beta);

// ========= QUIESCENCE SEARCH =========
// Searches only captures until the position is "quiet"
// This avoids the "horizon effect" where we stop searching right before
// a big capture happens (e.g., stopping search when queen can be taken next move)

// Delta pruning margins - if a capture doesnt raise alpha, skip it
static const int DELTA_MARGIN = 200;  // Safety margin
static const int PIECE_VALUES_QS[7] = {0, 100, 320, 330, 500, 900, 10000}; // For delta pruning

int quiescence(Board &board, int alpha, int beta)
{
    // ========= TIME CHECK =========
    if (check_time()) {
        return 0;  // Aborted, score doesn't matter
    }

    // Stand pat: evaluate the current position
    // This is the score if we choose not to make any capture
    int stand_pat = Evaluation::evaluate_board(board);

    // Beta cutoff: position is already too good for opponent
    if (stand_pat >= beta)
    {
        return beta;
    }

    // Update alpha if standing pat is better than current best
    if (stand_pat > alpha)
    {
        alpha = stand_pat;
    }

    // Generate all moves and filter for captures only
    std::vector<Move> moves = board.generate_moves();

    // Order captures by MVV-LVA
    order_moves(moves, board.team_white);

    for (const Move &move : moves)
    {
        // Only search captures in quiescence
        if (!is_capture(move))
        {
            continue;
        }

        // ========= DELTA PRUNING =========
        // Skip captures that can't possibly improve alpha
        // Even capturing the biggest piece + margin won't help
        int victim_value = 0;
        if (move.flag == EN_PASSANT) {
            victim_value = PIECE_VALUES_QS[1]; // Pawn
        } else {
            int victim_type = get_piece_on_square(move.to, !board.team_white);
            victim_value = PIECE_VALUES_QS[victim_type];
        }
        
        // Add bonus for promotions
        if (move.flag >= PROMO_QUEEN && move.flag <= PROMO_CAPTURE_K) {
            victim_value += 800; // Approximate promotion value gain
        }
        
        // If even with this capture we can't improve alpha, skip this 
        if (stand_pat + victim_value + DELTA_MARGIN < alpha) {
            continue;
        }

        board.make_move(move);
        int score = -quiescence(board, -beta, -alpha);
        board.unmake_move(move);

        // Beta cutoff
        if (score >= beta)
        {
            return beta;
        }

        // Update alpha
        if (score > alpha)
        {
            alpha = score;
        }
    }

    return alpha;
}

// Top-level function called to find the best move
// Uses iterative deepening: search depth 1, then 2, then 3... up to maxDepth
// Improvements:
// 1) Better move ordering from previous iterations
// 2) Time management - stops searching before 10 seconds
// 3) Always have a best move ready even if search is interrupted
SearchResult find_best_move(Board &board, int maxDepth)
{
    // Start the clock for time management
    search_start_time = std::chrono::steady_clock::now();
    search_aborted = false;
    node_count = 0;  // Reset node counter
    total_nodes_explored = 0;  // Reset total nodes counter
    max_depth_reached = 0;  // Reset depth tracker
    clear_killers();  // Reset killer moves for new search
    clear_history();  // Reset history table for new search

    std::vector<Move> moves = board.generate_moves();

    if (moves.empty())
    {
        Move dummyMove = Move(-1, -1, QUIET);
        int score;

        if (board.in_check())
        {
            // side to move is checkmated
            score = -INF + 1;
        }
        else
        {
            // draw by stalemate
            score = 0;
        }

        return SearchResult(dummyMove, score, 0, 0);
    }
    
    Move bestMove = moves[0];
    int bestScore = -INF;

    // ========= ITERATIVE DEEPENING =========
    // Search progressively deeper: depth 1, 2, 3, ... maxDepth
    // Each iteration improves move ordering for the next
    // Stops early if time is running out
    for (int currentDepth = 1; currentDepth <= maxDepth; currentDepth++)
    {
        // Check time before starting a new depth
        if (is_time_up())
        {
            break;  // Return best move from previous completed depth
        }

        int alpha = -INF;
        int beta = INF;

        
        int iterationBestScore = -INF;
        bool completedIteration = true;

        // Use best move from previous iteration for ordering
        order_moves(moves, board.team_white, bestMove, 0);
        Move iterationBestMove = moves[0];

        for (const Move &move : moves)
        {
            board.make_move(move);
            int score = -negamax(board, currentDepth - 1, -beta, -alpha, 1);
            board.unmake_move(move);

            // Check if search was aborted due to time
            if (search_aborted)
            {
                completedIteration = false;
                break;
            }

            if (score > iterationBestScore)
            {
                iterationBestScore = score;
                iterationBestMove = move;
            }

            if (score > alpha)
            {
                alpha = score;
            }
        }

        // Only update best move if we completed this iteration
        if (completedIteration)
        {
            bestMove = iterationBestMove;
            bestScore = iterationBestScore;
            max_depth_reached = currentDepth;  // Track the deepest depth so far
        }

        // Early exit if we found a forced mate
        if (bestScore > INF - 1000 || bestScore < -INF + 1000)
        {
            break;
        }

        // Check time after completing a depth
        if (is_time_up())
        {
            break;
        }
    }

    return SearchResult(bestMove, bestScore, total_nodes_explored, max_depth_reached);
}

int negamax(Board &board, int depth, int alpha, int beta, int ply)
{
    // ========= TIME CHECK =========
    if (check_time()) {
        return 0;  // Abort
    }

    // ========= DRAW DETECTION =========
    // Check for draws before searching deeper
    // Draws are evaluated as 0 (equal position)
    Board::DrawType draw_type = board.check_draw();
    if (draw_type != Board::NOT_A_DRAW)
    {
        return 0; // All draws return 0 score
    }

    // At depth 0, don't just evaluate but search captures until position is quiet
    if (depth == 0)
    {
        return quiescence(board, alpha, beta);
    }

    // ========= TRANSPOSITION TABLE LOOKUP =========
    // Check if we've seen this position before and returns the score if yes
    uint64_t hash = compute_hash(board);
    int ttScore;
    Move ttMove(-1, -1, QUIET);

    if (probe_tt(hash, depth, alpha, beta, ttScore, ttMove))
    {
        return ttScore; // Use cached result
    }

    // ========= NULL MOVE PRUNING =========
    // If we can give the opponent a free move and they still can't beat beta,
    // then this position is likely so good that we can prune the search.
    // Skip if: in check, at root, or at shallow depth
    bool in_check = board.in_check();
    if (!in_check && depth >= 3 && ply > 0)
    {
        // Make a "null move" we just flip the side to move (like we dont play)
        board.team_white = !board.team_white;
        
        // Search with reduced depth
        int R = 2;
        if (depth > 6) R = 3;  // More aggressive reduction at higher depths
        
        int null_score = -negamax(board, depth - 1 - R, -beta, -beta + 1, ply + 1);
        
        // Unmake null move
        board.team_white = !board.team_white;
        
        // If even with a free move opponent can't beat beta we prune
        if (null_score >= beta)
        {
            return beta;
        }
    }

    std::vector<Move> moves = board.generate_moves();

    if (moves.empty())
    {
        // no legal moves: checkmate or stalemate
        if (in_check)
        {
            return -INF + 1;
        }
        else
        {
            return 0;
        }
    }

    // Order moves for better alpha-beta pruning
    // Use TT best move if available, and pass ply for killer move scoring
    order_moves(moves, board.team_white, ttMove, ply);

    int bestScore = -INF;
    Move bestMove = moves[0];
    TTFlag ttFlag = TT_ALPHA; // Assume we won't improve alpha

    bool firstMove = true;
    int moveIndex = 0;

    for (const Move &move : moves)
    {
        board.make_move(move);

        int score;

        // Late Move Reductions (LMR): reduce depth for quiet late moves
        // the best move is most likely going to be at the beginning, so we dont want to
        // waste time doing full searches everytime, thus we do an LMR search which is a
        // more shallow search to quickly check if there could be a promising move, if a
        // promising move is found, then you perform the full search.
        int reduction = 0;
        bool dominated_by_in_check = board.in_check();  // Don't reduce if move gives check
        
        if (!is_capture(move) && !dominated_by_in_check && depth >= 2 && moveIndex >= 3 && !in_check)
        {
            // Increasing LMR aggressivness (start reducing earlier)
            reduction = 1;
            if (depth >= 4 && moveIndex >= 6)
                reduction = 2;
            if (depth >= 6 && moveIndex >= 10)
                reduction = 3;
        }

        if (firstMove)
        {
            //Full-window search for the first (best-ordered) move
            score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
        }
        else
        {

            if (reduction > 0)
            {
                // Do a reduced null-window search first
                score = -negamax(board, depth - 1 - reduction, -alpha - 1, -alpha, ply + 1);

                // If it looks interesting, we re-search at full depth
                // must treat any score that beats alpha as “suspiciously good” and verify it at full depth even if it is greater than beta
                if (score > alpha)
                {
                    score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
                }
            }
            else
            {
                // Principal Variation Search (PVS): null-window probe
                // idea: the first move (with good move ordering) is probably the best move.
                // do a cheap test search with a very narrow window [alpha, alpha+1] for later moves
                score = -negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1);
                // If it possibly improves alpha (it is a good candidate move), do a proper with full window [alpha, beta]
                // We don’t re-search if score >= beta, because we already know it’s good enough to cause a cutoff
                if (score > alpha && score < beta){
                    score = -negamax(board, depth - 1, -beta, -alpha, ply + 1);
                }
            }
        }

        board.unmake_move(move);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }

        if (score > alpha)
        {
            alpha = score;
            ttFlag = TT_EXACT; // Found an exact score (PV node)
        }

        if (alpha >= beta)
        {
            ttFlag = TT_BETA; // Failed high (cut node)
            // Store killer move: quiet move that caused beta cutoff
            store_killer(move, ply);
            // Update history: reward this move for causing cutoff
            update_history(move, depth);
            // store cutoff info in TT and break
            store_tt(hash, depth, bestScore, ttFlag, bestMove);
            return bestScore;
        }

        //once we've searched the first move, we are no longer on the 1st move
        firstMove = false;
        // move to the next move
        ++moveIndex;
    }

    // ========= STORE IN TRANSPOSITION TABLE =========
    store_tt(hash, depth, bestScore, ttFlag, bestMove);

    return bestScore;
}

// Try to get a move from the opening book
bool try_opening_book_move(Board &board, Move& book_move) {
    extern OpeningBook g_opening_book;
    
    if (!g_opening_book.is_loaded()) {
        return false;
    }
    
    // Get all available book moves for this position
    auto moves = g_opening_book.get_all_book_moves(board);
    
    if (moves.empty()) {
        return false;
    }
    
    // Return the move with the highest weight (best move)
    book_move = moves[0].first;
    return true;
}
