#include "MoveGen.h"
#include <cmath>
#include <iostream>

MoveGen::MoveGen(Board& b) : board(b) {}

int MoveGen::getPieceColor(int piece) const {
    if (piece >= W_PAWN && piece <= W_KING) return WHITE;
    if (piece >= B_PAWN && piece <= B_KING) return BLACK;
    return COLOR_NONE;
}

void MoveGen::generatePseudoLegal(int movingPieceColor, std::vector<Move>& out){
  out.clear();
  out.reserve(256);

  for(int square = 0; square < 64; square++){
    int piece = board.chessboard[square];
    if(piece == EMPTY) continue; // ignore empty square

    if(getPieceColor(piece) == movingPieceColor){
        // knight
        switch (piece) {
            case W_KNIGHT:
            case B_KNIGHT:
                generateKnightMoves(square, movingPieceColor, out);
                break;
            case W_KING:
            case B_KING:
                generateKingMoves(square, movingPieceColor, out);
                break;
            case W_PAWN:
            case B_PAWN:
                generatePawnMoves(square, movingPieceColor, out);
                break;
            case W_BISHOP:
            case B_BISHOP:
            case W_ROOK:
            case B_ROOK:
            case W_QUEEN:
            case B_QUEEN:
                generateSlidingMoves(square, movingPieceColor, piece, out);
                break;
        }
    }
  }
}

void MoveGen::generateKnightMoves(int square, int myColor, std::vector<Move>& out){
 // possible index offsets in a array
 int knightMoves[8] = {6,10,15,17,-6,-10,-15,-17};
 // knight coordinates
 int startX = square % 8; // x
 int startY = square / 8;

    for(int i = 0; i < 8; i++){
      int targetSquare = square + knightMoves[i];
      if (targetSquare >= 0 && targetSquare < 64){ // check if it stays within board bounds
         int targetX = targetSquare % 8;
         int targetY = targetSquare / 8;
          if(std::abs(startX - targetX)+std::abs(startY - targetY) == 3){ // edge wrap protection
            int targetPiece = board.chessboard[targetSquare];
             if(targetPiece == EMPTY || getPieceColor(targetPiece) != myColor){
               Move move;
               move.fromSquare = square;
               move.toSquare = targetSquare;
               move.movedPiece = board.chessboard[square];
               move.capturedPiece = targetPiece;
               out.push_back(move);
             }
          }
      }
    }
}

void MoveGen::generateKingMoves(int square, int myColor, std::vector<Move>& out) {
 int kingMoves[8] = {1,-1,8,-8,9,-9,7,-7};
 int startX = square % 8;
 int startY = square / 8;

    for(int i = 0; i < 8; i++){
      int targetSquare = square + kingMoves[i];
      if (targetSquare >= 0 && targetSquare < 64){
         int targetX = targetSquare % 8;
         int targetY = targetSquare / 8;
          if(std::abs(startX - targetX) <= 1 && std::abs(startY - targetY) <= 1){
            int targetPiece = board.chessboard[targetSquare];
             if(targetPiece == EMPTY || getPieceColor(targetPiece) != myColor){
               Move move;
               move.fromSquare = square;
               move.toSquare = targetSquare;
               move.movedPiece = board.chessboard[square];
               move.capturedPiece = targetPiece;
               out.push_back(move);
             }
          }
      }
    }

    // CASTLING

    // white
    if (myColor == WHITE && square == E1) {
        // short castling (towards h1)
        if (board.whiteKingsideCastlingRights) {
            // check if f1 and g1 squares are empty
            if (board.chessboard[F1] == EMPTY && board.chessboard[G1] == EMPTY) {
                if (!board.isSquareAttacked(E1, BLACK) &&
                    !board.isSquareAttacked(F1, BLACK) &&
                    !board.isSquareAttacked(G1, BLACK)) {
                        Move move;
                        move.fromSquare = E1;
                        move.toSquare = G1;
                        move.movedPiece = W_KING;
                        move.capturedPiece = EMPTY;
                        move.castling = true; // castling flag
                        out.push_back(move);
                    }
            }
        }
        // long castling analogously
        if (board.whiteQueensideCastlingRights) {
            // must be empty (b1, c1, d1)
            if (board.chessboard[B1] == EMPTY && board.chessboard[C1] == EMPTY && board.chessboard[D1] == EMPTY) {
                if (!board.isSquareAttacked(E1, BLACK) &&
                    !board.isSquareAttacked(D1, BLACK) &&
                    !board.isSquareAttacked(C1, BLACK)) {
                    Move move;
                    move.fromSquare = E1;
                    move.toSquare = C1;
                    move.movedPiece = W_KING;
                    move.capturedPiece = EMPTY;
                    move.castling = true;
                    out.push_back(move);
                }
            }
        }
    }
    // black
    else if (myColor == BLACK && square == E8) {
        // short castling
        if (board.blackKingsideCastlingRights) {
            if (board.chessboard[F8] == EMPTY && board.chessboard[G8] == EMPTY) {
                if (!board.isSquareAttacked(E8, WHITE) &&
                    !board.isSquareAttacked(F8, WHITE) &&
                    !board.isSquareAttacked(G8, WHITE)) {
                    Move move;
                    move.fromSquare = E8;
                    move.toSquare = G8;
                    move.movedPiece = B_KING;
                    move.capturedPiece = EMPTY;
                    move.castling = true;
                    out.push_back(move);
                }
            }
        }
        // long castling
        if (board.blackQueensideCastlingRights) {
            if (board.chessboard[B8] == EMPTY && board.chessboard[C8] == EMPTY && board.chessboard[D8] == EMPTY) {
              if (!board.isSquareAttacked(E8, WHITE) &&
                  !board.isSquareAttacked(D8, WHITE) &&
                  !board.isSquareAttacked(C8, WHITE)) {
                    Move move;
                    move.fromSquare = E8;
                    move.toSquare = C8;
                    move.movedPiece = B_KING;
                    move.capturedPiece = EMPTY;
                    move.castling = true;
                    out.push_back(move);
                }
            }
        }
    }
}

