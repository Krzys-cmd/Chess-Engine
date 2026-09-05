#pragma once
#include <string>
#include "Board.h"
#include "MoveGen.h"
#include "Search.h"

class UCI{
public:
    UCI(Board& board, MoveGen& gen, Search& search);

    void runLoop(); //main loop
private:
    Board& board;
    MoveGen& gen;
    Search& search;

    int activeColor;

    void handleUci();
    void handleIsReady();
    void handleUciNewGame();
    void handlePosition(std::istringstream& iss);
    void handleGo(std::istringstream& iss);

    int parseSquare(const std::string& s);
    std::string squareToString(int square);
    std::string moveToString(const Move& m);
    bool findMove(const std::string& text, Move& result);
};
