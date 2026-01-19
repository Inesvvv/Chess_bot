// Project: Chesster - Simple Chess Engine
// File: board.cpp
// Authors: Chloe Deux-Pancera, Ines Vallot, Mathis Vernier, and Apolline Weinstein
// Created: 2025-11-28

#include "board.h"
#include "magic.h"
#include "search.h"
#include <array>
#include <cstdlib>
#include <iostream>

// Memo:
// rank = rows (1 to 8)
// file = columns (a to h)

using namespace Bitboards;

Board::Board() {
  // Initialize bitboards, attacks and masks
  init_bitboards();
  init_attack_tables();
  init_masks();
  Magic::init_magics();
}

// utility function that pops least significant bit and returns index
int Board::pop_lsb(Bitboard &b) const {
  if (b == 0)
    return -1;
  int position = __builtin_ctzll(b);
  b &= b - 1;
  return position;
}

// Checks if a square is attacked by a given color (opponent)
bool Board::is_square_attacked(int square, bool by_white) const {
  Bitboard sq_bit = bit(square);

  if (by_white) {
    // white pawn attacks
    if (pawn_attacks_black[square] & white_pawns)
      return true;

    // white knight attacks
    if (knight_attacks[square] & white_knights)
      return true;

    // white king attacks
    if (king_attacks[square] & white_kings)
      return true;

    // white bishop and queen attacks (diagonal)
    Bitboard bishop_attackers = Magic::get_bishop_attacks(square, occupied);
    if (bishop_attackers & (white_bishops | white_queens))
      return true;

    // white rook and queen attacks (straight)
    Bitboard rook_attackers = Magic::get_rook_attacks(square, occupied);
    if (rook_attackers & (white_rooks | white_queens))
      return true;
  } else {
    // black pawn attacks
    if (pawn_attacks_white[square] & black_pawns)
      return true;

    // black knight attacks
    if (knight_attacks[square] & black_knights)
      return true;

    // black king attacks
    if (king_attacks[square] & black_kings)
      return true;

    // black bishop and queen attacks (diagonal)
    Bitboard bishop_attackers = Magic::get_bishop_attacks(square, occupied);
    if (bishop_attackers & (black_bishops | black_queens))
      return true;

    // black rook and queen attacks (straight)
    Bitboard rook_attackers = Magic::get_rook_attacks(square, occupied);
    if (rook_attackers & (black_rooks | black_queens))
      return true;
  }

  return false;
}

// helper function to find king's square
int Board::find_king(bool by_white) const {
  Bitboard king = by_white ? white_kings : black_kings;

  if (king == 0) {
    return -1;
  }

  // check whether king bitboard has a bit on a square i
  for (int i = 0; i < 64; i++) {
    if (king & bit(i)) {
      return i;
    }
  }

  return -1;
}

// detect check and double check
int Board::count_checkers(bool white_to_move) const {
  int king_square = find_king(white_to_move);
  if (king_square < 0)
    return 0;

  int king_rank = king_square / 8;
  int king_file = king_square % 8;

  bool by_white = !white_to_move;

  int attackers = 0;

  // pawn check
  if (by_white) {
    if (king_rank > 0) {
      if (king_file < 7) {
        int from = (king_rank - 1) * 8 + (king_file + 1);
        if (white_pawns & bit(from)) {
          attackers += 1;
        }
      }
      if (king_file > 0) {
        int from = (king_rank - 1) * 8 + (king_file - 1);
        if (white_pawns & bit(from)) {
          attackers += 1;
        }
      }
    }
  } else {
    if (king_rank < 7) {
      if (king_file > 0) {
        int from = (king_rank + 1) * 8 + (king_file - 1);
        if (black_pawns & bit(from)) {
          attackers += 1;
        }
      }
      if (king_file < 7) {
        int from = (king_rank + 1) * 8 + (king_file + 1);
        if (black_pawns & bit(from)) {
          attackers += 1;
        }
      }
    }
  }

  // knight check
  Bitboard knight_mask = knight_attacks[king_square];
  Bitboard knight_attackers =
      knight_mask & (by_white ? white_knights : black_knights);

  for (int i = 0; i < 64; i++) {
    if (knight_attackers & bit(i)) {
      attackers += 1;
    }
  }

  // bishop and queen diagonal check
  Bitboard bishop_rays = Magic::get_bishop_attacks(king_square, occupied);
  Bitboard diagonal_attackers =
      bishop_rays & ((by_white ? white_bishops : black_bishops) |
                     (by_white ? white_queens : black_queens));

  for (int i = 0; i < 64; i++) {
    if (diagonal_attackers & bit(i)) {
      attackers += 1;
    }
  }

  // rook and queen straight check
  Bitboard rook_rays = Magic::get_rook_attacks(king_square, occupied);
  Bitboard straight_attackers =
      rook_rays & ((by_white ? white_rooks : black_rooks) |
                   (by_white ? white_queens : black_queens));

  for (int i = 0; i < 64; i++) {
    if (straight_attackers & bit(i)) {
      attackers += 1;
    }
  }

  return attackers;
}

// compute pinned pieces for a given color
// Bitboards::Bitboard Board::compute_pinned(bool for_white) const {
//   Bitboards::Bitboard pinned = 0ULL; // start with no pinned pieces

//   int king_square = find_king(for_white); // find king

//   if (king_square < 0)
//     return pinned;
  
//   int king_rank = king_square / 8;
//   int king_file = king_square % 8;