void MoveGen::generatePawnMoves(int square, int myColor, std::vector<Move>& out) {
    // white
    if(myColor == WHITE){
        int forwardSquare = square + 8;
        if (forwardSquare < 64 && board.chessboard[forwardSquare] == EMPTY) {
            // promotion
            if (forwardSquare >= 56) { // promotion rank indices are 56 to 63 for white
                int promotion[4] = {W_QUEEN, W_BISHOP, W_ROOK, W_KNIGHT}; // possible promotions each as a separate move
                for( int i = 0; i < 4; i++){
                    Move move;
                    move.fromSquare = square;
                    move.toSquare = forwardSquare;
                    move.movedPiece = W_PAWN;
                    move.capturedPiece = EMPTY;
                    move.promotedPiece = promotion[i];
                    out.push_back(move);
                }
            }
            else{
                // regular move
                Move move;
                move.fromSquare = square;
                move.toSquare = forwardSquare;
                move.movedPiece = W_PAWN;
                move.capturedPiece = EMPTY;
                out.push_back(move);
                // initial double step move option
                if(square >= 8 && square <= 15){
                    int doubleMoveSquare = square + 16;
                    if(board.chessboard[doubleMoveSquare] == EMPTY){
                        Move doubleMove;
                        doubleMove.fromSquare = square;
                        doubleMove.toSquare = doubleMoveSquare;
                        doubleMove.movedPiece = W_PAWN;
                        doubleMove.capturedPiece = EMPTY;
                        doubleMove.promotedPiece = EMPTY;
                        out.push_back(doubleMove);
                    }
                }
            }
        }
        // capture diagonal left
        if(square % 8 != 0){
            int captureLeft = square + 7;
            if(board.chessboard[captureLeft] != EMPTY && getPieceColor(board.chessboard[captureLeft]) == BLACK){
                // possible promotion while capturing
                if(captureLeft >= 56){
                    int promotion[4] = {W_QUEEN, W_BISHOP, W_ROOK, W_KNIGHT};
                        for( int i = 0; i < 4; i++){
                            Move move;
                            move.fromSquare = square;
                            move.toSquare = captureLeft;
                            move.movedPiece = W_PAWN;
                            move.capturedPiece = board.chessboard[captureLeft];
                            move.promotedPiece = promotion[i];
                            out.push_back(move);
                        }
                }
                else{
                        Move move;
                        move.fromSquare = square;
                        move.toSquare = captureLeft;
                        move.movedPiece = W_PAWN;
                        move.capturedPiece = board.chessboard[captureLeft];
                        move.promotedPiece = EMPTY;
                        out.push_back(move);
                }
            }
        }
        // capture diagonal right (analogous to left)
        if(square % 8 != 7){
            int captureRight = square + 9;
            if (board.chessboard[captureRight] != EMPTY && getPieceColor(board.chessboard[captureRight]) == BLACK) {
                if (captureRight >= 56) {
                    int promotion[4] = { W_QUEEN, W_ROOK, W_BISHOP, W_KNIGHT };
                    for (int i = 0; i < 4; i++) {
                        Move move;
                        move.fromSquare = square;
                        move.toSquare = captureRight;
                        move.movedPiece = W_PAWN;
                        move.capturedPiece = board.chessboard[captureRight];
                        move.promotedPiece = promotion[i];
                        out.push_back(move);
                    }
                } else {
                    Move move;
                    move.fromSquare = square;
                    move.toSquare = captureRight;
                    move.movedPiece = W_PAWN;
                    move.capturedPiece = board.chessboard[captureRight];
                    move.promotedPiece = EMPTY;
                    out.push_back(move);
                }
            }
        }
        if (board.enPassantSquare != 64) { // enPassant square target (square behind the capturable pawn)
            // diagonal capture left
            if (square % 8 != 0 && (square + 7) == board.enPassantSquare) { // if not on the left edge and diagonal forward move lands on the enPassant square
                Move move;
                move.fromSquare = square;
                move.toSquare = board.enPassantSquare;
                move.movedPiece = W_PAWN;
                move.capturedPiece = B_PAWN;
                move.enPassant = true;
                out.push_back(move);
            }
            // diagonal capture right
            if (square % 8 != 7 && (square + 9) == board.enPassantSquare) { // analogously, just for the right side of the board
                Move move;
                move.fromSquare = square;
                move.toSquare = board.enPassantSquare;
                move.movedPiece = W_PAWN;
                move.capturedPiece = B_PAWN;
                move.enPassant = true;
                out.push_back(move);
            }
        }
    }
    // black analogously to white
    else if (myColor == BLACK) {
        int forwardSquare = square - 8;
        if (forwardSquare >= 0 && board.chessboard[forwardSquare] == EMPTY) {
            if (forwardSquare <= 7) {
                int promotion[4] = {B_QUEEN, B_BISHOP, B_ROOK, B_KNIGHT};
                for(int i = 0; i < 4; i++) {
                    Move move;
                    move.fromSquare = square;
                    move.toSquare = forwardSquare;
                    move.movedPiece = B_PAWN;
                    move.capturedPiece = EMPTY;
                    move.promotedPiece = promotion[i];
                    out.push_back(move);
                }
            }
            else {
                // regular move
                Move move;
                move.fromSquare = square;
                move.toSquare = forwardSquare;
                move.movedPiece = B_PAWN;
                move.capturedPiece = EMPTY;
                move.promotedPiece = EMPTY;
                out.push_back(move);

                // initial double step move (black pawns start on rank 7 with indices 48-55)
                if (square >= 48 && square <= 55) {
                    int doubleMoveSquare = square - 16;
                    if (board.chessboard[doubleMoveSquare] == EMPTY) {
                        Move doubleMove;
                        doubleMove.fromSquare = square;
                        doubleMove.toSquare = doubleMoveSquare;
                        doubleMove.movedPiece = B_PAWN;
                        doubleMove.capturedPiece = EMPTY;
                        doubleMove.promotedPiece = EMPTY;
                        out.push_back(doubleMove);
                    }
                }
            }
        }
        // diagonal capture LEFT for black
        if (square % 8 != 0) {
            int captureLeft = square - 9;
            if (board.chessboard[captureLeft] != EMPTY && getPieceColor(board.chessboard[captureLeft]) == WHITE) {
                if (captureLeft <= 7) {
                    int promotion[4] = {B_QUEEN, B_BISHOP, B_ROOK, B_KNIGHT};
                    for(int i = 0; i < 4; i++) {
                        Move move;
                        move.fromSquare = square;
                        move.toSquare = captureLeft;
                        move.movedPiece = B_PAWN;
                        move.capturedPiece = board.chessboard[captureLeft];
                        move.promotedPiece = promotion[i];
                        out.push_back(move);
                    }
                }
                else {
                    Move move;
                    move.fromSquare = square;
                    move.toSquare = captureLeft;
                    move.movedPiece = B_PAWN;
                    move.capturedPiece = board.chessboard[captureLeft];
                    move.promotedPiece = EMPTY;
                    out.push_back(move);
                }
            }
        }
        // diagonal capture RIGHT for black
        if (square % 8 != 7) {
            int captureRight = square - 7;
            if (board.chessboard[captureRight] != EMPTY && getPieceColor(board.chessboard[captureRight]) == WHITE) {
                if (captureRight <= 7) {
                    int promotion[4] = { B_QUEEN, B_ROOK, B_BISHOP, B_KNIGHT };
                    for (int i = 0; i < 4; i++) {
                        Move move;
                        move.fromSquare = square;
                        move.toSquare = captureRight;
                        move.movedPiece = B_PAWN;
                        move.capturedPiece = board.chessboard[captureRight];
                        move.promotedPiece = promotion[i];
                        out.push_back(move);
                    }
                } else {
                    Move move;
                    move.fromSquare = square;
                    move.toSquare = captureRight;
                    move.movedPiece = B_PAWN;
                    move.capturedPiece = board.chessboard[captureRight];
                    move.promotedPiece = EMPTY;
                    out.push_back(move);
                }
            }
        }
        if (board.enPassantSquare != 64) {
            // diagonal capture left (viewed from black's side)
            if (square % 8 != 0 && (square - 9) == board.enPassantSquare) {
                Move move;
                move.fromSquare = square;
                move.toSquare = board.enPassantSquare;
                move.movedPiece = B_PAWN;
                move.capturedPiece = W_PAWN;
                move.enPassant = true;
                out.push_back(move);
            }
            // diagonal capture right
            if (square % 8 != 7 && (square - 7) == board.enPassantSquare) {
                Move move;
                move.fromSquare = square;
                move.toSquare = board.enPassantSquare;
                move.movedPiece = B_PAWN;
                move.capturedPiece = W_PAWN;
                move.enPassant = true;
                out.push_back(move);
            }
        }
    }
}

