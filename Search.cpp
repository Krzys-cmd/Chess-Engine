#include "Search.h"


Search::Search(Board& board, MoveGen& gen) : board(board), gen(gen) {}

static const int wartosciFigur[13] = {
    0,
    100, 320, 330, 500, 900, 0,
    100, 320, 330, 500, 900, 0
};

int Search::ocenPozycje(int kolor) const{
 int suma = 0;
 for(int i = 0; i < 64; i++){
    int figura = board.szachownica[i];
    if(figura == EMPTY) continue;

    int wartosc = wartosciFigur[figura];
    int kolorFigury = (figura >= W_PAWN && figura <= W_KING) ? WHITE : BLACK;

    if(kolorFigury == kolor) suma += wartosc;
    else suma -= wartosc;
 }
 return suma;
}

bool Search::minalCzas(){
if(przerwane) return true;
auto teraz = std::chrono::steady_clock::now();
long long minelo = std::chrono::duration_cast<std::chrono::milliseconds>(teraz - czasStartu).count();
if (minelo >= limitCzasuMs) {
        przerwane = true;
    }
    return przerwane;
}

int Search::negamax(int kolor, int depth, int polRuch, int alpha, int beta , Move* najlepszyRuchOut){
 wezly++;

  if ((wezly & 2047) == 0) { //operacja bitowa dla optymalizacji
        if (minalCzas()) return 0;
    }
  if (przerwane) return 0;
  if (board.czyRemis50Ruchow() || board.czyPowtorzenieTrzykrotne()) {
        return 0;
   }

 std::vector<Move> ruchy;
 gen.generateLegal(kolor, ruchy);

 if(ruchy.empty()){
    int wrog = przeciwnyKolor(kolor);
    int poleKrol = board.znajdzKrola(kolor);
    bool szach = board.czyPoleJestAtakowane(poleKrol,wrog);

    if(szach) return -(MatWartosc - polRuch);
    else return 0;
 }
 if(depth == 0){
    return ocenPozycje(kolor);
 }

 int najlepszy = -inf;

 for(Move& m : ruchy){
    board.MakeMove(m);
    board.zapiszPozycje(przeciwnyKolor(kolor));

    int wynik = - negamax(przeciwnyKolor(kolor), depth - 1, polRuch + 1, -beta, -alpha);

    board.wycofajPozycje();
    board.UnmakeMove(m);

    if(przerwane) break;

    if(wynik > najlepszy){
      najlepszy = wynik;
      if(najlepszyRuchOut != nullptr){
        *najlepszyRuchOut = m;
      }
    }
    if(wynik > alpha) alpha = wynik;
    if(alpha >= beta) break;
 }
 return najlepszy;
}

Move Search::szukajNajlepszegoRuchu(int kolor, int depth) {
    przerwane = false;
    limitCzasuMs = 1LL << 40;
    czasStartu = std::chrono::steady_clock::now();

    Move najlepszyRuch{};
    int ocena = negamax(kolor, depth, 0, -inf, inf, &najlepszyRuch);

    std::cout << "info depth " << depth << " score cp " << ocena
               << " nodes " << wezly << std::endl;

    return najlepszyRuch;
}


Move Search::szukajRuchWCzasie(int kolor, long long limitCzasuMsParam) {
    czasStartu = std::chrono::steady_clock::now();
    limitCzasuMs = limitCzasuMsParam;
    przerwane = false;
    wezly = 0;

    Move najlepszyOgolem{};
    int gleb = 1;

    while (true) {
        Move najlepszyTejGlebokosci{};
        int ocena = negamax(kolor, gleb, 0, -inf, inf, &najlepszyTejGlebokosci);

        if (przerwane) break;

        najlepszyOgolem = najlepszyTejGlebokosci;

        auto teraz = std::chrono::steady_clock::now();
        long long czasMs = std::chrono::duration_cast<std::chrono::milliseconds>(teraz - czasStartu).count();
        long long nps = (czasMs > 0) ? (wezly * 1000 / czasMs) : 0;

        std::cout << "info depth " << gleb
                   << " score cp " << ocena
                   << " nodes " << wezly
                   << " nps " << nps
                   << " time " << czasMs
                   << std::endl;

        gleb++;
        if (gleb > 60) break;
    }

    return najlepszyOgolem;
}
