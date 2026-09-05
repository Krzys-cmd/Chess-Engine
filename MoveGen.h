#pragma once
#include <vector>
#include "Board.h"
#include "types.h"



class MoveGen {
public:
    MoveGen(Board& board);

    void generatePseudoLegal(int movingPieceColor, std::vector<Move>& out);
    void generateLegal(int color, std::vector<Move>& out);//legals moves filter

private:
    Board& board; //orginal chessboard

    //move gen for each piece
    void generateKnightMoves(int square, int myColor, std::vector<Move>& out);
    void generateKingMoves(int square, int myColor, std::vector<Move>& out);
    void generatePawnMoves(int square, int myColor, std::vector<Move>& out);
    void generateSlidingMoves(int square, int myColor, int pieceType, std::vector<Move>& out);//goniec wieza hetmamn

    int getPieceColor(int piece) const;
};
