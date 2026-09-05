#include "Board.h"
#include <iostream>

Board::Board(){
//swtting array to zero
     for(int i = 0; i < 64; i++){
        chessboard[i] = 0;
     }
}

void Board::startPositions(){
    for (int i = 0; i < 64; ++i) {
        chessboard[i] = EMPTY;
    }

    //biale figury
    chessboard[A1] = W_ROOK;
    chessboard[B1] = W_KNIGHT;
    chessboard[C1] = W_BISHOP;
    chessboard[D1] = W_QUEEN;
    chessboard[E1] = W_KING;
    chessboard[F1] = W_BISHOP;
    chessboard[G1] = W_KNIGHT;
    chessboard[H1] = W_ROOK;

    //white pawns
    for (int i = A2; i <= H2; ++i) {
        chessboard[i] = W_PAWN;
    }
    //black pawns
    for (int i = A7; i <= H7; ++i) {
        chessboard[i] = B_PAWN;
    }

    //black peices
    chessboard[A8] = B_ROOK;
    chessboard[B8] = B_KNIGHT;
    chessboard[C8] = B_BISHOP;
    chessboard[D8] = B_QUEEN;
    chessboard[E8] = B_KING;
    chessboard[F8] = B_BISHOP;
    chessboard[G8] = B_KNIGHT;
    chessboard[H8] = B_ROOK;

    //flags
    enPassantSquare = 64;
    whiteKingsideCastlingRights  = true;
    whiteQueensideCastlingRights = true;
    blackKingsideCastlingRights  = true;
    blackQueensideCastlingRights = true;
 }

void Board::printBoard() const{
  const char pieceChars[] = {
        '.', // EMPTY = 0
        'P', 'N', 'B', 'R', 'Q', 'K', // white: 1 - 6
        'p', 'n', 'b', 'r', 'q', 'k'  // black: 7 - 12
    };

   std::cout << "\x1b[?25l";
   std::cout << "\x1b[H";

   std::cout << "\n    a   b   c   d   e   f   g   h\n";
    std::cout << "  +---+---+---+---+---+---+---+---+\n";


    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " |";
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            int piece = chessboard[square];

            std::cout << " " << pieceChars[piece] << " |";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "\n";
}

void Board::makeMove(Move& move) {

    move.prevWhiteKingside  = whiteKingsideCastlingRights;
    move.prevWhiteQueenside = whiteQueensideCastlingRights;
    move.prevBlackKingside  = blackKingsideCastlingRights;
    move.prevBlackQueenside = blackQueensideCastlingRights;

    move.prevHalfMoveClock = halfmoveClock;

    if (move.movedPiece == W_PAWN || move.movedPiece == B_PAWN || move.capturedPiece != EMPTY) {
        halfmoveClock = 0;
    } else {
        halfmoveClock++;
    }

    move.oldEnPassantSquare = enPassantSquare; //old state
    enPassantSquare = 64;

    if (move.movedPiece == W_PAWN && (move.toSquare - move.fromSquare == 16)) {
        enPassantSquare = move.fromSquare + 8;
    }
    else if (move.movedPiece == B_PAWN && (move.fromSquare - move.toSquare == 16)) {
        enPassantSquare = move.fromSquare - 8;
    }

    //removing piece from starting postion and plaicing it on new postion
    chessboard[move.toSquare] = move.movedPiece;
    chessboard[move.fromSquare] = EMPTY;

    if (move.enPassant) {
        if (move.movedPiece == W_PAWN) {
            chessboard[move.toSquare - 8] = EMPTY;
        }
        else{
          chessboard[move.toSquare + 8] = EMPTY;
        }
    }

    //promotion swithcing pieces
    if (move.promotedPiece != EMPTY) {
        chessboard[move.toSquare] = move.promotedPiece;
    }

    //castling
    if (move.castling) {
             if (move.toSquare == G1) { chessboard[F1] = W_ROOK; chessboard[H1] = EMPTY; } //white short
        else if (move.toSquare == C1) { chessboard[D1] = W_ROOK; chessboard[A1] = EMPTY; } //white long
        else if (move.toSquare == G8) { chessboard[F8] = B_ROOK; chessboard[H8] = EMPTY; } //black short
        else if (move.toSquare == C8) { chessboard[D8] = B_ROOK; chessboard[A8] = EMPTY; } //black long
    }
    if (move.movedPiece == W_KING) { whiteKingsideCastlingRights = false; whiteQueensideCastlingRights = false; }
    if (move.movedPiece == B_KING) { blackKingsideCastlingRights = false; blackQueensideCastlingRights = false; }

    if (move.fromSquare == A1 || move.toSquare == A1) whiteQueensideCastlingRights = false;
    if (move.fromSquare == H1 || move.toSquare == H1) whiteKingsideCastlingRights = false;
    if (move.fromSquare == A8 || move.toSquare == A8) blackQueensideCastlingRights = false;
    if (move.fromSquare == H8 || move.toSquare == H8) blackKingsideCastlingRights = false;
}

