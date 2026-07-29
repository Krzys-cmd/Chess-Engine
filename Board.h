#pragma once
#include "types.h"
#include "string"
#include <vector>

struct Move {
    int zPola;
    int naPole;
    int ktoraFigura;
    int ktoraFiguraZbita;

    //flagi specjalne
    bool Castling  = false; //roszada
    bool EnPassant = false; //bicie w locie
    int promotedPiece = EMPTY; // promocja na jaka figure (EMPTY) brak promocji

    int stareEnPassantSquare = 64;

    bool poprzedniaBialaKrotka  = false;
    bool poprzedniaBialaDluga   = false;
    bool poprzedniaCzarnaKrotka = false;
    bool poprzedniaCzarnaDluga  = false;

    int poprzedniPolRuchow50 = 0;
};

struct PozycjaZapis {
    int szachownica[64];
    int kolorNaRuchu;
    bool bK, bQ, cK, cQ;
    int enPassant;

    bool rownaSie(const PozycjaZapis& inna) const {
        for (int i = 0; i < 64; i++) {
            if (szachownica[i] != inna.szachownica[i]) return false;
        }
        return kolorNaRuchu == inna.kolorNaRuchu &&
               bK == inna.bK && bQ == inna.bQ &&
               cK == inna.cK && cQ == inna.cQ &&
               enPassant == inna.enPassant;
    }
};

class Board {
public:

    int szachownica[64];
    int enPassantSquare = 64;

    //flagi roszady
    bool prawaBialeKrotka;
    bool prawaBialeDluga;
    bool prawaCzarneKrotka;
    bool prawaCzarneDluga;

    int polRuchow50 = 0;
    std::vector<PozycjaZapis> historiaPozycji;

    Board();

    int wczytajFEN(const std::string& fen);//pomocnicza funckja
    int znajdzKrola(int kolor) const;
    void StartPostions();//pozycja stratowa
    void printBoard() const; //rysowanie tabeli ANSI
    void UnmakeMove(Move ruch);
    void MakeMove(Move& ruch);
    bool czyPoleJestAtakowane(int pole, int kolorAtakujacego) const;

    void zapiszPozycje(int kolorNaRuchu);
    void wycofajPozycje();
    bool czyPowtorzenieTrzykrotne() const;
    bool czyRemis50Ruchow() const { return polRuchow50 >= 100; }

};