//   // 8 directions from king: N, S, E, W, NE, NW, SE, SW
//   const int dr[8] = {1, -1, 0, 0, 1, 1, -1, -1}; //rank change
//   const int df[8] = {0, 0, 1, -1, 1, -1, 1, -1}; // file change

//   // check each of 8 directions from the king
//   for (int dir = 0; dir < 8; ++dir) {
//     int r = king_rank + dr[dir];
//     int f = king_file + df[dir];
//     bool found_friendly = false;
//     int friendly_sq = -1;

//     // go along the direction until off the board
//     while (r >= 0 && r < 8 && f >= 0 && f < 8) {
//       //check each square
//       int s = sq(f, r);
//       Bitboard sbit = bit(s);
      
//       if (occupied & sbit) {
//         // if square is occupied, check if it's friendly or enemy
//         bool is_friendly = for_white ? (all_white & sbit) : (all_black & sbit); 
//         bool is_enemy = for_white ? (all_black & sbit) : (all_white & sbit);

//         if (is_friendly) { //case 1: found a friendly piece
//           if (!found_friendly) {
//             found_friendly = true;
//             friendly_sq = s; //potentially pinned piece
//           } else {
//             break;
//           }
//         } else if (is_enemy) { // case 2: found an enemy piece
//           if (!found_friendly) {
//             break; //in this case since we haven't seen a friendly piece yet, the enemy is just directly attacking the king (no pin)
//           } else {
//             // check whether this enemy piece is a sliding attacker along this direction
//             bool is_diag = (dir >= 4);
//             Bitboard enemy_rooks = for_white ? black_rooks : white_rooks;
//             Bitboard enemy_bishops = for_white ? black_bishops : white_bishops;
//             Bitboard enemy_queens = for_white ? black_queens : white_queens;

//             bool slider = false;
//             if (is_diag) {
//               if ((enemy_bishops & sbit) || (enemy_queens & sbit))
//                 slider = true;
//             } else {
//               if ((enemy_rooks & sbit) || (enemy_queens & sbit))
//                 slider = true;
//             }

//             if (slider) {
//               pinned |= bit(friendly_sq); //if the enemy is a slider, then the piece we found earlier (friendly_sq) is pinned !
//             }
//             break;
//           }
//         } else {
//           break;
//         }
//       }

//       r += dr[dir];
//       f += df[dir];
//     }
//   }
//   return pinned; // a bitboard with bits set for all pinned pieces
// }

// // build legal moves for pinned pieces
// void Board::compute_pin_masks( bool for_white, std::array<Bitboards::Bitboard, 64> &out_masks) const {
//   using Bitboard = Bitboards::Bitboard;
//   out_masks.fill(0ULL); // set all masks to 0 (no moves allowed by defualt)

//   int king_square = find_king(for_white);
//   if (king_square < 0)
//     return;

//   int king_rank = king_square / 8;
//   int king_file = king_square % 8;

//   const int dr[8] = {1, -1, 0, 0, 1, 1, -1, -1};
//   const int df[8] = {0, 0, 1, -1, 1, -1, 1, -1};

//   // for each direction from king:
//   for (int dir = 0; dir < 8; ++dir) {
//     int r = king_rank + dr[dir];
//     int f = king_file + df[dir];
//     bool found_friendly = false;
//     int friendly_sq = -1;

//     // go along the direction until off the board
//     while (r >= 0 && r < 8 && f >= 0 && f < 8) {
//       int s = sq(f, r); //square index
//       Bitboard sbit = bit(s); // square's bitboard

//       if (occupied & sbit) {
//         // if square is occupied, check if it's friendly or enemy (like in compute_pinned)
//         bool is_friendly = for_white ? (all_white & sbit) : (all_black & sbit);
//         bool is_enemy = for_white ? (all_black & sbit) : (all_white & sbit);

//         if (is_friendly) { 
//           if (!found_friendly) {
//             found_friendly = true;
//             friendly_sq = s;
//           } else {
//             break;
//           }
//         } else if (is_enemy) {
//           if (!found_friendly) {
//             break;
//           } else {
//             bool is_diag = (dir >= 4);
//             Bitboard enemy_rooks = for_white ? black_rooks : white_rooks;
//             Bitboard enemy_bishops = for_white ? black_bishops : white_bishops;
//             Bitboard enemy_queens = for_white ? black_queens : white_queens;

//             bool slider = false;
//             if (is_diag) {
//               if ((enemy_bishops & sbit) || (enemy_queens & sbit))
//                 slider = true;
//             } else {
//               if ((enemy_rooks & sbit) || (enemy_queens & sbit))
//                 slider = true;
//             }

//             // different from compute_pinned bc now we create the possible squares the pinned piece can go to (the mask)
//             if (slider) {
//               // build the pin ray square by square from king to attacker
//               // include the pinned piece's square in the mask
//               Bitboard line = bit(friendly_sq); // start a bitboard with just pinned piece's square for now

//               //start at square next to king in the direction of the pin we're currently looking at
//               int curr_r = king_rank + dr[dir]; 
//               int curr_f = king_file + df[dir];

//               // go along the direction until off the board
//               while (curr_r >= 0 && curr_r < 8 && curr_f >= 0 && curr_f < 8) {
                
//                 // for each square along the line, add it to the mask
//                 int curr_sq = sq(curr_f, curr_r);
//                 line |= bit(curr_sq); 

