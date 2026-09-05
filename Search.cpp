#include "Search.h"
#include <algorithm>

Search::Search(Board& board, MoveGen& gen) : board(board), gen(gen) {}

static const int pieceValues[13] = {
    0,
    100, 320, 330, 500, 900, 0,
    100, 320, 330, 500, 900, 0
};

int Search::evaluatePosition(int color) const{
 int sum = 0;
 for(int i = 0; i < 64; i++){
    int piece = board.chessboard[i];
    if(piece == EMPTY) continue;

    int value = pieceValues[piece];
    int pieceColor = (piece >= W_PAWN && piece <= W_KING) ? WHITE : BLACK;

    if(pieceColor == color) sum += value;
    else sum -= value;
 }
 return sum;
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
    return evaluatePosition(color);
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
