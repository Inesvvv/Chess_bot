# Chesster: a Chess Engine in C++

A complete chess engine written from scratch in C++17 by a team of four. It uses **bitboards**, **magic-bitboard** slider attacks, and an **alpha-beta** search with the standard modern pruning and move-ordering techniques. It speaks **UCI**, so it can play against Stockfish through cutechess, and it was connected to Lichess to play online.

On a laptop it reaches **depth 11 from the opening position in 2 seconds**.

## Architecture

```
src/
├── bitboards.*      64-bit board representation, rank/file masks
├── magic.*          magic bitboards for rook/bishop/queen attacks
├── board.*          move generation, make/unmake, legality, draw detection
├── move.h           compact move encoding (flags: castling, en passant, promotions…)
├── evaluation.*     tapered middlegame/endgame evaluation
├── search.*         iterative deepening alpha-beta search
├── main.cpp         file-based interface (-H history -m output)
├── uci_main.cpp     UCI protocol interface
├── eval_tester.cpp  evaluation sign-agreement checks against Stockfish
└── victory_regression.cpp  win-rate regression harness
```

### Move generation
- Bitboard representation with precomputed attack masks and **magic bitboards** for sliding pieces
- Full rules: castling, en passant, promotions, pins and check evasion
- **Perft-verified** legal move generation to depth 7
- Make/unmake instead of board copies, bulk counting and vector pre-allocation. These optimizations brought perft(5) down from over 10 seconds.

### Search
- Negamax **alpha-beta** with **iterative deepening** and time management
- **Transposition table** keyed by **Zobrist hashing**
- **Aspiration windows**, **principal variation search (PVS)** and **late move reductions (LMR)**
- **Null-move pruning** and **delta pruning** in quiescence search
- Move ordering: TT move, promotions, captures, **killer moves** and the **history heuristic**
- Draw handling for threefold repetition, the 50-move rule and insufficient material. The engine seeks a draw when losing and avoids one when winning.

### Evaluation
Tapered evaluation that interpolates between middlegame and endgame by game phase. Terms include material, piece-square tables, pawn structure (passed, isolated, backward and doubled pawns, pawn chains), king safety, mobility, knight outposts, rooks on open files, connected rooks, center control, space and trapped pieces.

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j

# Best move for a game given as a move-history file
./build/chesster -H tests/history_example2.txt -m next_move.txt

# UCI mode (for cutechess, Arena, or lichess-bot)
./build/chesster_uci

# Match against Stockfish limited to 1500 Elo (needs cutechess-cli + Stockfish)
bash scripts/test_elo.sh 1500 10
```

## Team and contributions

| Member | Main contributions |
|---|---|
| **Inès Vallot** | Board and move structure; knight, king, pawn and bishop move generation; en passant and pawn masks; perft testing and performance optimization; transposition table with Zobrist hashing; time control; killer/history move ordering; null-move and delta pruning; aspiration windows |
| Apolline Weinstein | File I/O, castling and promotion fixes, LMR and PVS, test positions, debugging |
| Mathis Vernier | Bitboards, magic bitboards, draw detection, tapered evaluation and evaluation terms |
| Chloé Deux-Pancera | Build system, attack/check/pin detection, king safety, UCI interface, Stockfish/Lichess testing |

Each member's week-by-week development log is in [`readmes/`](readmes/).