//                 if (curr_sq == s) {
//                   break; // stop when you reach teh enemy slider's square
//                 } 

//                 //move to the next square in same direction
//                 curr_r += dr[dir];
//                 curr_f += df[dir];
//               }

//               //store the mask for the pinned piece
//               out_masks[friendly_sq] = line;
//             }
//             break;
//           }
//         } else {
//           break;
//         }
//       }

//       r += dr[dir];
//       f += df[dir];
//     }
//   }
// }

// checks if the current player is in check
bool Board::in_check() const {
  int king_square = find_king(team_white);
  if (king_square == -1) {
    return false;
  }

  return is_square_attacked(king_square, !team_white); // check if king is attacked by opposing team
}

// Generate all pseudo-legal moves (no legality check - may leave king in check)
std::vector<Move> Board::generate_pseudo_legal_moves() const {
  std::vector<Move> moves;
  moves.reserve(64); // create a big vector to store all possible moves (up to 64 moves) for efficiency

  // set pieces to all our pieces and enemy_pieces to all opponent's pieces
  Bitboard pieces;
  Bitboard enemy_pieces;
  if (team_white) {
    pieces = all_white;
    enemy_pieces = all_black;
  } else {
    pieces = all_black;
    enemy_pieces = all_white;
  }

  // ==== KNIGHT MOVES =======
  Bitboard knights = team_white ? white_knights : black_knights;
  Bitboard temp_knights = knights;

  // loop through all knights
  while (temp_knights) {
    // for each knight, get all possible destination squares, excluding squares occupied by our pieces
    int from = pop_lsb(temp_knights);
    Bitboard attacks = knight_attacks[from] & ~pieces;

    Bitboard temp_attacks = attacks;

    // loop through all possible destination squares for this knight
    while (temp_attacks) {
      int to = pop_lsb(temp_attacks); //get next destination square
      if (enemy_pieces & bit(to))
        moves.push_back(Move(from, to, CAPTURE)); //if occupied by enemy piece, add capture move
      else
        moves.push_back({from, to}); //else, add quiet move
    }
  }

  // ======= KING MOVES ===========
  Bitboard king = team_white ? white_kings : black_kings;
  Bitboard temp_king = king;
  while (temp_king) {
    int from = pop_lsb(temp_king);
    Bitboard attacks = king_attacks[from] & ~pieces;

    Bitboard temp_attacks = attacks;

    while (temp_attacks) {
      int to = pop_lsb(temp_attacks);
      // Generate all king moves, legality checked later
      if (enemy_pieces & bit(to))
        moves.push_back(Move(from, to, CAPTURE));
      else
        moves.push_back({from, to});
    }

    // Castling
    if (team_white) {
      // White kingside castling
      if (white_castle_kingside) {
        if ((white_kings & bit(sq(4, 0))) && (white_rooks & bit(sq(7, 0))) && //check that king and rook are on their starting squares
            !(occupied & bit(sq(5, 0))) && !(occupied & bit(sq(6, 0)))) { //check that squares between king and chosen rook are empty
          if (!is_square_attacked(sq(4, 0), false) && // check that king's path and destination not attacked (ie does not leave king in check)
              !is_square_attacked(sq(5, 0), false) &&
              !is_square_attacked(sq(6, 0), false)) {
            moves.push_back(Move(sq(4, 0), sq(6, 0), CASTLE_KINGSIDE)); // if all conditions met, add a special castling move to move list : CASTLE_KINGSIDE
          }
        }
      }

      // White queenside castling
      if (white_castle_queenside) {
        if ((white_kings & bit(sq(4, 0))) && (white_rooks & bit(sq(0, 0))) &&
            !(occupied & bit(sq(1, 0))) && !(occupied & bit(sq(2, 0))) &&
            !(occupied & bit(sq(3, 0)))) { 
          if (!is_square_attacked(sq(4, 0), false) &&
              !is_square_attacked(sq(3, 0), false) &&
              !is_square_attacked(sq(2, 0), false)) {
            moves.push_back(Move(sq(4, 0), sq(2, 0), CASTLE_QUEENSIDE));
          }
        }
      }
    } else {
      // Black kingside castling (same as for white)
      if (black_castle_kingside) {
        if ((black_kings & bit(sq(4, 7))) && (black_rooks & bit(sq(7, 7))) &&
            !(occupied & bit(sq(5, 7))) && !(occupied & bit(sq(6, 7)))) {
          if (!is_square_attacked(sq(4, 7), true) &&
              !is_square_attacked(sq(5, 7), true) &&
              !is_square_attacked(sq(6, 7), true)) {
            moves.push_back(Move(sq(4, 7), sq(6, 7), CASTLE_KINGSIDE));
          }
        }
      }

      // Black queenside castling (same as for white)
      if (black_castle_queenside) {
        if ((black_kings & bit(sq(4, 7))) && (black_rooks & bit(sq(0, 7))) &&
            !(occupied & bit(sq(1, 7))) && !(occupied & bit(sq(2, 7))) &&
            !(occupied & bit(sq(3, 7)))) {
          if (!is_square_attacked(sq(4, 7), true) &&
              !is_square_attacked(sq(3, 7), true) &&
              !is_square_attacked(sq(2, 7), true)) {
            moves.push_back(Move(sq(4, 7), sq(2, 7), CASTLE_QUEENSIDE));
          }
        }
      }
    }
  }

  // =========== PAWN MOVES ============
  Bitboard pawns = team_white ? white_pawns : black_pawns;
  Bitboard temp_pawns = pawns;
  while (temp_pawns) {
    int from = pop_lsb(temp_pawns);

    if (team_white) {
      int to = from + 8;
      if (to < 64 && !(occupied & bit(to))) {

        int to_rank = to / 8;
        if (to_rank == 7) {
          moves.push_back(Move(from, to, PROMO_QUEEN));
          moves.push_back(Move(from, to, PROMO_ROOK));
          moves.push_back(Move(from, to, PROMO_BISHOP));
          moves.push_back(Move(from, to, PROMO_KNIGHT));
        } else {
          moves.push_back({from, to});
          if (from / 8 == 1) {
            int to2 = from + 16;
            if (!(occupied & bit(to2))) {
              moves.push_back(Move(from, to2, DOUBLE_PUSH));
            }
          }
        }
      }
    } else {
      int to = from - 8;
      if (to >= 0 && !(occupied & bit(to))) {

        int to_rank = to / 8;
        if (to_rank == 0) {
          moves.push_back(Move(from, to, PROMO_QUEEN));
          moves.push_back(Move(from, to, PROMO_ROOK));
          moves.push_back(Move(from, to, PROMO_BISHOP));
          moves.push_back(Move(from, to, PROMO_KNIGHT));
        } else {
          moves.push_back({from, to});
          if (from / 8 == 6) {
            int to2 = from - 16;
            if (!(occupied & bit(to2))) {
              moves.push_back(Move(from, to2, DOUBLE_PUSH));
            }
          }
        }
      }
    }

    // Capture moves
    Bitboard attacks;
    if (team_white) {
      attacks = pawn_attacks_white[from];
    } else {
      attacks = pawn_attacks_black[from];
    }
    Bitboard capture_targets = attacks & enemy_pieces;

    Bitboard temp_caps = capture_targets;
    while (temp_caps) {
      int to = pop_lsb(temp_caps);
      int to_rank = to / 8;
      bool promotion_rank =
          (team_white && to_rank == 7) || (!team_white && to_rank == 0);

      if (promotion_rank) {
        moves.push_back(Move(from, to, PROMO_CAPTURE_Q));
        moves.push_back(Move(from, to, PROMO_CAPTURE_R));
        moves.push_back(Move(from, to, PROMO_CAPTURE_B));
        moves.push_back(Move(from, to, PROMO_CAPTURE_K));
      } else {
        moves.push_back(Move(from, to, CAPTURE));
      }
    }

    // En passant capture - check if pawn can capture en passant
    if (ep_square != -1) {
      int from_rank = from / 8;
      int from_file = from % 8;
      int ep_rank = ep_square / 8;
      int ep_file = ep_square % 8;
      
      bool can_ep = false;
      if (team_white) {
        // White: pawn must be on rank 4, ep_square on rank 5, adjacent files
        can_ep = (from_rank == 4 && ep_rank == 5 && 
                  (ep_file == from_file - 1 || ep_file == from_file + 1));
      } else {
        // Black: pawn must be on rank 3, ep_square on rank 2, adjacent files
        can_ep = (from_rank == 3 && ep_rank == 2 && 
                  (ep_file == from_file - 1 || ep_file == from_file + 1));
      }
      
      if (can_ep) {
        // allow en-passant as a pseudo-legal move; exact legality (discovered
        // checks) will be filtered by generate_legal_moves
        moves.push_back(Move(from, ep_square, EN_PASSANT));
      }
    }
  }

  // ========== ROOK MOVES ==============
  Bitboard rooks = team_white ? white_rooks : black_rooks;
  Bitboard temp_rooks = rooks;
  while (temp_rooks) {
    int from = pop_lsb(temp_rooks);
    Bitboard attacks = Magic::get_rook_attacks(from, occupied) & ~pieces;

    Bitboard temp_attacks = attacks;
    while (temp_attacks) {
      int to = pop_lsb(temp_attacks);
      if (enemy_pieces & bit(to))
        moves.push_back(Move(from, to, CAPTURE));
      else
        moves.push_back({from, to});
    }
  }

  // ========== BISHOP MOVES ============
  Bitboard bishops = team_white ? white_bishops : black_bishops;
  Bitboard temp_bishops = bishops;
  while (temp_bishops) {
    int from = pop_lsb(temp_bishops);
    Bitboard attacks = Magic::get_bishop_attacks(from, occupied) & ~pieces;

    Bitboard temp_attacks = attacks;
    while (temp_attacks) {
      int to = pop_lsb(temp_attacks);
      if (enemy_pieces & bit(to))
        moves.push_back(Move(from, to, CAPTURE));
      else
        moves.push_back({from, to});
    }
  }

  // =========== QUEEN MOVES ============
  Bitboard queens = team_white ? white_queens : black_queens;
  Bitboard temp_queens = queens;
  while (temp_queens) {
    int from = pop_lsb(temp_queens);
    Bitboard attacks = Magic::get_queen_attacks(from, occupied) & ~pieces;

    Bitboard temp_attacks = attacks;
    while (temp_attacks) {
      int to = pop_lsb(temp_attacks);
      if (enemy_pieces & bit(to))
        moves.push_back(Move(from, to, CAPTURE));
      else
        moves.push_back({from, to});
    }
  }

  return moves;
}

