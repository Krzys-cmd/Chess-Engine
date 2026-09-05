<h3 align="center">Chess Engine (C++)</h3>

<p align="center">
A chess engine written from scratch in <b>C++23</b>, based on a flat board representation (64-square array, so-called mailbox) with boundary validation via x/y coordinates, and a move generator compliant with full game rules - including castling, en passant, and promotion.
</p>

##  Key Features

| Feature | Description |
|---|---|
| **Board Representation** | A one-dimensional `int[64]` array (A1-H8 indices) storing piece states, featuring a full set of game state flags: castling rights (separate for both sides and flanks) and the en passant target square (`enPassantSquare`). |
| **Pseudo-Legal Move Generator** | Separate functions generating moves for each piece type - knights and kings via index offset arrays; bishops/rooks/queens by iteratively "sliding" in 8 directions until hitting an obstacle or board edge. |
| **Legal Move Filter** | Verification of every pseudo-legal move using the *make-check-unmake* method: the move is played on the board, own king's exposure to check is evaluated, and then the move is taken back - this eliminates moves that expose the king (pins). The generator writes results to an externally provided vector (`out`-parameter) instead of returning a new copy of the list on each call, eliminating redundant memory allocations in the hot search path. |
| **Attacked Square Detection** | The `isSquareAttacked` function checks if a given square is threatened by any enemy piece - utilizing separate logic for knights, kings, pawns (asymmetrical capture direction for white/black), and sliding pieces (bishop/rook/queen) stopping at the first encountered obstacle. |
| **Full Special Move Support** | Castling (kingside and queenside, with empty square and king transit path attack verification), en passant with correct target square calculation based on the last two-square pawn advance, and pawn promotion to all four minor/major pieces. |
| **Move Reversibility (Make/Unmake)** | Every move carries the full context required to revert the state: the captured piece, previous castling rights, and the previous en passant square - this allows searching the game tree without copying the entire board array for every move. |
| **Game Tree Search (Negamax + Alpha-Beta)** | Recursive variant search with branch pruning (alpha-beta pruning), scaling mate values by depth, and draw detection (50-move rule, three-fold repetition). |
| **Iterative Deepening + Time Management** | The engine deepens its search progressively (1, 2, 3...), constantly publishing the best move found so far - ensuring it always has a reasonable move ready even if time runs out during a deeper iteration. The time limit is checked every 2048 nodes (bitwise optimization) without adding unnecessary overhead to every iteration. |
| **UCI Protocol** | Full support for basic **Universal Chess Interface** commands (`uci`, `isready`, `position`, `go`, `ucinewgame`) - the engine connects directly to external GUIs (Arena, CuteChess) and online platforms supporting UCI engines. |
| **FEN Support** | Loading arbitrary positions from FEN notation (`loadFEN`), which allows testing the engine on specific tactical scenarios rather than just from the starting position. |
| **Draw Detection by Repetition** | After every move, the engine saves a board state snapshot (`savePosition`) to history - comparing subsequent snapshots allows it to detect a three-fold repetition of the exact same position (`isThreefoldRepetition`), independent of the 50-move rule counter. |
| **Move Generator Tests (Perft)** | The generator's correctness is verified using the **perft** algorithm - counting all game tree nodes up to a specified depth and comparing them against reference values. This is a standard method for catching bugs in special move logic (castling, en passant, promotion) that remain invisible during standard test play. |
---

## Architecture and Project Logic

The project emphasizes a clear separation of concerns between the game state representation (`Board`) and the move generation logic (`MoveGen`). This facilitates future expansion with game tree search algorithms (Minimax/Negamax) without interfering with the engine's core.

### Move Representation and Reversible State

The `Move` structure stores not only the starting and target squares but also the full context required to losslessly revert operations on the board. This is crucial for *depth-first* search algorithms to avoid allocating a new copy of the board at each recursion level:

```cpp
struct Move {
    int fromSquare, toSquare;
    int movedPiece, capturedPiece;
    bool isCastling, isEnPassant;
    int promotedPiece;
    int previousEnPassantSquare;
    int previousRule50;
    bool prevWhiteKingside, prevWhiteQueenside;
    bool prevBlackKingside, prevBlackQueenside;
};
```

Because of this, `makeMove` and `unmakeMove` operate in constant time relative to the board size, avoiding the overhead of copying a 64-square array at every node in the search tree.

### En Passant Detection

The en passant square is calculated dynamically after every two-square pawn advance. It is not a static flag but an actively refreshed target square valid for only one half-move (ply):

```cpp
if (move.movedPiece == W_PAWN && (move.toSquare - move.fromSquare == 16))
    enPassantSquare = move.fromSquare + 8;
```

The pawn move generator checks this square independently of standard diagonal captures, correctly handling one of the most frequently misimplemented moves in chess engines.

### Legality Verification via Simulation (Make-Check-Unmake)

Instead of maintaining a separate, costly structure for detecting pinned pieces, the engine verifies the legality of each move using a brute-force approach. It plays the move, checks if its own king is attacked, and then reverses it:

```cpp
board.makeMove(m);
int kingSquare = board.findKing(color);
if (!board.isSquareAttacked(kingSquare, enemyColor))
    legalMoves.push_back(m);
board.unmakeMove(m);
```

While less performant than bitboard-based approaches with precalculated attack rays, this solution is conceptually much simpler and far less prone to errors during initial implementation. Both `generatePseudoLegal` and `generateLegal` take the target vector as an output parameter (`std::vector<Move>& out`) rather than returning it. This allows the caller (e.g., `Search`) to reuse the same memory buffer repeatedly, skipping new allocations at every search tree node.

### Draw Detection by Repetition

