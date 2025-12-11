#WEEK 1: 24/11 - 30/11

- Created /src folder
- Created main.cpp file and implemented argument parsing for -H and -m
- Created CMakeLists.txt file and implemented the build and make system
- Implemented get_random_move to board.cpp
- Implemented make_move to board.cpp and board.h


#WEEK 2: 01/12 - 06/12

- Implemented attack detection (with Mathis, merge conflict but we resolved it)
- Implemented king location detection
- Implemented check and double check detection
- Implemented legal move filtering and added this to get_random_move so that it only returns legal moves
- Implemented pin detection using pin masks and added this to generate_move()

#WEEK 3: 07/12 - 13/12

- Created the victory_regression testing file
- Reworked CMakeLists to share one source list/flags across binaries and test targets
- Implemented evaluate_king_safety to evaluation file
- Tested against stockfish (elo 1500), thus made uci_main.cpp and test_elo.sh files