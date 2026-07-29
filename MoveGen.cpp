#include "MoveGen.h"
#include <cmath>
#include <iostream>

MoveGen::MoveGen(Board& b) : board(b) {}

int MoveGen::kolorFigury(int piece) const {
    if (piece >= W_PAWN && piece <= W_KING) return WHITE;
    if (piece >= B_PAWN && piece <= B_KING) return BLACK;
    return COLOR_NONE;
}

void MoveGen::generatePseudoLegal(int kolorFiguryRuszanej, std::vector<Move>& out){
  out.clear();
  out.reserve(256);

  for(int pole = 0; pole < 64; pole++){
    int figura = board.szachownica[pole];
    if(figura == EMPTY) continue; //puste pole ignorujemy

    if(kolorFigury(figura) == kolorFiguryRuszanej){
        //kon
        switch (figura) {
            case W_KNIGHT:
            case B_KNIGHT:
                generujRuchyKnight(pole, kolorFiguryRuszanej, out);
                break;
            case W_KING:
            case B_KING:
                generujRuchyKing(pole, kolorFiguryRuszanej, out);
                break;
            case W_PAWN:
            case B_PAWN:
                generujRuchyPawn(pole, kolorFiguryRuszanej, out);
                break;
            case W_BISHOP:
            case B_BISHOP:
            case W_ROOK:
            case B_ROOK:
            case W_QUEEN:
            case B_QUEEN:
                generujRuchySliding(pole, kolorFiguryRuszanej, figura, out);
                break;
        }
    }
  }
}

void MoveGen::generujRuchyKnight(int pole, int mojKolor, std::vector<Move>& out){
 //mozliwe przeskoki indeksu w jednowymiarowej tablicy
 int ruchyKonia[8] = {6,10,15,17,-6,-10,-15,-17};
 //wsprozedne konia
 int xStarowe = pole % 8; //x
 int yStarowe = pole / 8;

   for(int i = 0; i < 8; i++){
     int docelowePole = pole + ruchyKonia[i];
     if (docelowePole >= 0 && docelowePole < 64){ //czy nie ucieka poza szachownice
        int xDocelowe = docelowePole % 8;
        int yDocelowe = docelowePole / 8;
         if(std::abs(xStarowe - xDocelowe)+std::abs(yStarowe - yDocelowe) == 3){ //zabezpieczc=enie krawedzi
           int docelowaFigura = board.szachownica[docelowePole];
            if(docelowaFigura == EMPTY || kolorFigury(docelowaFigura) != mojKolor){
              Move ruch;
              ruch.zPola = pole;
              ruch.naPole = docelowePole;
              ruch.ktoraFigura = board.szachownica[pole];
              ruch.ktoraFiguraZbita = docelowaFigura;
              out.push_back(ruch);
            }
         }
     }
   }
}

