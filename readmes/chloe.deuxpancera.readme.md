#WEEK 1

- Created /src folder
- Created main.cpp file and implemented argument parsing for -H and -m
- Created CMakeLists.txt file and implemented the build and make system
- Implemented get_random_move to board.cpp
- Implemented make_move to board.cpp and board.h


#WEEK 2

- Implemented attack detection (with Mathis, merge conflict but we resolved it)
- Implemented king location detection
- Implemented check and double check detection
- Implemented legal move filtering and added this to get_random_move so that it only returns legal moves
- Implemented pin detection using pin masks and added this to generate_move()

#WEEK 3

- Created the victory_regression testing file
- Reworked CMakeLists to share one source list/flags across binaries and test targets
- Implemented evaluate_king_safety to evaluation file
- Tested against stockfish (elo 1500), thus made uci_main.cpp and test_elo.sh files

#WEEK 4

- Added time management in search.cpp: hard-capped the time limit for each move to 9 seconds
- Clarified board.cpp, board.h, evaluation.cpp, and move.h files: comments and removed unnecessary code
- Edited some evaluation values in evaluation.h (for rooks semi open files, knight outpost, and queen severe)
- Connected chesster to a lichess account (chesster_engine) so that we can play against it
- Started learning about /deciding if we need  neutral networks

#WEEK 5

- Tried fixing eval_tester test with Mathis to get a higher sign agreement to match Stockfish's... did not work
- Edited searchDepth variable in main.cpp by testing different values (higher ones), and tested values by playing against Stockfish
- Stockfish-vs-Chesster commands so everyone can test (if they have stockfish):
	- Build the UCI binary: `cmake --build build --target chesster_uci`
	- Run 20 1500 Elo matches: `bash scripts/test_elo.sh 1500 10` (writes PGN log to build/games_vs_1500.pgn)