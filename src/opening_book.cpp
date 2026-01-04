// Project: Chesster - Simple Chess Engine
// File: opening_book.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-16

#include "opening_book.h"
#include "board.h"
#include "bitboards.h"
#include <fstream>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <iomanip>

OpeningBook g_opening_book;

// Forward declaration from search.cpp
extern uint64_t compute_hash(const Board &board);

// Initialize zobrist constants
static void init_zobrist()
{
    // Nothing to do - search.cpp handles zobrist initialization
    // We delegate to search.cpp's compute_hash function
}

OpeningBook::OpeningBook()
{
    init_zobrist();
}

OpeningBook::~OpeningBook()
{
    unload_book();
}

bool OpeningBook::load_book(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "ERROR: Could not open opening book file: " << filename << "\n";
        return false;
    }

    // Clear any existing entries
    entries.clear();

    // Read all entries from the file
    PolyglotEntry entry;
    while (file.read(reinterpret_cast<char *>(&entry), sizeof(PolyglotEntry)))
    {
        entries.push_back(entry);
    }

    file.close();

    if (entries.empty())
    {
        std::cerr << "WARNING: Opening book file was empty or could not be read\n";
        book_loaded = false;
        return false;
    }

    // Sort entries by hash for efficient binary search
    std::sort(entries.begin(), entries.end(),
              [](const PolyglotEntry &a, const PolyglotEntry &b)
              {
                  return a.hash < b.hash;
              });

    book_loaded = true;
    std::cout << "Loaded opening book with " << entries.size() << " positions from "
              << filename << "\n";

    return true;
}

void OpeningBook::unload_book()
{
    entries.clear();
    book_loaded = false;
}

int OpeningBook::find_entry_index(uint64_t hash) const
{
    if (entries.empty())
        return -1;

    // Binary search for the first entry with this hash
    auto it = std::lower_bound(entries.begin(), entries.end(), hash,
                               [](const PolyglotEntry &entry, uint64_t h)
                               {
                                   return entry.hash < h;
                               });

    if (it != entries.end() && it->hash == hash)
    {
        return std::distance(entries.begin(), it);
    }

    return -1;
}

bool OpeningBook::has_position(uint64_t zobrist_hash) const
{
    return find_entry_index(zobrist_hash) != -1;
}

uint64_t OpeningBook::compute_board_hash(const Board &board) const
{
    // Delegate to search.cpp's compute_hash to ensure we use identical zobrist hashing
    return compute_hash(board);
}

Move OpeningBook::polyglot_move_to_move(const Board &board, uint16_t polyglot_move)
{
    // get the components from polyglot move format:
    // bits 0-5: from square (0-63)
    // bits 6-11: to square (0-63)
    // bits 12-14: promotion piece (0=none, 1=knight, 2=bishop, 3=rook, 4=queen)

    int from = polyglot_move & 0x3F;
    int to = (polyglot_move >> 6) & 0x3F;
    int promo = (polyglot_move >> 12) & 0x7;

    // determine the move type and create an appropriate Move
    using namespace Bitboards;
    MoveFlag flag = QUIET;

    // check if target square is occupied (capture)
    if ((occupied & bit(to)) != 0)
    {
        if (promo > 0)
        {
            // promotion capture
            switch (promo)
            {
            case 1:
                flag = PROMO_CAPTURE_K;
                break;
            case 2:
                flag = PROMO_CAPTURE_B;
                break;
            case 3:
                flag = PROMO_CAPTURE_R;
                break;
            case 4:
                flag = PROMO_CAPTURE_Q;
                break;
            }
        }
        else
        {
            flag = CAPTURE;
        }
    }
    else if (promo > 0)
    {
        // promotion non-capture
        switch (promo)
        {
        case 1:
            flag = PROMO_KNIGHT;
            break;
        case 2:
            flag = PROMO_BISHOP;
            break;
        case 3:
            flag = PROMO_ROOK;
            break;
        case 4:
            flag = PROMO_QUEEN;
            break;
        }
    }
    else if (board.ep_square == to)
    {
        // en passant
        flag = EN_PASSANT;
    }
    else if ((board.team_white && from == 4 && (to == 6 || to == 2)) || (!board.team_white && from == 60 && (to == 62 || to == 58)))
    {
        // castling
        if (to > from)
        {
            flag = CASTLE_KINGSIDE;
        }
        else
        {
            flag = CASTLE_QUEENSIDE;
        }
    }

    return Move(from, to, flag);
}

Move OpeningBook::get_book_move(const Board &board)
{
    if (!book_loaded)
    {
        return Move{-1, -1, QUIET};
    }

    // get all book moves and return the one with highest weight
    auto moves = get_all_book_moves(board);
    if (moves.empty())
    {
        return Move{-1, -1, QUIET};
    }

    return moves[0].first;
}

std::vector<std::pair<Move, uint16_t>> OpeningBook::get_all_book_moves(const Board &board)
{
    std::vector<std::pair<Move, uint16_t>> result;

    if (!book_loaded)
    {
        return result;
    }

    // compute the hash of the current position
    uint64_t board_hash = compute_board_hash(board);

    // find the first entry with this hash
    int idx = find_entry_index(board_hash);
    if (idx == -1)
    {
        return result; // position not in opening book
    }

    // collect all moves for this position (there may be multiple with same hash)
    for (size_t i = idx; i < entries.size(); i++)
    {
        if (entries[i].hash != board_hash)
        {
            break;
        }

        // convert polyglot move to our Move format
        Move move = polyglot_move_to_move(board, entries[i].move);

        // Only include moves with valid from/to squares
        if (move.from >= 0 && move.from < 64 && move.to >= 0 && move.to < 64 && move.from != move.to)
        {
            result.push_back({move, entries[i].weight});
        }
    }

    // sort by weight (highest first)
    std::sort(result.begin(), result.end(),
              [](const auto &a, const auto &b)
              {
                  return a.second > b.second;
              });

    return result;
}
