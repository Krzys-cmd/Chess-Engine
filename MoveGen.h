#pragma once
#include <vector>
#include "Board.h"
#include "types.h"



class MoveGen {
public:
    MoveGen(Board& board);

    void generatePseudoLegal(int kolorFiguryRuszanej, std::vector<Move>& out);
    void generateLegal(int kolor, std::vector<Move>& out);//filr legalnych ruchow

private:
    Board& board; //orginal szachownicy

    //funkcje dla figur
    void generujRuchyKnight(int pole, int mojKolor, std::vector<Move>& out);
    void generujRuchyKing(int pole, int mojKolor, std::vector<Move>& out);
    void generujRuchyPawn(int pole, int mojKolor, std::vector<Move>& out);
    void generujRuchySliding(int pole, int mojKolorr, int rodzajFigury, std::vector<Move>& out);//goniec wieza hetmamn

    int kolorFigury(int piece) const;
};