// Check if a move is legal (doesn't leave king in check)
// This was taking too long because we were always copying to be able to tests
// move so we implement make and unmake here that temporarly test a move and
// then restore the original bitboard HUGE improvement in move generation
// optimization from 74 seconds to 36 seconds for perft depth6
bool Board::is_move_legal(const Move &m) const {
  // Cast away const to use make/unmake but we restore state after
  Board *self = const_cast<Board *>(this);

  self->make_move(m);

  // After make_move team_white has flipped so need to modify king to
  // !team_white
  int king_square = self->find_king(!self->team_white);

  // If king not found move illegal
  if (king_square == -1) {
    self->unmake_move(m);
    return false;
  }

  // Check if our king is attacked by the opponent (current team_white)
  bool in_check = self->is_square_attacked(king_square, self->team_white);

  self->unmake_move(m);

  return !in_check;
}

// Generate only legal moves
std::vector<Move> Board::generate_moves() const {
  std::vector<Move> pseudo_legal = generate_pseudo_legal_moves();
  std::vector<Move> legal;
  legal.reserve(pseudo_legal.size()); // Most moves are legal

  for (const Move &m : pseudo_legal) {
    if (is_move_legal(m)) {
      legal.push_back(m);
    }
  }

  return legal;
}

/*
// OLD generate_legal_moves - replaced by new implementation above
std::vector<Move> Board::generate_legal_moves_OLD() const
{
    std::vector<Move> psuedo_legal = generate_pseudo_legal_moves();
    std::vector<Move> legal;

    for (int i = 0; i < psuedo_legal.size(); i++)
    {
        Move m = psuedo_legal[i];

        Bitboards::BoardState snap = Bitboards::snapshot_state();
        Board copy = *this;
        copy.make_move(m);

        int king_square = copy.find_king(!copy.team_white);

        if (king_square == -1)
        {
            Bitboards::restore_state(snap);
            continue;
        }

        bool in_check = copy.is_square_attacked(king_square, copy.team_white);
        Bitboards::restore_state(snap);

        if (!in_check)
        {
            legal.push_back(m);
        }
    }

    return legal;
}
*/

