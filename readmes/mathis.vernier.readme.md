#Week 1: 24/11 - 30/11

- Created the bitboards.h and bitboards.cpp files initialising our general bitboards as well as the piece specific bitboards
- Initialisation of bit masks for ranks and files
- Defined squares using our standart bitboard
- Added the logic for rook and queen moves in the generate_moves vector

#Week 2: 1/12 - 6/12

- Implemented magic bitboards in order to efficiently code slider attacks
- redid the logic for rook, bishop and queen using these magic bitboards to be more efficient
- Fixed our pawn starting structure
- Added an initial castling logic which was corrected by apolline later
- Created an is_attacked function with Chloe in order to check if a square is being attacked, (I used it to check if castling is possible)
- Added the files for evaluation using piece values and specific cases to determine the evaluation of a board
- Fixed magic.cpp and magic.h

#Week 3: 7/12 - 13/12

- Fixed structure in board to try and get 100% legal moves
- added draw detection, checking or stalemate, threefold repetition, 50-move rule and insufficient material
- Added draw detection in search function: If losing, try to get a threefold repetition or another draw, If winning prevent draw


#Week 4: 14/12 - 21/12

- Added new evaluation functions like: outposts, multiple pawn penalty/bonus, bonus for space
- Fixed some eval values since they were impacting the game to much (we were doing stupid rook moves in almost every game a1b1 and h1g1 while none were open files losing tempo)
- implemented tempered evalution to avoid having a single switch (MIDDLEGAME/ENDGAME) and have a smoother transition