void MoveGen::generujRuchyKing(int pole, int mojKolor, std::vector<Move>& out) {
 int ruchyKinga[8] = {1,-1,8,-8,9,-9,7,-7};
 int xStarowe = pole % 8;
 int yStarowe = pole / 8;

   for(int i = 0; i < 8; i++){
     int docelowePole = pole + ruchyKinga[i];
     if (docelowePole >= 0 && docelowePole < 64){
        int xDocelowe = docelowePole % 8;
        int yDocelowe = docelowePole / 8;
         if(std::abs(xStarowe - xDocelowe) <= 1 && std::abs(yStarowe - yDocelowe) <= 1){
           int docelowaFigura = board.szachownica[docelowePole];
            if(docelowaFigura == EMPTY || kolorFigury(docelowaFigura) != mojKolor){
              Move ruch;
              ruch.zPola = pole;
              ruch.naPole = docelowePole;
              ruch.ktoraFigura = board.szachownica[pole];
              ruch.ktoraFiguraZbita = docelowaFigura;
              out.push_back(ruch);
            }
         }
     }
   }

    //ROSZADY

    //biale
    if (mojKolor == WHITE && pole == E1) {
        //krotka roszada (w strone h1)
        if (board.prawaBialeKrotka) {
            //sprawdzamy czy pola f1 i g1 sa puste
            if (board.szachownica[F1] == EMPTY && board.szachownica[G1] == EMPTY) {
                if (!board.czyPoleJestAtakowane(E1, BLACK) &&
                    !board.czyPoleJestAtakowane(F1, BLACK) &&
                    !board.czyPoleJestAtakowane(G1, BLACK)) {
                       Move ruch;
                        ruch.zPola = E1;
                        ruch.naPole = G1;
                        ruch.ktoraFigura = W_KING;
                        ruch.ktoraFiguraZbita = EMPTY;
                        ruch.Castling = true; //flaga roszad
                        out.push_back(ruch);
                    }
            }
        }
        //dluga roszada analogicznie
        if (board.prawaBialeDluga) {
            //musza byc puste (b1, c1, d1)
            if (board.szachownica[B1] == EMPTY && board.szachownica[C1] == EMPTY && board.szachownica[D1] == EMPTY) {
                if (!board.czyPoleJestAtakowane(E1, BLACK) &&
                    !board.czyPoleJestAtakowane(D1, BLACK) &&
                    !board.czyPoleJestAtakowane(C1, BLACK)) {
                    Move ruch;
                    ruch.zPola = E1;
                    ruch.naPole = C1;
                    ruch.ktoraFigura = W_KING;
                    ruch.ktoraFiguraZbita = EMPTY;
                    ruch.Castling = true;
                    out.push_back(ruch);
                }
            }
        }
    }
    // czarne
    else if (mojKolor == BLACK && pole == E8) {
        //krotka roszada
        if (board.prawaCzarneKrotka) {
            if (board.szachownica[F8] == EMPTY && board.szachownica[G8] == EMPTY) {
                if (!board.czyPoleJestAtakowane(E8, WHITE) &&
                    !board.czyPoleJestAtakowane(F8, WHITE) &&
                    !board.czyPoleJestAtakowane(G8, WHITE)) {
                    Move ruch;
                    ruch.zPola = E8;
                    ruch.naPole = G8;
                    ruch.ktoraFigura = B_KING;
                    ruch.ktoraFiguraZbita = EMPTY;
                    ruch.Castling = true;
                    out.push_back(ruch);
                }
            }
        }
        //dluga roszada
        if (board.prawaCzarneDluga) {
            if (board.szachownica[B8] == EMPTY && board.szachownica[C8] == EMPTY && board.szachownica[D8] == EMPTY) {
              if (!board.czyPoleJestAtakowane(E8, WHITE) &&
                  !board.czyPoleJestAtakowane(D8, WHITE) &&
                  !board.czyPoleJestAtakowane(C8, WHITE)) {
                    Move ruch;
                    ruch.zPola = E8;
                    ruch.naPole = C8;
                    ruch.ktoraFigura = B_KING;
                    ruch.ktoraFiguraZbita = EMPTY;
                    ruch.Castling = true;
                    out.push_back(ruch);
                }
            }
        }
    }
}

