#include "UCI.h"
#include <iostream>
#include <sstream>
#include <vector>

UCI::UCI(Board& board, MoveGen& gen, Search& search)
    : board(board), gen(gen), search(search), activeColor(WHITE) {}


int UCI::parseSquare(const std::string& s) {
    int file = s[0] - 'a';
    int rank = s[1] - '1';
    return rank * 8 + file;
}

std::string UCI::squareToString(int square) {
    std::string result = "";
    result += char('a' + (square % 8));
    result += char('1' + (square / 8));
    return result;
}

std::string UCI::moveToString(const Move& m) {
    std::string s = squareToString(m.fromSquare) + squareToString(m.toSquare);
    if (m.promotedPiece != EMPTY) {
        if (m.promotedPiece == W_QUEEN  || m.promotedPiece == B_QUEEN)  s += "q";
        else if (m.promotedPiece == W_ROOK   || m.promotedPiece == B_ROOK)   s += "r";
        else if (m.promotedPiece == W_BISHOP || m.promotedPiece == B_BISHOP) s += "b";
        else if (m.promotedPiece == W_KNIGHT || m.promotedPiece == B_KNIGHT) s += "n";
    }
    return s;
}

bool UCI::findMove(const std::string& text, Move& result) {
    std::vector<Move> legalMoves;
    gen.generateLegal(activeColor, legalMoves);

    int fromSquare = parseSquare(text.substr(0, 2));
    int toSquare = parseSquare(text.substr(2, 2));

    int promotion = EMPTY;
    if (text.size() == 5) {
        char p = text[4];
        bool isWhite = (activeColor == WHITE);
        if (p == 'q') promotion = isWhite ? W_QUEEN : B_QUEEN;
        else if (p == 'r') promotion = isWhite ? W_ROOK : B_ROOK;
        else if (p == 'b') promotion = isWhite ? W_BISHOP : B_BISHOP;
        else if (p == 'n') promotion = isWhite ? W_KNIGHT : B_KNIGHT;
    }

    for (Move& m : legalMoves) {
        if (m.fromSquare == fromSquare && m.toSquare == toSquare && m.promotedPiece == promotion) {
            result = m;
            return true;
        }
    }
    return false;
}

void UCI::handleUci() {
    std::cout << "id name MojSilnik\n";
    std::cout << "id author Krzysztof Glomb\n";
    std::cout << "uciok\n";
}

void UCI::handleIsReady() {
    std::cout << "readyok\n";
}

void UCI::handleUciNewGame() {
    board = Board();
}

void UCI::handlePosition(std::istringstream& iss) {
    std::string type;
    iss >> type;

    board.positionHistory.clear();
    board.halfmoveClock = 0;

    if (type == "startpos") {
        board.startPositions();
        activeColor = WHITE;

        std::string nextToken;
        iss >> nextToken;

        board.savePosition(activeColor);

        if (nextToken == "moves") {
            std::string moveText;
            while (iss >> moveText) {
                Move m;
                if (findMove(moveText, m)) {
                    board.makeMove(m);
                    activeColor = getOppositeColor(activeColor);
                    board.savePosition(activeColor);
                }
            }
        }
    }
    else if (type == "fen") {
        std::vector<std::string> fenParts;
        std::string token;
        bool movesHit = false;

        while (iss >> token) {
            if (token == "moves") {
                movesHit = true;
                break;
            }
            fenParts.push_back(token);
        }

        std::string fen;
        for (size_t i = 0; i < fenParts.size(); i++) {
            fen += fenParts[i];
            if (i + 1 < fenParts.size()) fen += " ";
        }

        activeColor = board.loadFEN(fen);
        board.savePosition(activeColor);

        if (movesHit) {
            std::string moveText;
            while (iss >> moveText) {
                Move m;
                if (findMove(moveText, m)) {
                    board.makeMove(m);
                    activeColor = getOppositeColor(activeColor);
                    board.savePosition(activeColor);
                }
            }
        }
    }
}

void UCI::handleGo(std::istringstream& iss) {
    std::string token;
    long long wtime = -1, btime = -1, movetime = -1;
    int fixedDepth = -1;

    while (iss >> token) {
        if (token == "wtime") iss >> wtime;
        else if (token == "btime") iss >> btime;
        else if (token == "movetime") iss >> movetime;
        else if (token == "depth") iss >> fixedDepth;
    }

    Move bestMove;

    if (fixedDepth > 0) {
        bestMove = search.findBestMove(activeColor, fixedDepth);
    }
    else if (movetime > 0) {
        bestMove = search.searchTimedMove(activeColor, movetime);
    }
    else {
        long long myTime = (activeColor == WHITE) ? wtime : btime;
        if (myTime <= 0) myTime = 5000;

        long long allocatedTime = myTime / 30;
        if (allocatedTime < 50) allocatedTime = 50;

        bestMove = search.searchTimedMove(activeColor, allocatedTime);
    }

    std::cout << "bestmove " << moveToString(bestMove) << "\n";
}

void UCI::runLoop() {
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "uci") handleUci();
        else if (command == "isready") handleIsReady();
        else if (command == "ucinewgame") handleUciNewGame();
        else if (command == "position") handlePosition(iss);
        else if (command == "go") handleGo(iss);
        else if (command == "quit") break;

    }
}
