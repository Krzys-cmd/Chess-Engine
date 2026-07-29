#include "UCI.h"
#include <iostream>
#include <sstream>
#include <vector>

UCI::UCI(Board& board, MoveGen& gen, Search& ser)
    : board(board), gen(gen), ser(ser), kolorNaRuchu(WHITE) {}


int UCI::poleZTekstu(const std::string& s) {
    int kolumna = s[0] - 'a';
    int wiersz = s[1] - '1';
    return wiersz * 8 + kolumna;
}

std::string UCI::poleNaTekst(int pole) {
    std::string wynik = "";
    wynik += char('a' + (pole % 8));
    wynik += char('1' + (pole / 8));
    return wynik;
}

std::string UCI::ruchNaTekst(const Move& m) {
    std::string s = poleNaTekst(m.zPola) + poleNaTekst(m.naPole);
    if (m.promotedPiece != EMPTY) {
        if (m.promotedPiece == W_QUEEN  || m.promotedPiece == B_QUEEN)  s += "q";
        else if (m.promotedPiece == W_ROOK   || m.promotedPiece == B_ROOK)   s += "r";
        else if (m.promotedPiece == W_BISHOP || m.promotedPiece == B_BISHOP) s += "b";
        else if (m.promotedPiece == W_KNIGHT || m.promotedPiece == B_KNIGHT) s += "n";
    }
    return s;
}

bool UCI::znajdzRuch(const std::string& tekst, Move& wynik) {
    std::vector<Move> legalne;
    gen.generateLegal(kolorNaRuchu, legalne);

    int zPola = poleZTekstu(tekst.substr(0, 2));
    int naPole = poleZTekstu(tekst.substr(2, 2));

    int promocja = EMPTY;
    if (tekst.size() == 5) {
        char p = tekst[4];
        bool biale = (kolorNaRuchu == WHITE);
        if (p == 'q') promocja = biale ? W_QUEEN : B_QUEEN;
        else if (p == 'r') promocja = biale ? W_ROOK : B_ROOK;
        else if (p == 'b') promocja = biale ? W_BISHOP : B_BISHOP;
        else if (p == 'n') promocja = biale ? W_KNIGHT : B_KNIGHT;
    }

    for (Move& m : legalne) {
        if (m.zPola == zPola && m.naPole == naPole && m.promotedPiece == promocja) {
            wynik = m;
            return true;
        }
    }
    return false;
}

void UCI::obslugaUci() {
    std::cout << "id name MojSilnik\n";
    std::cout << "id author Krzys\n";
    std::cout << "uciok\n";
}

void UCI::obslugaIsReady() {
    std::cout << "readyok\n";
}

void UCI::obslugaUciNewGame() {
    board = Board();
}

void UCI::obslugaPosition(std::istringstream& iss) {
    std::string typ;
    iss >> typ;

    board.historiaPozycji.clear();
    board.polRuchow50 = 0;

    if (typ == "startpos") {
        board.StartPostions();
        kolorNaRuchu = WHITE;

        std::string kolejnyToken;
        iss >> kolejnyToken;

        board.zapiszPozycje(kolorNaRuchu);

        if (kolejnyToken == "moves") {
            std::string ruchTekst;
            while (iss >> ruchTekst) {
                Move m;
                if (znajdzRuch(ruchTekst, m)) {
                    board.MakeMove(m);
                    kolorNaRuchu = przeciwnyKolor(kolorNaRuchu);
                    board.zapiszPozycje(kolorNaRuchu);
                }
            }
        }
    }
    else if (typ == "fen") {
        std::vector<std::string> czesciFen;
        std::string token;
        bool trafionoMoves = false;

        while (iss >> token) {
            if (token == "moves") {
                trafionoMoves = true;
                break;
            }
            czesciFen.push_back(token);
        }

        std::string fen;
        for (size_t i = 0; i < czesciFen.size(); i++) {
            fen += czesciFen[i];
            if (i + 1 < czesciFen.size()) fen += " ";
        }

        kolorNaRuchu = board.wczytajFEN(fen);
        board.zapiszPozycje(kolorNaRuchu);

        if (trafionoMoves) {
            std::string ruchTekst;
            while (iss >> ruchTekst) {
                Move m;
                if (znajdzRuch(ruchTekst, m)) {
                    board.MakeMove(m);
                    kolorNaRuchu = przeciwnyKolor(kolorNaRuchu);
                    board.zapiszPozycje(kolorNaRuchu);
                }
            }
        }
    }
}

void UCI::obslugaGo(std::istringstream& iss) {
    std::string token;
    long long wtime = -1, btime = -1, movetime = -1;
    int stalaGlebokosc = -1;

    while (iss >> token) {
        if (token == "wtime") iss >> wtime;
        else if (token == "btime") iss >> btime;
        else if (token == "movetime") iss >> movetime;
        else if (token == "depth") iss >> stalaGlebokosc;
    }

    Move najlepszy;

    if (stalaGlebokosc > 0) {
        najlepszy = ser.szukajNajlepszegoRuchu(kolorNaRuchu, stalaGlebokosc);
    }
    else if (movetime > 0) {
        najlepszy = ser.szukajRuchWCzasie(kolorNaRuchu, movetime);
    }
    else {
        long long mojCzas = (kolorNaRuchu == WHITE) ? wtime : btime;
        if (mojCzas <= 0) mojCzas = 5000;

        long long przydzielonyCzas = mojCzas / 30;
        if (przydzielonyCzas < 50) przydzielonyCzas = 50;

        najlepszy = ser.szukajRuchWCzasie(kolorNaRuchu, przydzielonyCzas);
    }

    std::cout << "bestmove " << ruchNaTekst(najlepszy) << "\n";
}
void UCI::uruchomPetle() {
    std::string linia;
    while (std::getline(std::cin, linia)) {
        std::istringstream iss(linia);
        std::string komenda;
        iss >> komenda;

        if (komenda == "uci") obslugaUci();
        else if (komenda == "isready") obslugaIsReady();
        else if (komenda == "ucinewgame") obslugaUciNewGame();
        else if (komenda == "position") obslugaPosition(iss);
        else if (komenda == "go") obslugaGo(iss);
        else if (komenda == "quit") break;
        // inne komendy na razie ignorujemy
    }
}
