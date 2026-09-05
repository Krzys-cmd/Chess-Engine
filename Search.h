#pragma once
#include "Board.h"
#include "MoveGen.h"
#include <chrono>
#include <iostream>

const int inf = 1000000;
const int mateValue = 100000;

class Search {
 public:
    Search(Board& board, MoveGen& gen);

    Move findBestMove(int color, int depth);
    Move searchTimedMove(int color, long long timeLimitMs);

    int negamax(int color, int depth, int ply, int alpha, int beta, Move* bestMoveOut = nullptr);
    long long getNodeCount() const { return nodes; }
 private:
    Board& board;
    MoveGen& gen;
    long long nodes = 0;

    int evaluatePosition(int color) const;

    int getMVVLVAMoveValue(const Move& m) const;

    std::chrono::steady_clock::time_point startTime;
    long long timeLimitMs = 0;
    bool interrupted = false;
    bool isTimeUp();

};
