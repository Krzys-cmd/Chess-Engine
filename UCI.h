#pragma once
#include <string>
#include "Board.h"
#include "MoveGen.h"
#include "Search.h"

class UCI{
public:
    UCI(Board& board, MoveGen& gen, Search& ser);

    void uruchomPetle(); //glowna petla wczytuajca komendy
private:
    Board& board;
    MoveGen& gen;
    Search& ser;

    int kolorNaRuchu;

    void obslugaUci();
    void obslugaIsReady();
    void obslugaUciNewGame();
    void obslugaPosition(std::istringstream& iss);
    void obslugaGo(std::istringstream& iss);

    int poleZTekstu(const std::string& s);
    std::string poleNaTekst(int pole);
    std::string ruchNaTekst(const Move& m);
    bool znajdzRuch(const std::string& tekst, Move& wynik);
};

