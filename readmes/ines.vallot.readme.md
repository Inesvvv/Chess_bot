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
- created perft file to test 100% legal moves until depth7
- We had issues with being 100% legal so fixed magic bitboards which were causing the problem as well as generate_legal_moves which was removing legal moves
- optimized the code because for depth5 in perft it was running in more than 10 seconds which wasn't a good sign for later so optimized by: fixing magic bitboards, using bulk counting (just for depth1), implemented make/un_make instead of board copies, using a vector reserve to gain time on reallocation, maybe update the compiler with DNDEBUG but not sure thats legal so in comment for now 

Week 3
- worked on search file implementing transposition tables to avoid recalculating the evaluation of the same position multiple times, for that used the zobrist hashing method to create the table keys, modified the move ordering to put the best move sfirst (promotion, capture then quiet) first in case of timing out to still have one of the best moves
- implemented time check in the search to return the current best move in case of timing out 
- we noticed that are black team was always playing the worst move because we had forgotten to return the negative score evaluation when we were the black team so modified that part too 