After every executed move, the engine saves a full snapshot of the game state (piece placement, castling rights, en passant square, side to move) to the history array (`positionHistory`). Detecting a draw by threefold repetition involves comparing the current snapshot with all previous ones:

```cpp
int count = 0;
for (const PositionSnapshot& p : positionHistory) {
    if (p.equals(current)) count++;
}
return count >= 3;
```

Saving and reverting the snapshot (`savePosition` / `revertPosition`) is symmetric to `makeMove` / `unmakeMove`. This ensures the history remains consistent even during deep search recursion, where the same nodes are visited and reverted multiple times.

### Attacked Square Detection (Sliding Pieces)

For sliding pieces (bishop, rook, queen), the engine iterates in 8 possible directions using an offset array, stopping at the first encountered piece. It distinguishes between "straight" directions (indices 0-3: rook/queen) and "diagonal" directions (indices 4-7: bishop/queen):

```cpp
int directions[8] = { 8, -8, 1, -1, 9, -9, 7, -7 };
```

Protection against the ray "wrapping around" the board edge is implemented by comparing the x/y coordinates of consecutive squares, rather than relying solely on the raw array index.
### Negamax with Alpha-Beta Pruning

Game tree traversal is based on the **Negamax** algorithm—a simplified version of Minimax that leverages the zero-sum nature of the game (`eval(player) = -eval(opponent)`)—enhanced with the pruning of branches that cannot affect the final outcome:

```cpp
int score = -negamax(getOppositeColor(color), depth - 1, ply + 1, -beta, -alpha);
...
if (score > alpha) alpha = score;
if (alpha >= beta) break; // branch pruning
```

The checkmate value is scaled by the search depth (`ply`), ensuring the engine always prefers the **fastest possible mate** rather than just any distant game-ending sequence:

```cpp
if (inCheck) return -(MATE_VALUE - ply);
```
### Iterative Deepening and Safe Time Management

Instead of immediately searching to a fixed depth, the engine deepens its analysis iteratively (`depth = 1, 2, 3, ...`), retaining the best move found after every **fully completed** iteration. If the time limit is exceeded during a deeper iteration, its incomplete result is discarded, and the engine returns the move from the last completed depth. This guarantees the bot never plays a move based on partial analysis:

```cpp
if (isInterrupted) break;
globalBestMove = bestMoveThisDepth;
```

Clock checks occur infrequently (every 2048 nodes, via `nodes & 2047`) rather than at every node. This eliminates the unnecessary overhead of calling `steady_clock::now()` on the engine's hottest execution path.

### UCI Protocol - External Communication

The engine implements a command loop compliant with the UCI standard. It translates textual commands (e.g., `position startpos moves e2e4 e7e5`) into internal `Board`/`MoveGen` operations and converts calculated moves back into the algebraic notation required by the protocol:

```cpp
std::string UCI::moveToText(const Move& m) {
    std::string s = squareToText(m.fromSquare) + squareToText(m.toSquare);
    if (m.promotedPiece != EMPTY) s += /* promotion letter */;
    return s;
}
```

Handling the `go` command automatically selects the engine's operation mode based on the received parameters: fixed depth (`depth`), fixed move time (`movetime`), or remaining game time allocation (`wtime`/`btime`, with a minimum 50 ms buffer for extreme time-trouble scenarios).

### Entry Point (`main.cpp`)

Running the engine comes down to assembling four main components into a single, operational unit: `Board` as the source of truth for the game state, `MoveGen` and `Search` operating on it via reference, and `UCI` tying everything together into a command loop driven via standard input:

```cpp
Board board;
MoveGen gen(board);
Search search(board, gen);
UCI uci(board, gen, search);
uci.runLoop();
```

This setup (using references rather than copies) ensures all components consistently operate on the exact same board state, preventing desynchronization risks, as mentioned in the `MoveGen` architecture description.

---
## Class Structure

* **`Board`**: Stores the board state (`chessboard[64]`), castling rights, en passant square, position history (`positionHistory`), and the half-move clock for the 50-move rule (`rule50Clock`). It is responsible for initializing the starting position and loading FENs, making and unmaking moves (`makeMove` / `unmakeMove`), detecting attacked squares (`isSquareAttacked`), locating the king (`findKing`), and detecting draws by saving and comparing position snapshots (`savePosition`, `revertPosition`, `isThreefoldRepetition`).
* **`MoveGen`**: A move generator operating on a reference to `Board`. It splits logic into separate methods per piece type (`generateKnightMoves`, `generateKingMoves`, `generatePawnMoves`, `generateSlidingMoves`), writing generated moves directly to the provided output vector. It also includes a `generateLegal` filter that weeds out moves leaving the king in check.
* **`Search`**: The engine component that traverses the game tree using Negamax with alpha-beta pruning. It exposes two operational modes: fixed-depth search (`searchBestMove`) and iterative deepening with time limits (`searchMoveWithTime`). It features a static material evaluation function (`evaluatePosition`) and a mechanism to safely interrupt calculations when the time limit is reached (`isTimeUp`).
* **`UCI`**: The communication layer implementing the Universal Chess Interface protocol. It parses textual commands from standard input, translates move notations (e.g., `e2e4`) into `Move` objects, and binds `Board`, `MoveGen`, and `Search` together into an externally controllable game loop.
* **`Move`**: A data structure describing a single move alongside the full context required to revert it (captured piece, previous castling rights, previous en passant square).
---

## Project Status

The engine features a fully verified (via **perft** tests) board representation and move generator, a working Negamax search with alpha-beta pruning, and full UCI protocol integration—it is already capable of playing games in external GUIs. Future development milestones focus on playing strength:

- [ ] Piece-Square Tables (PST) - positional piece evaluation, beyond pure material
- [ ] Transposition Table (Zobrist hashing)
- [ ] Null-Move Pruning and Late Move Reduction (LMR)