void Board::unmakeMove(Move move) {
    //piece retruning to starting postion
    chessboard[move.toSquare] = EMPTY;

    //reversing capture or promotion
    chessboard[move.fromSquare] = move.movedPiece;

    if (move.enPassant) {
        //enpassant cofanie piona
        if (move.movedPiece == W_PAWN) {
            chessboard[move.toSquare - 8] = move.capturedPiece;
        } else {
            chessboard[move.toSquare + 8] = move.capturedPiece;
        }
    }
    else if (move.capturedPiece != EMPTY) {
        //normal capture
        chessboard[move.toSquare] = move.capturedPiece;
    }


    //reversing castling
    if (move.castling) {
             if (move.toSquare == G1) { chessboard[H1] = W_ROOK; chessboard[F1] = EMPTY; } //white short
        else if (move.toSquare == C1) { chessboard[A1] = W_ROOK; chessboard[D1] = EMPTY; } //white long
        else if (move.toSquare == G8) { chessboard[H8] = B_ROOK; chessboard[F8] = EMPTY; } //black short
        else if (move.toSquare == C8) { chessboard[A8] = B_ROOK; chessboard[D8] = EMPTY; } //black long
    }
    //reversing counter
    halfmoveClock = move.prevHalfMoveClock;

    // enpassant law
    enPassantSquare = move.oldEnPassantSquare;
    //castling law
    whiteKingsideCastlingRights = move.prevWhiteKingside;
    whiteQueensideCastlingRights = move.prevWhiteQueenside;
    blackKingsideCastlingRights = move.prevBlackKingside;
    blackQueensideCastlingRights = move.prevBlackQueenside;
}

bool Board::isSquareAttacked(int square, int attackingColor) const{
//Checks if a given square is attacked by any opponent piece, based on their move rules

 //Knight
    int knightMoves[8] = {6, 10, 15, 17, -6, -10, -15, -17};
    int startX = square % 8;
    int startY = square / 8;

    for (int i = 0; i < 8; i++) {
        int target = square + knightMoves[i];
        if (target >= 0 && target < 64) {
            int targetX = target % 8;
            int targetY = target / 8;
            if (std::abs(startX - targetX) + std::abs(startY - targetY) == 3) {
                int piece = chessboard[target];
                if (piece != EMPTY) {
                    //check if it is an enemy knight
                    bool isEnemyKnight = (attackingColor == WHITE && piece == W_KNIGHT) ||
                                         (attackingColor == BLACK && piece == B_KNIGHT);
                    if (isEnemyKnight) return true;
                }
            }
        }
    }
    //king
    int kingMoves[8] = { 8, -8, 1, -1, 9, 7, -7, -9 };
    for (int i = 0; i < 8; i++) {
        int target = square + kingMoves[i];
        if (target >= 0 && target < 64) {
            int targetX = target % 8;
            int targetY = target / 8;
            if (std::abs(startX - targetX) <= 1 && std::abs(startY - targetY) <= 1) {
                int piece = chessboard[target];
                if ((attackingColor == WHITE && piece == W_KING) ||
                    (attackingColor == BLACK && piece == B_KING)) {
                    return true;
                }
            }
        }
    }
    //pawn
    //conditions checking left and right attack
    if(attackingColor == WHITE){
        if(square % 8 != 0 && square - 9 >= 0 && chessboard[square - 9] == W_PAWN) return true;
        if(square % 8 != 7 && square - 7 >= 0 && chessboard[square - 7] == W_PAWN) return true;

    }
    else{
        if(square % 8 != 0 && square + 7 < 64 && chessboard[square + 7] == B_PAWN) return true;
        if(square % 8 != 7 && square + 9 < 64 && chessboard[square + 9] == B_PAWN) return true;
    }
    //sliding pieces
    int directions[8] = { 8, -8, 1, -1, 9, -9, 7, -7};

    for(int i = 0; i < 8; i++){
        int dir = directions[i];
        int currentSquare = square;

        while (true){
            int target = currentSquare + dir;
            if(target < 0 || target > 63) break;

            int currentX = currentSquare % 8;
            int currentY = currentSquare / 8;
            int targetX = target % 8;
            int targetY = target / 8;

            if(std::abs(targetX - currentX) > 1 || std::abs(targetY - currentY) > 1) break;

            int piece = chessboard[target];
            if(piece != EMPTY){

                int pieceColor = WHITE;
                if (piece >= B_PAWN && piece <= B_KING) pieceColor = BLACK;

                if (pieceColor == attackingColor) {
                    //checking straight lines
                    if (i <= 3) {
                        if (piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN) return true;
                    }
                    //remaining diagonals
                    else {
                        if (piece == W_BISHOP || piece == B_BISHOP || piece == W_QUEEN || piece == B_QUEEN) return true;
                    }
                }
                break;
            }
            currentSquare = target;
        }

    }
return false;
}

int Board::findKing(int color) const {
    int targetPiece = (color == WHITE) ? W_KING : B_KING;
    for (int i = 0; i < 64; ++i) {
        if (chessboard[i] == targetPiece) {
            return i;
        }
    }
    return SQ_NONE; //safeguard, although the king should always be on the board
}

void Board::savePosition(int sideToMove){
 PositionState state;
 for(int i = 0; i < 64; i++) state.chessboard[i] = chessboard[i];
 state.sideToMove = sideToMove;
 state.wK = whiteKingsideCastlingRights;
 state.wQ = whiteQueensideCastlingRights;
 state.bK = blackKingsideCastlingRights;
 state.bQ = blackQueensideCastlingRights;
 state.enPassant = enPassantSquare;
 positionHistory.push_back(state);

}

void Board::undoPosition(){
 if(!positionHistory.empty()){
    positionHistory.pop_back();
 }
}

bool Board::isThreefoldRepetition() const {
    if (positionHistory.empty()) return false;
    const PositionState& current = positionHistory.back();

    int count = 0;
    for (const PositionState& p : positionHistory) {
        if (p.equals(current)) count++;
    }
    return count >= 3;
}