void MoveGen::generateSlidingMoves(int startSquare, int myColor, int pieceType, std::vector<Move>& out) {
    // directions
    int directions[8] = {8, -8, 1, -1, 9, -9, 7, -7};

    int startIndex = 0;
    int endIndex = 8;

    if (pieceType == W_ROOK || pieceType == B_ROOK) {
        endIndex = 4; // rook
    }
    else if (pieceType == W_BISHOP || pieceType == B_BISHOP) {
        startIndex = 4;  // bishop
    }

    // if queen, do not limit directions
    for (int i = startIndex; i < endIndex; ++i) {
        int direction = directions[i];
        int currentSquare = startSquare;

        while(true){
            int targetSquare = currentSquare + direction;
            if (targetSquare < 0 || targetSquare > 63) break; // out of bounds

            int startX = currentSquare % 8;
            int startY = currentSquare / 8;
            int targetX = targetSquare % 8;
            int targetY = targetSquare / 8;

            if (std::abs(targetX - startX) > 1 || std::abs(targetY - startY) > 1) { // edge wrapping protection
                break;
            }

            int targetPiece = board.chessboard[targetSquare];

            if(targetPiece != EMPTY){
               if(getPieceColor(targetPiece) != myColor){
                Move move;
                    move.fromSquare = startSquare;
                    move.toSquare = targetSquare;
                    move.movedPiece = board.chessboard[startSquare];
                    move.capturedPiece = targetPiece;
                    out.push_back(move);
               }
               break;
            }
            Move move;
            move.fromSquare = startSquare;
            move.toSquare = targetSquare;
            move.movedPiece = board.chessboard[startSquare];
            move.capturedPiece = EMPTY;
            out.push_back(move);

            currentSquare = targetSquare;
        }
    }
}

void MoveGen::generateLegal(int color, std::vector<Move>& out) {
  out.clear();
  std::vector<Move> pseudoMoves;
  generatePseudoLegal(color, pseudoMoves);

  out.reserve(pseudoMoves.size()); // memory reservation

  int enemy = getOppositeColor(color);

  for(Move& m : pseudoMoves){
    board.makeMove(m);
    int kingSquare = board.findKing(color);
    if(!board.isSquareAttacked(kingSquare, enemy)){
        out.push_back(m);
    }
    board.unmakeMove(m);
  }
}