// for now, return a random move from all moves
Move Board::get_random_move() const {
  std::vector<Move> moves = generate_moves();

  // for now this is a temporary output if there are no legal moves; avoids
  // division by zero
  if (moves.empty()) {
    return Move(-1, -1, QUIET);
  }

  // get a random index from moves and return it
  int index = rand() % moves.size();
  return moves[index];
}

// applies a given move to the board and updates who the current player is
void Board::make_move(const Move &move) {
  // save current state to undo stack
  UndoState u;
  u.bb_state = Bitboards::snapshot_state();
  u.team_white = team_white;
  u.ep_square = ep_square;
  u.white_castle_kingside = white_castle_kingside;
  u.white_castle_queenside = white_castle_queenside;
  u.black_castle_kingside = black_castle_kingside;
  u.black_castle_queenside = black_castle_queenside;
  u.position_history_size = position_history.size(); // Save history size before adding
  undo_stack.push_back(u);

  int from = move.from;
  int to = move.to;

  Bitboard from_bit = bit(from);
  Bitboard to_bit = bit(to);

  // ==== Update make move with move types =======
  ep_square = -1; // reset en passant square by default
  bool isPawn = false;
  if (team_white)
    isPawn = (white_pawns & from_bit);
  else
    isPawn = (black_pawns & from_bit);
  if (move.flag == DOUBLE_PUSH) {
    if (team_white) {
      ep_square = from + 8; // square behind the pawn after double push
    } else {
      ep_square = from - 8;
    }
  }
  // === En passant capture ==
  if (move.flag == EN_PASSANT) {
    int captured_pawn_square;
    if (team_white) {
      captured_pawn_square = to - 8;
      black_pawns &= ~bit(captured_pawn_square); // remove the captured pawn
    } else {
      captured_pawn_square = to + 8;
      white_pawns &= ~bit(captured_pawn_square);
    };
    // move the pawn to to
    if (team_white) {
      white_pawns &= ~from_bit;
      white_pawns |= to_bit;
    } else {
      black_pawns &= ~from_bit;
      black_pawns |= to_bit;
    }
    // Skip to the end of the move handling logic
    // Update the general bitboards and switch teams
    all_white = white_pawns | white_knights | white_bishops | white_rooks |
                white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks |
                black_queens | black_kings;
    occupied = all_white | all_black;

    // Switch teams
    team_white = !team_white;
    return;
  }
  // === Promotion===
  bool is_promotion =
      (move.flag == PROMO_QUEEN || move.flag == PROMO_ROOK ||
       move.flag == PROMO_BISHOP || move.flag == PROMO_KNIGHT ||
       move.flag == PROMO_CAPTURE_Q || move.flag == PROMO_CAPTURE_R ||
       move.flag == PROMO_CAPTURE_B || move.flag == PROMO_CAPTURE_K);
  if (is_promotion && isPawn) {
    // add the promoted piece
    if (team_white) {
      white_pawns &= ~from_bit;
      if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q)
        white_queens |= to_bit;
      else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R)
        white_rooks |= to_bit;
      else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B)
        white_bishops |= to_bit;
      else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K)
        white_knights |= to_bit;
    } else {
      black_pawns &= ~from_bit;
      if (move.flag == PROMO_QUEEN || move.flag == PROMO_CAPTURE_Q)
        black_queens |= to_bit;
      else if (move.flag == PROMO_ROOK || move.flag == PROMO_CAPTURE_R)
        black_rooks |= to_bit;
      else if (move.flag == PROMO_BISHOP || move.flag == PROMO_CAPTURE_B)
        black_bishops |= to_bit;
      else if (move.flag == PROMO_KNIGHT || move.flag == PROMO_CAPTURE_K)
        black_knights |= to_bit;
    }
    // If capture type remove the captured piece
    if (move.flag == PROMO_CAPTURE_Q || move.flag == PROMO_CAPTURE_R ||
        move.flag == PROMO_CAPTURE_B || move.flag == PROMO_CAPTURE_K) {
      if (team_white) {
        black_pawns &= ~to_bit;
        black_knights &= ~to_bit;
        black_bishops &= ~to_bit;
        black_rooks &= ~to_bit;
        black_queens &= ~to_bit;
        black_kings &= ~to_bit;
      } else {
        white_pawns &= ~to_bit;
        white_knights &= ~to_bit;
        white_bishops &= ~to_bit;
        white_rooks &= ~to_bit;
        white_queens &= ~to_bit;
        white_kings &= ~to_bit;
      }
    }
    // skip to the end and update the bitboards
    all_white = white_pawns | white_knights | white_bishops | white_rooks |
                white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks |
                black_queens | black_kings;
    occupied = all_white | all_black;

    team_white = !team_white;
    return;
  }

  // ===== CASTLING SPECIAL CASES =====
  if (move.flag == CASTLE_KINGSIDE || move.flag == CASTLE_QUEENSIDE) {
    if (team_white) {
      white_kings &= ~from_bit;
      white_kings |= to_bit;

      if (move.flag == CASTLE_KINGSIDE) {
        Bitboard rook_from = bit(sq(7, 0));
        Bitboard rook_to = bit(sq(5, 0));
        white_rooks &= ~rook_from;
        white_rooks |= rook_to;
      } else { // CASTLE_QUEENSIDE
        Bitboard rook_from = bit(sq(0, 0));
        Bitboard rook_to = bit(sq(3, 0));
        white_rooks &= ~rook_from;
        white_rooks |= rook_to;
      }

      white_castle_kingside = false;
      white_castle_queenside = false;
    } else {
      // Black castling
      black_kings &= ~from_bit;
      black_kings |= to_bit;

      if (move.flag == CASTLE_KINGSIDE) {
        Bitboard rook_from = bit(sq(7, 7));
        Bitboard rook_to = bit(sq(5, 7));
        black_rooks &= ~rook_from;
        black_rooks |= rook_to;
      } else { // CASTLE_QUEENSIDE
        Bitboard rook_from = bit(sq(0, 7));
        Bitboard rook_to = bit(sq(3, 7));
        black_rooks &= ~rook_from;
        black_rooks |= rook_to;
      }

      black_castle_kingside = false;
      black_castle_queenside = false;
    }

    // Update combined bitboards
    all_white = white_pawns | white_knights | white_bishops | white_rooks |
                white_queens | white_kings;
    all_black = black_pawns | black_knights | black_bishops | black_rooks |
                black_queens | black_kings;
    occupied = all_white | all_black;

    // Switch side to move
    team_white = !team_white;
    return;
  }

  // remove captured pieces
  if (team_white) {
    black_pawns &= ~to_bit;
    black_knights &= ~to_bit;
    black_bishops &= ~to_bit;
    black_rooks &= ~to_bit;
    black_queens &= ~to_bit;
    black_kings &= ~to_bit;

    // If capturing a rook on its starting square, update opponent's castling
    // rights
    if (to == sq(0, 7))
      black_castle_queenside = false; // a8 rook captured
    if (to == sq(7, 7))
      black_castle_kingside = false; // h8 rook captured
  } else {
    white_pawns &= ~to_bit;
    white_knights &= ~to_bit;
    white_bishops &= ~to_bit;
    white_rooks &= ~to_bit;
    white_queens &= ~to_bit;
    white_kings &= ~to_bit;

    // If capturing a rook on its starting square, update opponent's castling
    // rights
    if (to == sq(0, 0))
      white_castle_queenside = false; // a1 rook captured
    if (to == sq(7, 0))
      white_castle_kingside = false; // h1 rook captured
  }

  // move the piece (if a piece is on the square from_bit, remove it from
  // from_bit, and put it on to_bit)
  if (team_white) {
    if (white_pawns & from_bit) {
      white_pawns &= ~from_bit;
      white_pawns |= to_bit;
    } else if (white_knights & from_bit) {
      white_knights &= ~from_bit;
      white_knights |= to_bit;
    } else if (white_bishops & from_bit) {
      white_bishops &= ~from_bit;
      white_bishops |= to_bit;
    } else if (white_rooks & from_bit) {
      white_rooks &= ~from_bit;
      white_rooks |= to_bit;

      if (from == sq(0, 0))
        white_castle_queenside = false;
      if (from == sq(7, 0))
        white_castle_kingside = false;
    } else if (white_queens & from_bit) {
      white_queens &= ~from_bit;
      white_queens |= to_bit;
    } else if (white_kings & from_bit) {
      white_kings &= ~from_bit;
      white_kings |= to_bit;

      white_castle_kingside = false;
      white_castle_queenside = false;
    }
  } else {
    if (black_pawns & from_bit) {
      black_pawns &= ~from_bit;
      black_pawns |= to_bit;
    } else if (black_knights & from_bit) {
      black_knights &= ~from_bit;
      black_knights |= to_bit;
    } else if (black_bishops & from_bit) {
      black_bishops &= ~from_bit;
      black_bishops |= to_bit;
    } else if (black_rooks & from_bit) {
      black_rooks &= ~from_bit;
      black_rooks |= to_bit;

      if (from == sq(0, 7))
        black_castle_queenside = false;
      if (from == sq(7, 7))
        black_castle_kingside = false;
    } else if (black_queens & from_bit) {
      black_queens &= ~from_bit;
      black_queens |= to_bit;
    } else if (black_kings & from_bit) {
      black_kings &= ~from_bit;
      black_kings |= to_bit;

      black_castle_kingside = false;
      black_castle_queenside = false;
    }
  }

  // update the general bitboards
  all_white = white_pawns | white_knights | white_bishops | white_rooks |
              white_queens | white_kings;
  all_black = black_pawns | black_knights | black_bishops | black_rooks |
              black_queens | black_kings;
  occupied = all_white | all_black;

  // switch teams
  team_white = !team_white;

  // Add current position hash to history for threefold repetition detection
  // Note: We add the hash AFTER switching teams since the hash includes side to
  // move
  position_history.push_back(compute_hash(*this));
}