void MoveGen::generujRuchyPawn(int pole, int mojKolor, std::vector<Move>& out) {
    //biale
    if(mojKolor == WHITE){
        int poleDoPrzodu = pole + 8;
        if (poleDoPrzodu < 64 && board.szachownica[poleDoPrzodu] == EMPTY) {
            //promojca
            if (poleDoPrzodu >= 56) { //wiersz promocyjny ma indeksy od 56 do 63 dla bialych
                int promocja[4] = {W_QUEEN, W_BISHOP, W_ROOK, W_KNIGHT}; //mozliwe promocje kazda osobny ruch
                for( int i = 0; i < 4; i++){
                    Move ruch;
                    ruch.zPola = pole;
                    ruch.naPole = poleDoPrzodu;
                    ruch.ktoraFigura = W_PAWN;
                    ruch.ktoraFiguraZbita = EMPTY;
                    ruch.promotedPiece = promocja[i];
                    out.push_back(ruch);
                }
            }
            else{
                //zwykly ruch
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = poleDoPrzodu;
                ruch.ktoraFigura = W_PAWN;
                ruch.ktoraFiguraZbita = EMPTY;
                out.push_back(ruch);
                //ruch startowy mozliwei o 2 pola
                if(pole >= 8 && pole <= 15){
                    int podwojnyRuch = pole + 16;
                    if(board.szachownica[podwojnyRuch] == EMPTY){
                        Move podwojny;
                        podwojny.zPola = pole;
                        podwojny.naPole = podwojnyRuch;
                        podwojny.ktoraFigura = W_PAWN;
                        podwojny.ktoraFiguraZbita = EMPTY;
                        podwojny.promotedPiece = EMPTY;
                        out.push_back(podwojny);
                    }
                }
            }
        }
        //bicie skoks lewo
        if(pole % 8 != 0){
            int bicieLewo = pole + 7;
            if(board.szachownica[bicieLewo] != EMPTY && kolorFigury(board.szachownica[bicieLewo]) == BLACK){
                //ewentualna promocja podczas bicia
                if(bicieLewo >= 56){
                    int promocja[4] = {W_QUEEN, W_BISHOP, W_ROOK, W_KNIGHT};
                        for( int i = 0; i < 4; i++){
                            Move ruch;
                            ruch.zPola = pole;
                            ruch.naPole = bicieLewo;
                            ruch.ktoraFigura = W_PAWN;
                            ruch.ktoraFiguraZbita = board.szachownica[bicieLewo];
                            ruch.promotedPiece = promocja[i];
                            out.push_back(ruch);
                        }
                }
                else{
                        Move ruch;
                        ruch.zPola = pole;
                        ruch.naPole = bicieLewo;
                        ruch.ktoraFigura = W_PAWN;
                        ruch.ktoraFiguraZbita = board.szachownica[bicieLewo];
                        ruch.promotedPiece = EMPTY;
                        out.push_back(ruch);
                }
            }
        }
        //bicie skos prawo (analogicznie jak w lewo)
        if(pole % 8 != 7){
            int biciePrawo = pole + 9;
            if (board.szachownica[biciePrawo] != EMPTY && kolorFigury(board.szachownica[biciePrawo]) == BLACK) {
                if (biciePrawo >= 56) {
                    int promojca[4] = { W_QUEEN, W_ROOK, W_BISHOP, W_KNIGHT };
                    for (int i = 0; i < 4; i++) {
                        Move ruch;
                        ruch.zPola = pole;
                        ruch.naPole = biciePrawo;
                        ruch.ktoraFigura = W_PAWN;
                        ruch.ktoraFiguraZbita = board.szachownica[biciePrawo];
                        ruch.promotedPiece = promojca[i];
                        out.push_back(ruch);
                    }
                } else {
                    Move ruch;
                    ruch.zPola = pole;
                    ruch.naPole = biciePrawo;
                    ruch.ktoraFigura = W_PAWN;
                    ruch.ktoraFiguraZbita = board.szachownica[biciePrawo];
                    ruch.promotedPiece = EMPTY;
                    out.push_back(ruch);
                }
            }
        }
        if (board.enPassantSquare != 64) { //zmeina do enPassant (pole za figura ktora da sie zbic w locie)
            //bicie w lewo na ukos
            if (pole % 8 != 0 && (pole + 7) == board.enPassantSquare) { //jesli nie stoi przy lewej kraedzi i po ruchu na skoks do przodu znajdzie sie na polu enPassant
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = board.enPassantSquare;
                ruch.ktoraFigura = W_PAWN;
                ruch.ktoraFiguraZbita = B_PAWN;
                ruch.EnPassant = true;
                out.push_back(ruch);
            }
            //bicie w prawo na ukos
            if (pole % 8 != 7 && (pole + 9) == board.enPassantSquare) { //analogicznie tylko dla prawej strony planszy
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = board.enPassantSquare;
                ruch.ktoraFigura = W_PAWN;
                ruch.ktoraFiguraZbita = B_PAWN;
                ruch.EnPassant = true;
                out.push_back(ruch);
            }
        }
    }
    //czarne analogicznie jak biale
    else if (mojKolor == BLACK) {
        int poleDoPrzodu = pole - 8;
        if (poleDoPrzodu >= 0 && board.szachownica[poleDoPrzodu] == EMPTY) {
            if (poleDoPrzodu <= 7) {
                int promocja[4] = {B_QUEEN, B_BISHOP, B_ROOK, B_KNIGHT};
                for(int i = 0; i < 4; i++) {
                    Move ruch;
                    ruch.zPola = pole;
                    ruch.naPole = poleDoPrzodu;
                    ruch.ktoraFigura = B_PAWN;
                    ruch.ktoraFiguraZbita = EMPTY;
                    ruch.promotedPiece = promocja[i];
                    out.push_back(ruch);
                }
            }
            else {
                //zwykly ruch
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = poleDoPrzodu;
                ruch.ktoraFigura = B_PAWN;
                ruch.ktoraFiguraZbita = EMPTY;
                ruch.promotedPiece = EMPTY;
                out.push_back(ruch);

                //ruch startowy o 2 pola (czarne piony startuja na linii 7 o indeksy 48-55)
                if (pole >= 48 && pole <= 55) {
                    int podwojnyRuch = pole - 16;
                    if (board.szachownica[podwojnyRuch] == EMPTY) {
                        Move podwojny;
                        podwojny.zPola = pole;
                        podwojny.naPole = podwojnyRuch;
                        podwojny.ktoraFigura = B_PAWN;
                        podwojny.ktoraFiguraZbita = EMPTY;
                        podwojny.promotedPiece = EMPTY;
                        out.push_back(podwojny);
                    }
                }
            }
        }
        //bicie po skosie w LEWO dla czarnych
        if (pole % 8 != 0) {
            int bicieLewo = pole - 9;
            if (board.szachownica[bicieLewo] != EMPTY && kolorFigury(board.szachownica[bicieLewo]) == WHITE) {
                if (bicieLewo <= 7) {
                    int promocja[4] = {B_QUEEN, B_BISHOP, B_ROOK, B_KNIGHT};
                    for(int i = 0; i < 4; i++) {
                        Move ruch;
                        ruch.zPola = pole;
                        ruch.naPole = bicieLewo;
                        ruch.ktoraFigura = B_PAWN;
                        ruch.ktoraFiguraZbita = board.szachownica[bicieLewo];
                        ruch.promotedPiece = promocja[i];
                        out.push_back(ruch);
                    }
                }
                else {
                    Move ruch;
                    ruch.zPola = pole;
                    ruch.naPole = bicieLewo;
                    ruch.ktoraFigura = B_PAWN;
                    ruch.ktoraFiguraZbita = board.szachownica[bicieLewo];
                    ruch.promotedPiece = EMPTY;
                    out.push_back(ruch);
                }
            }
        }
        //bicie po skosie w PRAWO dla czarnych
        if (pole % 8 != 7) {
            int biciePrawo = pole - 7;
            if (board.szachownica[biciePrawo] != EMPTY && kolorFigury(board.szachownica[biciePrawo]) == WHITE) {
                if (biciePrawo <= 7) {
                    int promocja[4] = { B_QUEEN, B_ROOK, B_BISHOP, B_KNIGHT };
                    for (int i = 0; i < 4; i++) {
                        Move ruch;
                        ruch.zPola = pole;
                        ruch.naPole = biciePrawo;
                        ruch.ktoraFigura = B_PAWN;
                        ruch.ktoraFiguraZbita = board.szachownica[biciePrawo];
                        ruch.promotedPiece = promocja[i];
                        out.push_back(ruch);
                    }
                } else {
                    Move ruch;
                    ruch.zPola = pole;
                    ruch.naPole = biciePrawo;
                    ruch.ktoraFigura = B_PAWN;
                    ruch.ktoraFiguraZbita = board.szachownica[biciePrawo];
                    ruch.promotedPiece = EMPTY;
                    out.push_back(ruch);
                }
            }
        }
        if (board.enPassantSquare != 64) {
            //bicie w lewo na ukos (patrzac od strony czarnych)
            if (pole % 8 != 0 && (pole - 9) == board.enPassantSquare) {
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = board.enPassantSquare;
                ruch.ktoraFigura = B_PAWN;
                ruch.ktoraFiguraZbita = W_PAWN;
                ruch.EnPassant = true;
                out.push_back(ruch);
            }
            //bicie w prawo na ukos
            if (pole % 8 != 7 && (pole - 7) == board.enPassantSquare) {
                Move ruch;
                ruch.zPola = pole;
                ruch.naPole = board.enPassantSquare;
                ruch.ktoraFigura = B_PAWN;
                ruch.ktoraFiguraZbita = W_PAWN;
                ruch.EnPassant = true;
                out.push_back(ruch);
            }
        }
    }
}

