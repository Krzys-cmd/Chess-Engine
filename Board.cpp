#include "Board.h"
#include <iostream>

Board::Board(){
//ustaniwenie claej tabeli na zero
     for(int i = 0; i < 64; i++){
        szachownica[i] = 0;
     }
}

void Board::StartPostions(){ //startowy uklad szachownicy
    for (int i = 0; i < 64; ++i) {
        szachownica[i] = EMPTY;
    }

    //biale figury
    szachownica[A1] = W_ROOK;
    szachownica[B1] = W_KNIGHT;
    szachownica[C1] = W_BISHOP;
    szachownica[D1] = W_QUEEN;
    szachownica[E1] = W_KING;
    szachownica[F1] = W_BISHOP;
    szachownica[G1] = W_KNIGHT;
    szachownica[H1] = W_ROOK;

    //biale piony
    for (int i = A2; i <= H2; ++i) {
        szachownica[i] = W_PAWN;
    }
    //czarne piony
    for (int i = A7; i <= H7; ++i) {
        szachownica[i] = B_PAWN;
    }

    //czarne figury
    szachownica[A8] = B_ROOK;
    szachownica[B8] = B_KNIGHT;
    szachownica[C8] = B_BISHOP;
    szachownica[D8] = B_QUEEN;
    szachownica[E8] = B_KING;
    szachownica[F8] = B_BISHOP;
    szachownica[G8] = B_KNIGHT;
    szachownica[H8] = B_ROOK;

    //flagi roszad
    enPassantSquare = 64;
    prawaBialeKrotka  = true;
    prawaBialeDluga   = true;
    prawaCzarneKrotka = true;
    prawaCzarneDluga  = true;
 }