// Unmake move used in is_move_legal to restore the original bitboard after
// testing a move
void Board::unmake_move(const Move &move) {
  if (undo_stack.empty()) {
    return;
  }

  UndoState u = undo_stack.back();
  undo_stack.pop_back();

  // restore global bitboards
  Bitboards::restore_state(u.bb_state);

  // restore Board flags
  team_white = u.team_white;
  ep_square = u.ep_square;
  white_castle_kingside = u.white_castle_kingside;
  white_castle_queenside = u.white_castle_queenside;
  black_castle_kingside = u.black_castle_kingside;
  black_castle_queenside = u.black_castle_queenside;

  // Restore position history by removing entries added after this state
  // This is critical for performance during search - prevents unbounded growth
  position_history.resize(u.position_history_size);
}

// uses the current board state to interpret the UCI text and set the correct
// flag (enpassant, capture, ...).
Move Board::parse_uci_move(const std::string &uci) const {
  // Minimal UCI: "e2e4" or "e7e8q"
  if (uci.size() < 4) {
    // invalid, return sentinel
    return Move(-1, -1, QUIET);
  }

  auto file_index = [](char f) { return f - 'a'; };
  auto rank_index = [](char r) { return r - '1'; };

  int from_file = file_index(uci[0]);
  int from_rank = rank_index(uci[1]);
  int to_file = file_index(uci[2]);
  int to_rank = rank_index(uci[3]);

  if (from_file < 0 || from_file > 7 || to_file < 0 || to_file > 7 ||
      from_rank < 0 || from_rank > 7 || to_rank < 0 || to_rank > 7) {
    return Move(-1, -1, QUIET);
  }

  int from = from_rank * 8 + from_file;
  int to = to_rank * 8 + to_file;

  Bitboard from_bit = bit(from);
  Bitboard to_bit = bit(to);

  bool white = team_white;
  Bitboard my_pawns = white ? white_pawns : black_pawns;
  Bitboard my_kings = white ? white_kings : black_kings;
  Bitboard my_pieces = white ? all_white : all_black;
  Bitboard opp_pieces = white ? all_black : all_white;

  bool is_pawn = (my_pawns & from_bit) != 0;
  bool is_king = (my_kings & from_bit) != 0;

  MoveFlag flag = QUIET;

  // ---- Promotions ----
  bool promotion_rank = (white && to_rank == 7) || (!white && to_rank == 0);
  bool has_promo_char = (uci.size() >= 5);
  char promo_char = has_promo_char ? uci[4] : '\0';

  auto classify_promo = [&](bool is_capture) {
    switch (promo_char) {
    case 'q':
    case 'Q':
      return is_capture ? PROMO_CAPTURE_Q : PROMO_QUEEN;
    case 'r':
    case 'R':
      return is_capture ? PROMO_CAPTURE_R : PROMO_ROOK;
    case 'b':
    case 'B':
      return is_capture ? PROMO_CAPTURE_B : PROMO_BISHOP;
    case 'n':
    case 'N':
      return is_capture ? PROMO_CAPTURE_K : PROMO_KNIGHT;
    default:
      // default to queen if no char / invalid char
      return is_capture ? PROMO_CAPTURE_Q : PROMO_QUEEN;
    }
  };

  // ---- Castling ----
  if (is_king && std::abs(to_file - from_file) == 2) {
    if (to_file > from_file)
      flag = CASTLE_KINGSIDE;
    else
      flag = CASTLE_QUEENSIDE;

    return Move(from, to, flag);
  }

  // ---- En passant ----
  if (is_pawn && ep_square != -1 && to == ep_square && !(opp_pieces & to_bit)) {
    // pawn moves diagonally to empty square that is ep_square
    flag = EN_PASSANT;
    return Move(from, to, flag);
  }

  // ---- Promotions (quiet or capture) ----
  if (is_pawn && promotion_rank) {
    bool is_capture = (opp_pieces & to_bit) != 0;
    flag = classify_promo(is_capture);
    return Move(from, to, flag);
  }

  // ---- Double pawn push ----
  if (is_pawn && std::abs(to - from) == 16) {
    flag = DOUBLE_PUSH;
    return Move(from, to, flag);
  }

  // ---- Normal capture / quiet ----
  if (opp_pieces & to_bit)
    flag = CAPTURE;
  else
    flag = QUIET;

  return Move(from, to, flag);
}

