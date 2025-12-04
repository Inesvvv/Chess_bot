Week 1
- created board.cpp and board.h
- implementation of the board class and move structure 
- created pop_lsb utility function which allows to find the positions of the pieces in the bitboard
- created generate_moves vector : knight moves, king moves, pawn moves, and bishop moves 

Week 2 
- improve pawns using masks to improve efficiency as well as avoid boundary issues such as wraparound 
- added en passant move to pawns 
- created isolated_pawn mask and the passed_pawn masks which will be used later for the evaluation of moves because an isolated pawn for example is very weak 
- modified the implementation of pawn in board.cpp to calculate the pawn moves 
- created move.h to modify the move structure by adding a type field 
- modified make_move to make en passant, double push moves 
- created perft file to test 100% legal moves