void MoveGen::generujRuchySliding(int startPole, int mojKolor, int rodzajFigury, std::vector<Move>& out) {
    //kierunki
    int kierunki[8] = {8, -8, 1, -1, 9, -9, 7, -7};

    int startIndeks = 0;
    int koniecIndeks = 8;

    if (rodzajFigury == W_ROOK || rodzajFigury == B_ROOK) {
        koniecIndeks = 4; //wieza
    }
    else if (rodzajFigury == W_BISHOP || rodzajFigury == B_BISHOP) {
        startIndeks = 4;  //goniec
    }

    //jesli hetman to nieograniczamy kierunkow
    for (int i = startIndeks; i < koniecIndeks; ++i) {
        int kierunek = kierunki[i];
        int aktualnePole = startPole;

        while(true){
            int celPole = aktualnePole + kierunek;
            if (celPole < 0 || celPole > 63) break; //poza mapa

            int xStartowy = aktualnePole % 8;
            int yStartowy = aktualnePole / 8;
            int xDocelowe = celPole % 8;
            int yDocelowe = celPole / 8;

            if (std::abs(xDocelowe - xStartowy) > 1 || std::abs(yDocelowe - yStartowy) > 1) { //przeskakiwanie przez krawedzie mapy
                break;
            }

            int docelowaFigura = board.szachownica[celPole];

            if(docelowaFigura != EMPTY){
               if(kolorFigury(docelowaFigura) != mojKolor){
                Move ruch;
                    ruch.zPola = startPole;
                    ruch.naPole = celPole;
                    ruch.ktoraFigura = board.szachownica[startPole];
                    ruch.ktoraFiguraZbita = docelowaFigura;
                    out.push_back(ruch);
               }
               break;
            }
            Move ruch;
            ruch.zPola = startPole;
            ruch.naPole = celPole;
            ruch.ktoraFigura = board.szachownica[startPole];
            ruch.ktoraFiguraZbita = EMPTY;
            out.push_back(ruch);

            aktualnePole = celPole;
        }
    }
}

void MoveGen::generateLegal(int kolor, std::vector<Move>& out) {
  out.clear();
  std::vector<Move> pseudoRuchy;
  generatePseudoLegal(kolor, pseudoRuchy);

  out.reserve(pseudoRuchy.size()); //rezerwacja pamieci

  int wrog = przeciwnyKolor(kolor);

  for(Move& m : pseudoRuchy){
    board.MakeMove(m);
    int poleKrol = board.znajdzKrola(kolor);
    if(!board.czyPoleJestAtakowane(poleKrol, wrog)){
        out.push_back(m);
    }
    board.UnmakeMove(m);
  }
}