// for testing purposes
void Board::print_board() const {
  std::cout << "\n  +-----------------+\n";
  for (int rank = 7; rank >= 0; --rank) {
    std::cout << rank + 1 << " | ";
    for (int file = 0; file < 8; ++file) {
      int sq_idx = rank * 8 + file;
      Bitboard b = bit(sq_idx);
      char c = '.';

      // White pieces (uppercase)
      if (white_pawns & b)
        c = 'P';
      else if (white_knights & b)
        c = 'N';
      else if (white_bishops & b)
        c = 'B';
      else if (white_rooks & b)
        c = 'R';
      else if (white_queens & b)
        c = 'Q';
      else if (white_kings & b)
        c = 'K';
      // Black pieces (lowercase)
      else if (black_pawns & b)
        c = 'p';
      else if (black_knights & b)
        c = 'n';
      else if (black_bishops & b)
        c = 'b';
      else if (black_rooks & b)
        c = 'r';
      else if (black_queens & b)
        c = 'q';
      else if (black_kings & b)
        c = 'k';

      std::cout << c << ' ';
    }
    std::cout << "|\n";
  }
  std::cout << "  +-----------------+\n";
  std::cout << "    a b c d e f g h\n";
  std::cout << (team_white ? "Side to move: White\n" : "Side to move: Black\n");
}

