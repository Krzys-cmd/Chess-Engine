#pragma once
#include "Board.h"
#include "MoveGen.h"
#include <chrono>
#include <iostream>

const int inf = 1000000;
const int MatWartosc = 100000;

class Search {
 public:
    Search(Board& board, MoveGen& gen);

    Move szukajNajlepszegoRuchu(int kolor, int depth);
    Move szukajRuchWCzasie(int kolor, long long limitCzasuMs);

    int negamax(int kolor, int depth, int polRuch, int alpha, int beta , Move* najlepszyRuchOut = nullptr);
    long long policzWezly() const { return wezly; }
 private:
    Board& board;
    MoveGen& gen;
    long long wezly = 0;

    int ocenPozycje(int kolor) const;

    std::chrono::steady_clock::time_point czasStartu;
    long long limitCzasuMs = 0;
    bool przerwane = false;
    bool minalCzas();

};
