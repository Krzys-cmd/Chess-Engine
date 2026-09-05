#pragma once
#include "types.h"
#include <string>
#include <vector>

struct Move {
    int fromSquare;
    int toSquare;
    int movedPiece;
    int capturedPiece;

    //special flags
    bool castling = false;
    bool enPassant = false;
    int promotedPiece = EMPTY; // (EMPTY) no promotion

    int oldEnPassantSquare = 64;

    bool prevWhiteKingside = false;
    bool prevWhiteQueenside = false;
    bool prevBlackKingside = false;
    bool prevBlackQueenside = false;

    int prevHalfMoveClock = 0;
};

struct PositionState {
    int chessboard[64];
    int sideToMove;
    bool wK, wQ, bK, bQ;
    int enPassant;

    bool equals(const PositionState& other) const {
        for (int i = 0; i < 64; i++) {
            if (chessboard[i] != other.chessboard[i]) return false;
        }
        return sideToMove == other.sideToMove &&
               wK == other.wK && wQ == other.wQ &&
               bK == other.bK && bQ == other.bQ &&
               enPassant == other.enPassant;
    }
};

class Board {
public:

    int chessboard[64];
    int enPassantSquare = 64;

    //castling flags
    bool whiteKingsideCastlingRights;
    bool whiteQueensideCastlingRights;
    bool blackKingsideCastlingRights;
    bool blackQueensideCastlingRights;

    int halfmoveClock = 0;
    std::vector<PositionState> positionHistory;

    Board();

    int loadFEN(const std::string& fen);//helper
    int findKing(int color) const;
    void startPositions();
    void printBoard() const; // ANSI print (testing)
    void unmakeMove(Move move);
    void makeMove(Move& move);
    bool isSquareAttacked(int square, int attackingColor) const;

    void savePosition(int sideToMove);
    void undoPosition();
    bool isThreefoldRepetition() const;
    bool is50MoveDraw() const { return halfmoveClock >= 100; }

};
