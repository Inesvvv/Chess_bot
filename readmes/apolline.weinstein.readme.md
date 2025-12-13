#Week 1: 24/11 - 30/11

- This week I coded the input and output file reading and writing cfunctionality in order to be able to work with
the data provided by the professor.
- I also gathered everybody's code and fixed minor bugs in board.cpp and main.cpp so that the code fully runs.
- I created a tester file history_example in order to successfully test the code.
- Finally I organized the code and added headers to the tops of the files for more clarity.

#Week 2: 1/12 - 6/12

- I created a bunch of history example files
- I created a testing system to print the board so we can easily debug out code
- I finished the castling function which did not work (only the king was moving but the rook was never updated)
- I debugged everyone's code to ensure 100% legal moves
- I redid the promotion logic for pawns
- I fixed string_to_move as it only set from/to, but never flagged (new feature Ines added). So castling, en passant, promotions, and double push info was lost.
- For parsing through history files and outputting moves, i was not considering output promotion notation so i fixed that.


#Week 3: 7/12 - 13/12

- I implemented Late Move Reduction (LMR) and Principal Variation Search (PVS) to optimize the code
    - Our code works so that the best move is most likely going to be at the beginning. We don't want to waste time doing full searches everytime, thus we do a more shallow search to quickly check if there could be a promising move, if a promising move is found, then you perform the full search. 
- Debugged the code for a while here are some examples of bugs i fixed:
    - we were doing 50% legal moves --> this was because we were timing out. To fix this I realized we were not restoring position history in unmake_move() so while we were searching the position history was growing and slowing down the search
    - we had a duplicate en passant move so i fixed that
    - I also fixed a lot spacing and just visual aesthetics (that's why I randomly have 1000 insertions in my commits)
