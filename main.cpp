#include <iostream>
#include "Board.h"
#include "UCI.h"
#include "MoveGen.h"
#include "pferty.h"

int main()
{
    Board board;
    MoveGen gen(board);
    Search search(board, gen);
    UCI uci(board, gen, search);

    uci.uruchomPetle();

    return 0;
}
