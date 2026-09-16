#include "Search.h"
#include <algorithm>

Search::Search(Board& board, MoveGen& gen) : board(board), gen(gen) {}

static const int pieceValues[13] = {
    0,
    100, 320, 330, 500, 900, 0,
    100, 320, 330, 500, 900, 0
};
//values for postions on board
static const int pawnPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10,-20,-20, 10, 10,  5,
     5, -5,-10,  0,  0,-10, -5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5,  5, 10, 25, 25, 10,  5,  5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
     0,  0,  0,  0,  0,  0,  0,  0
};

static const int knightPST[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const int bishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int rookPST[64] = {
      0,  0,  0,  5,  5,  0,  0,  0,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
     -5,  0,  0,  0,  0,  0,  0, -5,
      5, 10, 10, 10, 10, 10, 10,  5,
      0,  0,  0,  0,  0,  0,  0,  0
};

static const int queenPST[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int kingPST[64] = {
     20, 30, 10,  0,  0, 10, 30, 20,
     20, 20,  0,  0,  0,  0, 20, 20,
    -10,-20,-20,-20,-20,-20,-20,-10,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30
};

static const int* pstArrays[13] = {
    nullptr,
    pawnPST, knightPST, bishopPST, rookPST, queenPST, kingPST,  // white (1-6)
    pawnPST, knightPST, bishopPST, rookPST, queenPST, kingPST   // black (7-12)
};

int Search::evaluatePosition(int color) const{
    int whiteScore = 0;
    int blackScore = 0;

    for(int i = 0; i < 64; i++){
        int piece = board.chessboard[i];
        if(piece == EMPTY) continue;

        int pieceColor = (piece >= W_PAWN && piece <= W_KING) ? WHITE : BLACK;
        int square = (pieceColor == WHITE) ? i : (i ^ 56);

        int value = pieceValues[piece] + pstArrays[piece][square];

        if(pieceColor == WHITE) {
            whiteScore += value;
        } else {
            blackScore += value;
        }
    }
    return (color == WHITE) ? (whiteScore - blackScore) : (blackScore - whiteScore);
}

int Search::quiescence(int color, int alpha, int beta){
    nodes++;

    if ((nodes & 2047) == 0) {
        if (isTimeUp()) return 0;
    }
    if (interrupted) return 0;

    int standPat = evaluatePosition(color);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    std::vector<Move> moves;
    gen.generateLegal(color, moves);

    std::vector<Move> captures;
    captures.reserve(moves.size());
    for (Move& m : moves) {
        if (m.capturedPiece != EMPTY) captures.push_back(m);
    }

    std::sort(captures.begin(), captures.end(), [this](const Move& a, const Move& b){
        return getMVVLVAMoveValue(a) > getMVVLVAMoveValue(b);
    });

    for (Move& m : captures) {
        board.makeMove(m);
        board.savePosition(getOppositeColor(color));

        int score = -quiescence(getOppositeColor(color), -beta, -alpha);

        board.undoPosition();
        board.unmakeMove(m);

        if (interrupted) break;

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

int Search::getMVVLVAMoveValue(const Move& m) const{
  if(m.capturedPiece == EMPTY) return 0;

  int victimValue = pieceValues[m.capturedPiece];
  int attackerValue = pieceValues[m.movedPiece];

  return victimValue * 100 - attackerValue;
}

bool Search::isTimeUp(){
if(interrupted) return true;
auto now = std::chrono::steady_clock::now();
long long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
if (elapsed >= timeLimitMs) {
        interrupted = true;
    }
    return interrupted;
}

int Search::negamax(int color, int depth, int ply, int alpha, int beta , Move* bestMoveOut){
 nodes++;

  if ((nodes & 2047) == 0) { //bit operation for optimization
        if (isTimeUp()) return 0;
    }
  if (interrupted) return 0;

 std::vector<Move> moves;

  if (board.is50MoveDraw() || board.isThreefoldRepetition()) {
        return 0;
   }

 gen.generateLegal(color, moves);

 if(moves.empty()){
    int enemy = getOppositeColor(color);
    int kingSquare = board.findKing(color);
    bool check = board.isSquareAttacked(kingSquare, enemy);

    if(check) return -(mateValue - ply);
    else return 0;
 }



 if(depth == 0){
    return quiescence(color, alpha, beta);
 }

 std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b){ //using a lambda as the sort key
  return getMVVLVAMoveValue(a) > getMVVLVAMoveValue(b);
 });

 int best = -inf;

 for(Move& m : moves){
    board.makeMove(m);
    board.savePosition(getOppositeColor(color));

    int score = - negamax(getOppositeColor(color), depth - 1, ply + 1, -beta, -alpha);

    board.undoPosition();
    board.unmakeMove(m);

    if(interrupted) break;

    if(score > best){
      best = score;
      if(bestMoveOut != nullptr){
        *bestMoveOut = m;
      }
    }
    if(score > alpha) alpha = score;
    if(alpha >= beta) break;
 }
 return best;
}

Move Search::findBestMove(int color, int depth) {
    interrupted = false;
    timeLimitMs = 1LL << 40;
    startTime = std::chrono::steady_clock::now();

    Move bestMove{};
    int score = negamax(color, depth, 0, -inf, inf, &bestMove);

    std::cout << "info depth " << depth << " score cp " << score
              << " nodes " << nodes << std::endl;

    return bestMove;
}

Move Search::searchTimedMove(int color, long long timeLimitMsParam) {
    startTime = std::chrono::steady_clock::now();
    timeLimitMs = timeLimitMsParam;
    interrupted = false;
    nodes = 0;

    Move overallBest{};
    int depth = 1;

    while (true) {
        Move bestAtCurrentDepth{};
        int score = negamax(color, depth, 0, -inf, inf, &bestAtCurrentDepth);

        if (interrupted) break;

        overallBest = bestAtCurrentDepth;

        auto now = std::chrono::steady_clock::now();
        long long timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
        long long nps = (timeMs > 0) ? (nodes * 1000 / timeMs) : 0;

        std::cout << "info depth " << depth
                  << " score cp " << score
                  << " nodes " << nodes
                  << " nps " << nps
                  << " time " << timeMs
                  << std::endl;

        depth++;
        if (depth > 60) break;
    }

    return overallBest;
}
