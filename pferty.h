#pragma once
#include "Board.h"
#include "MoveGen.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <iomanip>


//knots count
uint64_t perft(int depth, Board& board, MoveGen& moveGen, int color) {
    if (depth == 0) return 1ULL;

    uint64_t nodes = 0;

    std::vector<Move> legalMoves;
    moveGen.generateLegal(color, legalMoves);

    for (Move& move : legalMoves) {
        board.makeMove(move);
        nodes += perft(depth - 1, board, moveGen, getOppositeColor(color));
        board.unmakeMove(move);
    }

    return nodes;
}

//deepr testing
void runPerftTest(int maxDepth, Board& board, MoveGen& moveGen, int color) {
    for (int d = 1; d <= maxDepth; ++d) {
        auto start = std::chrono::high_resolution_clock::now();

        uint64_t nodes = perft(d, board, moveGen, color);

        auto end = std::chrono::high_resolution_clock::now();
        double seconds = std::chrono::duration<double>(end - start).count();

        //time in sec
        std::cout << "Perft(" << d << ") = " << nodes
                  << "    (czas: " << std::fixed << std::setprecision(4) << seconds << " s)\n";
    }
}

int Board::loadFEN(const std::string& fen) {
    //reset of chessboard and flags
    for (int i = 0; i < 64; i++) chessboard[i] = EMPTY;
    whiteKingsideCastlingRights = false;
    whiteQueensideCastlingRights = false;
    blackKingsideCastlingRights = false;
    blackQueensideCastlingRights = false;
    enPassantSquare = 64;

    std::istringstream ss(fen);
    std::string piecePlacement, activeColor, castlingRights, enPassantTarget;
    ss >> piecePlacement >> activeColor >> castlingRights >> enPassantTarget;

    //fen piece placement
    int rank = 7; //8 row (7 inedx)
    int file = 0; //a col (index 0)

    for (char c : piecePlacement) {
        if (c == '/') {
            rank--;      //lower row
            file = 0;    //col reset
        }
        else if (isdigit(c)) {
            file += (c - '0');
        }
        else {
            int square = rank * 8 + file;
            int piece = EMPTY;

            //letters to piece
            switch (c) {
                case 'P': piece = W_PAWN; break;
                case 'N': piece = W_KNIGHT; break;
                case 'B': piece = W_BISHOP; break;
                case 'R': piece = W_ROOK; break;
                case 'Q': piece = W_QUEEN; break;
                case 'K': piece = W_KING; break;
                case 'p': piece = B_PAWN; break;
                case 'n': piece = B_KNIGHT; break;
                case 'b': piece = B_BISHOP; break;
                case 'r': piece = B_ROOK; break;
                case 'q': piece = B_QUEEN; break;
                case 'k': piece = B_KING; break;
            }

            if (square >= 0 && square < 64) {
                chessboard[square] = piece;
            }
            file++;
        }
    }

    if (castlingRights != "-") {
        for (char character : castlingRights) {
            if (character == 'K') whiteKingsideCastlingRights = true;
            else if (character == 'Q') whiteQueensideCastlingRights = true;
            else if (character == 'k') blackKingsideCastlingRights = true;
            else if (character == 'q') blackQueensideCastlingRights = true;
        }
    }

    if (enPassantTarget != "-") {
        int epFile = enPassantTarget[0] - 'a';
        int epRank = enPassantTarget[1] - '1';
        enPassantSquare = epRank * 8 + epFile;
    }

    return (activeColor == "w") ? WHITE : BLACK;
}
