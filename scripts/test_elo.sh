#!/bin/bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

OPPONENT_ELO=${1:-1500}
NUM_GAMES=${2:-50}

ENGINE_PATH="$REPO_ROOT/build/chesster_uci"
CUTECHESS_PATH="$HOME/Tools/cutechess/build/cutechess-cli"
STOCKFISH_PATH="$HOME/Tools/stockfish/stockfish-macos-m1-apple-silicon"
PGN_OUTPUT="$REPO_ROOT/build/games_vs_${OPPONENT_ELO}.pgn"

if [ ! -x "$ENGINE_PATH" ]; then
    echo "Error: Chesster UCI binary not found at $ENGINE_PATH"
    echo "Please run: cmake -S . -B build && cmake --build build --target chesster_uci"
    exit 1
fi

if [ ! -x "$CUTECHESS_PATH" ]; then
    echo "Error: cutechess-cli not found at $CUTECHESS_PATH"
    echo "Please build/install cutechess before running this script."
    exit 1
fi

if [ ! -x "$STOCKFISH_PATH" ]; then
    echo "Error: Stockfish binary not found at $STOCKFISH_PATH"
    echo "Ensure Stockfish is installed or update STOCKFISH_PATH in scripts/test_elo.sh"
    exit 1
fi

cat <<EOF
==============================================
 Chesster Elo Testing
==============================================
Opponent Elo : ${OPPONENT_ELO}
Games        : ${NUM_GAMES}
PGN output   : ${PGN_OUTPUT}
==============================================
EOF

"$CUTECHESS_PATH" \
    -engine name=Chesster cmd="$ENGINE_PATH" proto=uci \
    -engine name="Stockfish_${OPPONENT_ELO}" cmd="$STOCKFISH_PATH" proto=uci \
        option.UCI_LimitStrength=true \
        option.UCI_Elo="$OPPONENT_ELO" \
    -each tc=30+0.3 \
    -rounds "$NUM_GAMES" \
    -games 2 \
    -pgnout "$PGN_OUTPUT" \
    -recover \
    -repeat

cat <<EOF
==============================================
 Match Complete
==============================================
Games saved to: ${PGN_OUTPUT}
EOF