void Board::printBoard() const{
  const char pieceChars[] = {
        '.', // EMPTY = 0
        'P', 'N', 'B', 'R', 'Q', 'K', // Biale: 1 - 6
        'p', 'n', 'b', 'r', 'q', 'k'  // Czarne: 7 - 12
    };

   std::cout << "\x1b[?25l";
   std::cout << "\x1b[H";

   std::cout << "\n    a   b   c   d   e   f   g   h\n";
    std::cout << "  +---+---+---+---+---+---+---+---+\n";


    for (int wiersz = 7; wiersz >= 0; wiersz--) {
        std::cout << wiersz + 1 << " |";
        for (int kolumna = 0; kolumna < 8; kolumna++) {
            int square = wiersz * 8 + kolumna;
            int piece = szachownica[square];

            std::cout << " " << pieceChars[piece] << " |";
        }
        std::cout << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    std::cout << "\n";
}

void Board::MakeMove(Move& ruch) {

    ruch.poprzedniaBialaKrotka  = prawaBialeKrotka;
    ruch.poprzedniaBialaDluga   = prawaBialeDluga;
    ruch.poprzedniaCzarnaKrotka = prawaCzarneKrotka;
    ruch.poprzedniaCzarnaDluga  = prawaCzarneDluga;

    ruch.poprzedniPolRuchow50 = polRuchow50;

    if (ruch.ktoraFigura == W_PAWN || ruch.ktoraFigura == B_PAWN || ruch.ktoraFiguraZbita != EMPTY) {
        polRuchow50 = 0;
    } else {
        polRuchow50++;
    }

    ruch.stareEnPassantSquare = enPassantSquare; //zapisujemy stary stan
    enPassantSquare = 64;                        //kasujemy pole na nowy ruch

    if (ruch.ktoraFigura == W_PAWN && (ruch.naPole - ruch.zPola == 16)) {
        enPassantSquare = ruch.zPola + 8;
    }
    else if (ruch.ktoraFigura == B_PAWN && (ruch.zPola - ruch.naPole == 16)) {
        enPassantSquare = ruch.zPola - 8;
    }

    //usuwamy figure z pola startowego i kladzeimy na docelowym
    szachownica[ruch.naPole] = ruch.ktoraFigura;
    szachownica[ruch.zPola] = EMPTY;

    if (ruch.EnPassant) {
        if (ruch.ktoraFigura == W_PAWN) {
            szachownica[ruch.naPole - 8] = EMPTY;
        }
        else{
          szachownica[ruch.naPole + 8] = EMPTY;
        }
    }

    //Promocja (podmiana figury)
    if (ruch.promotedPiece != EMPTY) {
        szachownica[ruch.naPole] = ruch.promotedPiece;
    }

    //reczny zapis roszady
    if (ruch.Castling) {
             if (ruch.naPole == G1) { szachownica[F1] = W_ROOK; szachownica[H1] = EMPTY; } //biala krotka
        else if (ruch.naPole == C1) { szachownica[D1] = W_ROOK; szachownica[A1] = EMPTY; } //biala dluga
        else if (ruch.naPole == G8) { szachownica[F8] = B_ROOK; szachownica[H8] = EMPTY; } //czarna krotka
        else if (ruch.naPole == C8) { szachownica[D8] = B_ROOK; szachownica[A8] = EMPTY; } //czarna dluga
    }
    if (ruch.ktoraFigura == W_KING) { prawaBialeKrotka = false; prawaBialeDluga = false; }
    if (ruch.ktoraFigura == B_KING) { prawaCzarneKrotka = false; prawaCzarneDluga = false; }

    if (ruch.zPola == A1 || ruch.naPole == A1) prawaBialeDluga = false;
    if (ruch.zPola == H1 || ruch.naPole == H1) prawaBialeKrotka = false;
    if (ruch.zPola == A8 || ruch.naPole == A8) prawaCzarneDluga = false;
    if (ruch.zPola == H8 || ruch.naPole == H8) prawaCzarneKrotka = false;
}

void Board::UnmakeMove(Move ruch) {
    //powrot figury na pole startowe
    szachownica[ruch.naPole] = EMPTY;

    //cofanie zbicia lub usuwanie figuy z promocji
    szachownica[ruch.zPola] = ruch.ktoraFigura;

    if (ruch.EnPassant) {
        //enpassant cofanie piona
        if (ruch.ktoraFigura == W_PAWN) {
            szachownica[ruch.naPole - 8] = ruch.ktoraFiguraZbita;
        } else {
            szachownica[ruch.naPole + 8] = ruch.ktoraFiguraZbita;
        }
    }
    else if (ruch.ktoraFiguraZbita != EMPTY) {
        //zwykle bicie
        szachownica[ruch.naPole] = ruch.ktoraFiguraZbita;
    }


    //cofanie roszady
    if (ruch.Castling) {
             if (ruch.naPole == G1) { szachownica[H1] = W_ROOK; szachownica[F1] = EMPTY; } //biala krotka
        else if (ruch.naPole == C1) { szachownica[A1] = W_ROOK; szachownica[D1] = EMPTY; } //biala dluga
        else if (ruch.naPole == G8) { szachownica[H8] = B_ROOK; szachownica[F8] = EMPTY; } //czarna krotka
        else if (ruch.naPole == C8) { szachownica[A8] = B_ROOK; szachownica[D8] = EMPTY; } //czarna dluga
    }
    //cofanie licnzika
    polRuchow50 = ruch.poprzedniPolRuchow50;

    //prawo do enpassant
    enPassantSquare = ruch.stareEnPassantSquare;
    //prawo do roszady
    prawaBialeKrotka = ruch.poprzedniaBialaKrotka;
    prawaBialeDluga = ruch.poprzedniaBialaDluga;
    prawaCzarneKrotka = ruch.poprzedniaCzarnaKrotka;
    prawaCzarneDluga = ruch.poprzedniaCzarnaDluga;
}

bool Board::czyPoleJestAtakowane(int pole, int kolorAtakujacego) const{
//Sprawdza, czy dane pole jest atakowane przez dowolną figure przeciwnika, na podstawie ich regul ruchu

 //Skoczek
    int ruchyKonia[8] = {6, 10, 15, 17, -6, -10, -15, -17};
    int xStarowe = pole % 8;
    int yStarowe = pole / 8;

    for (int i = 0; i < 8; i++) {
        int cel = pole + ruchyKonia[i];
        if (cel >= 0 && cel < 64) {
            int xCel = cel % 8;
            int yCel = cel / 8;
            if (std::abs(xStarowe - xCel) + std::abs(yStarowe - yCel) == 3) {
                int figura = szachownica[cel];
                if (figura != EMPTY) {
                    //sprawdzamy czy to wrogi skoczek
                    bool toWrogiSkoczek = (kolorAtakujacego == WHITE && figura == W_KNIGHT) ||
                                          (kolorAtakujacego == BLACK && figura == B_KNIGHT);
                    if (toWrogiSkoczek) return true;
                }
            }
        }
    }
    //krol
    int ruchyKrola[8] = { 8, -8, 1, -1, 9, 7, -7, -9 };
    for (int i = 0; i < 8; i++) {
        int cel = pole + ruchyKrola[i];
        if (cel >= 0 && cel < 64) {
            int xCel = cel % 8;
            int yCel = cel / 8;
            if (std::abs(xStarowe - xCel) <= 1 && std::abs(yStarowe - yCel) <= 1) {
                int figura = szachownica[cel];
                if ((kolorAtakujacego == WHITE && figura == W_KING) ||
                    (kolorAtakujacego == BLACK && figura == B_KING)) {
                    return true;
                }
            }
        }
    }
    //pion
    //warunki sprawdzajce atak z lewej i prawej
    if(kolorAtakujacego == WHITE){
        if(pole % 8 != 0 && pole - 9 >= 0 && szachownica[pole - 9] == W_PAWN) return true;
        if(pole % 8 != 7 && pole - 7 >= 0 && szachownica[pole - 7] == W_PAWN) return true;

    }
    else{
        if(pole % 8 != 0 && pole + 7 < 64 && szachownica[pole + 7] == B_PAWN) return true;
        if(pole % 8 != 7 && pole + 9 < 64 && szachownica[pole + 9] == B_PAWN) return true;
    }
    //slizgajace figury
    int kierunki[8] = { 8, -8, 1, -1, 9, -9, 7, -7};

    for(int i = 0; i < 8; i++){
        int kierunek = kierunki[i];
        int aktualnePole = pole;

        while (true){
            int cel = aktualnePole + kierunek;
            if(cel < 0 || cel > 63) break;

            int xAktualny = aktualnePole % 8;
            int yAktualny = aktualnePole / 8;
            int xCel = cel % 8;
            int yCel = cel / 8;

            if(std::abs(xCel - xAktualny) > 1 || std::abs(yCel - yAktualny) > 1) break;

            int figura = szachownica[cel];
            if(figura != EMPTY){

                int jejKolor = WHITE;
                if (figura >= B_PAWN && figura <= B_KING) jejKolor = BLACK;

                if (jejKolor == kolorAtakujacego) {
                    //patrzymy po indekasch peirwszych czyli proste linie
                    if (i <= 3) {
                        if (figura == W_ROOK || figura == B_ROOK || figura == W_QUEEN || figura == B_QUEEN) return true;
                    }
                    //reszta indeskow proste skosy
                    else {
                        if (figura == W_BISHOP || figura == B_BISHOP || figura == W_QUEEN || figura == B_QUEEN) return true;
                    }
                }
                break;
            }
            aktualnePole = cel;
        }

    }
return false;
}

int Board::znajdzKrola(int kolor) const {
    int szukanaFigura = (kolor == WHITE) ? W_KING : B_KING;
    for (int i = 0; i < 64; ++i) {
        if (szachownica[i] == szukanaFigura) {
            return i;
        }
    }
    return SQ_NONE; // Zabezpieczenie, choć król zawsze powinien być na planszy
}

void Board::zapiszPozycje(int kolorNaRuchu){
 PozycjaZapis zapis;
 for(int i = 0; i < 64; i++) zapis.szachownica[i] = szachownica[i];
 zapis.kolorNaRuchu = kolorNaRuchu;
 zapis.bK = prawaBialeKrotka;
 zapis.bQ = prawaBialeDluga;
 zapis.cK = prawaCzarneKrotka;
 zapis.cQ = prawaCzarneDluga;
 zapis.enPassant = enPassantSquare;
 historiaPozycji.push_back(zapis);

}

void Board::wycofajPozycje(){
 if(!historiaPozycji.empty()){
    historiaPozycji.pop_back();
 }
}

bool Board::czyPowtorzenieTrzykrotne() const {
    if (historiaPozycji.empty()) return false;
    const PozycjaZapis& aktualna = historiaPozycji.back();

    int licznik = 0;
    for (const PozycjaZapis& p : historiaPozycji) {
        if (p.rownaSie(aktualna)) licznik++;
    }
    return licznik >= 3;
}
