// Project: Chesster - Simple Chess Engine
// File: board.h
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#pragma once
#include <cstdint>

// === Move Flags ===
// Flags tell us the type of the move
// Quiet = 0 means normal move (no capture, no promo, no special)
enum MoveFlag : uint8_t {
    QUIET            = 0,
    CAPTURE          = 1,
    DOUBLE_PUSH      = 2,
    EN_PASSANT       = 3,
    // Uncomment later when we add promotion implementation 
    /* PROMO_QUEEN      = 4,
    PROMO_ROOK       = 5,
    PROMO_BISHOP     = 6,
    PROMO_KNIGHT     = 7,
    PROMO_CAPTURE_Q  = 8,
    PROMO_CAPTURE_R  = 9,
    PROMO_CAPTURE_B  = 10,
    PROMO_CAPTURE_N  = 11 */
};


struct Move {
    int from;
    int to;
    MoveFlag flag;
    // constructor where flag is QUIET by default so here Move(from,to) is valid because the QUIET is implicit
    Move(int f, int t, MoveFlag fl = QUIET)
        : from(f), to(t), flag(fl) {}
};