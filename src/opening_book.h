// Project: Chesster - Simple Chess Engine
// File: opening_book.h
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-12-16

#pragma once

#include "board.h"
#include "move.h"
#include <cstdint>
#include <string>
#include <vector>
#include <map>

// Polyglot opening book entry structure (16 bytes)
// 8 bytes (zobrist hash) + 2 bytes (move) + 2 bytes (weight od the move) + 4 bytes (learn)
struct PolyglotEntry {
    uint64_t hash;          
    uint16_t move;           
    uint16_t weight;        
    uint32_t learn;          
};

class OpeningBook {
public:
    OpeningBook();
    ~OpeningBook();
    
    //tries loading a polyglot opening book from file
    bool load_book(const std::string& filename);
    
    //check if a position is in the opening book
    bool has_position(uint64_t zobrist_hash) const;
    
    //get a book move for the current position (returns a valid move if found)
    Move get_book_move(const Board& board);
    
    //get all available moves from the opening book for a position and return a vector of moves
    std::vector<std::pair<Move, uint16_t>> get_all_book_moves(const Board& board);
    
    //unload the opening book from memory
    void unload_book();
    
    //check if a book is currently loaded
    bool is_loaded() const { return book_loaded; }
    
    //get the number of positions in the book
    size_t size() const { return entries.size(); }
    
private:
    std::vector<PolyglotEntry> entries;
    bool book_loaded = false;
    
    //convert from a polyglot move format to our Move structure
    Move polyglot_move_to_move(const Board& board, uint16_t polyglot_move);
    
    //compute Zobrist hash for the current board position
    uint64_t compute_board_hash(const Board& board) const;
    
    //binary search to find entries for a given hash
    int find_entry_index(uint64_t hash) const;
};

//global opening book 
extern OpeningBook g_opening_book;