// ============= DRAW DETECTION METHODS =============

// Check for stalemate (no legal moves but not in check)
bool Board::is_stalemate() const {
  // Not a stalemate if in check
  if (in_check()) {
    return false;
  }

  // Generate all legal moves
  std::vector<Move> moves = generate_moves();

  // Stalemate if no legal moves and not in check
  return moves.empty();
}

// Check for insufficient material to checkmate
bool Board::is_insufficient_material() const {
  // Count pieces for both sides
  int white_pawn_count = __builtin_popcountll(white_pawns);
  int white_knight_count = __builtin_popcountll(white_knights);
  int white_bishop_count = __builtin_popcountll(white_bishops);
  int white_rook_count = __builtin_popcountll(white_rooks);
  int white_queen_count = __builtin_popcountll(white_queens);

  int black_pawn_count = __builtin_popcountll(black_pawns);
  int black_knight_count = __builtin_popcountll(black_knights);
  int black_bishop_count = __builtin_popcountll(black_bishops);
  int black_rook_count = __builtin_popcountll(black_rooks);
  int black_queen_count = __builtin_popcountll(black_queens);

  // Any pawns, rooks, or queens means sufficient material
  if (white_pawn_count > 0 || black_pawn_count > 0 || white_rook_count > 0 ||
      black_rook_count > 0 || white_queen_count > 0 || black_queen_count > 0) {
    return false;
  }

  // Total minor pieces
  int white_minors = white_knight_count + white_bishop_count;
  int black_minors = black_knight_count + black_bishop_count;

  // King vs King
  if (white_minors == 0 && black_minors == 0) {
    return true;
  }

  // King + minor vs King
  if ((white_minors == 1 && black_minors == 0) ||
      (white_minors == 0 && black_minors == 1)) {
    return true;
  }

  // King + Knight vs King + Knight
  if (white_knight_count == 1 && black_knight_count == 1 &&
      white_bishop_count == 0 && black_bishop_count == 0) {
    return true;
  }

  // Check for same-colored bishops (K+B vs K+B on same color)
  if (white_bishop_count == 1 && black_bishop_count == 1 &&
      white_knight_count == 0 && black_knight_count == 0) {
    // Get the squares of both bishops
    int white_bishop_sq = __builtin_ctzll(white_bishops);
    int black_bishop_sq = __builtin_ctzll(black_bishops);

    // Check if they're on same colored squares
    // A square is light if (rank + file) is even
    int white_color = (white_bishop_sq / 8 + white_bishop_sq % 8) % 2;
    int black_color = (black_bishop_sq / 8 + black_bishop_sq % 8) % 2;

    if (white_color == black_color) {
      return true;
    }
  }

  return false;
}

// Check for fifty-move rule
bool Board::is_fifty_move_rule() const {
  return halfmove_clock >= 100; // 100 half-moves = 50 full moves
}

// Check for threefold repetition
bool Board::is_threefold_repetition() const {
  // Compute current position hash (don't assume it's in history)
  uint64_t current_hash = compute_hash(*this);

  // Count how many times this position has occurred in history
  int count = 0;
  for (size_t i = 0; i < position_history.size(); ++i) {
    if (position_history[i] == current_hash) {
      count++;
    }
  }

  // Threefold repetition if position occurred 2+ times in history
  // (2 previous + current = 3 total)
  return count >= 2;
}

// Main draw checking function that returns the type of draw
Board::DrawType Board::check_draw() const {
  // Check stalemate first (no legal moves but not in check)
  if (is_stalemate()) {
    return STALEMATE;
  }

  // Check fifty-move rule
  if (is_fifty_move_rule()) {
    return FIFTY_MOVE_RULE;
  }

  // Check threefold repetition
  if (is_threefold_repetition()) {
    return THREEFOLD_REPETITION;
  }

  // Check insufficient material
  if (is_insufficient_material()) {
    return INSUFFICIENT_MATERIAL;
  }

  return NOT_A_DRAW;
}