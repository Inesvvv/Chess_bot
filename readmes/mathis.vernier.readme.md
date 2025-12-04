#Week 1: 24/11 - 30/11

- Created the bitboards.h and bitboards.cpp files initialising our general bitboards as well as the piece specific bitboards
- Initialisation of bit masks for ranks and files
- Defined squares using our standart bitboard
- Added the logic for rook and queen moves in the generate_moves vector

#Week 2: 1/12 - 6/12

- Implemented magic bitboards in order to efficiently code sliders attacks
- redid the logic for rook, bishop and queen using these magic bitboards to be more efficient
- Fixed our pawn starting structure
- Added an initial catling logic which was corrected by apolline later
- Created an is attacked function with Chloe in order to check if a square is being attacked, (I used it to check if castling is possible)