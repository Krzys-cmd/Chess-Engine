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
| **Attacked Square Detection** | The `czyPoleJestAtakowane` function checks if a given square is threatened by any enemy piece - utilizing separate logic for knights, kings, pawns (asymmetrical capture direction for white/black), and sliding pieces (bishop/rook/queen) stopping at the first encountered obstacle. |
| **Full Special Move Support** | Castling (kingside and queenside, with empty square and king transit path attack verification), en passant with correct target square calculation based on the last two-square pawn advance, and pawn promotion to all four minor/major pieces. |
| **Move Reversibility (Make/Unmake)** | Every move carries the full context required to revert the state: the captured piece, previous castling rights, and the previous en passant square - this allows searching the game tree without copying the entire board array for every move. |
| **Game Tree Search (Negamax + Alpha-Beta)** | Recursive variant search with branch pruning (alpha-beta pruning), scaling mate values by depth, and draw detection (50-move rule, three-fold repetition). |
| **Iterative Deepening + Time Management** | The engine deepens its search progressively (1, 2, 3...), constantly publishing the best move found so far - ensuring it always has a reasonable move ready even if time runs out during a deeper iteration. The time limit is checked every 2048 nodes (bitwise optimization) without adding unnecessary overhead to every iteration. |
| **UCI Protocol** | Full support for basic **Universal Chess Interface** commands (`uci`, `isready`, `position`, `go`, `ucinewgame`) - the engine connects directly to external GUIs (Arena, CuteChess) and online platforms supporting UCI engines. |
| **FEN Support** | Loading arbitrary positions from FEN notation (`wczytajFEN`), which allows testing the engine on specific tactical scenarios rather than just from the starting position. |
| **Draw Detection by Repetition** | After every move, the engine saves a board state snapshot (`zapiszPozycje`) to history - comparing subsequent snapshots allows it to detect a three-fold repetition of the exact same position (`czyPowtorzenieTrzykrotne`), independent of the 50-move rule counter. |
| **Move Generator Tests (Perft)** | The generator's correctness is verified using the **perft** algorithm - counting all game tree nodes up to a specified depth and comparing them against reference values. This is a standard method for catching bugs in special move logic (castling, en passant, promotion) that remain invisible during standard test play. |

---

##  Architektura i Logika Projektu

Projekt stawia na czytelny podział odpowiedzialności między reprezentacją stanu gry (`Board`) a logiką generowania ruchów (`MoveGen`), co ułatwia dalszą rozbudowę o przeszukiwanie drzewa gry (Minimax/Negamax) bez ingerencji w rdzeń silnika.

###  Reprezentacja Ruchu i Stanu Odwracalnego

Struktura `Move` przechowuje nie tylko pole startowe i docelowe, ale też pełen kontekst niezbędny do bezstratnego cofnięcia operacji na planszy - kluczowe dla przeszukiwania typu *depth-first* bez alokowania nowej kopii planszy na każdym poziomie rekurencji:

```cpp
struct Move {
    int zPola, naPole;
    int ktoraFigura, ktoraFiguraZbita;
    bool Castling, EnPassant;
    int promotedPiece;
    int stareEnPassantSquare;
    int poprzedniPolRuchow50;
    bool poprzedniaBialaKrotka, poprzedniaBialaDluga;
    bool poprzedniaCzarnaKrotka, poprzedniaCzarnaDluga;
};
```

Dzięki temu `MakeMove`/`UnmakeMove` działają w czasie stałym względem rozmiaru planszy, bez kopiowania tablicy 64 pól przy każdym węźle drzewa.

###  Wykrywanie Bicia w Przelocie (En Passant)

Pole en passant jest wyliczane dynamicznie po każdym ruchu piona o dwa pola do przodu - nie jest to flaga statyczna, lecz aktywnie odświeżane pole ważne wyłącznie przez jeden półruch:

```cpp
if (ruch.ktoraFigura == W_PAWN && (ruch.naPole - ruch.zPola == 16))
    enPassantSquare = ruch.zPola + 8;
```

Generator pionów sprawdza to pole niezależnie od standardowych bić po skosie, co pozwala poprawnie obsłużyć jeden z najczęściej błędnie implementowanych ruchów w silnikach szachowych.

###  Weryfikacja Legalności przez Symulację (Make-Check-Unmake)

Zamiast utrzymywać osobną, kosztowną strukturę do wykrywania spinek (pinned pieces), silnik weryfikuje legalność każdego ruchu brute-force - wykonuje go, sprawdza czy własny król jest atakowany, po czym cofa:

```cpp
plansza.MakeMove(m);
int poleKrol = plansza.znajdzKrola(kolor);
if (!plansza.czyPoleJestAtakowane(poleKrol, wrog))
    legalneRuchy.push_back(m);
plansza.UnmakeMove(m);
```

Rozwiązanie mniej wydajne niż podejście oparte na bitboardach i preliczonych liniach ataku, ale znacznie prostsze koncepcyjnie i mniej podatne na błędy przy pierwszej implementacji. Zarówno `generatePseudoLegal`, jak i `generateLegal` przyjmują docelowy wektor jako parametr wyjściowy (`std::vector<Move>& out`) zamiast go zwracać - pozwala to wywołującemu (np. `Search`) wielokrotnie reużywać ten sam bufor pamięci zamiast alokować nowy przy każdym węźle drzewa przeszukiwania.

###  Detekcja Remisu przez Powtórzenie Pozycji

Po każdym wykonanym ruchu silnik zapisuje pełną migawkę stanu gry (ułożenie figur, prawa do roszady, pole en passant, strona na ruchu) do historii `historiaPozycji`. Wykrycie remisu przez trzykrotne powtórzenie polega na porównaniu aktualnej migawki z wszystkimi poprzednimi:

```cpp
int licznik = 0;
for (const PozycjaZapis& p : historiaPozycji) {
    if (p.rownaSie(aktualna)) licznik++;
}
return licznik >= 3;
```

Zapis i wycofanie migawki (`zapiszPozycje`/`wycofajPozycje`) są symetryczne względem `MakeMove`/`UnmakeMove`, dzięki czemu historia pozostaje spójna nawet podczas głębokiej rekurencji przeszukiwania, gdzie te same węzły są wielokrotnie odwiedzane i cofane.

###  Detekcja Ataku na Pole (Sliding Pieces)

Dla figur ślizgających się (goniec, wieża, hetman) silnik iteruje w 8 możliwych kierunkach z tablicy przesunięć, zatrzymując się na pierwszej napotkanej figurze - rozróżniając kierunki „proste" (indeksy 0-3: pion/wieża/hetman) od „skośnych" (indeksy 4-7: goniec/hetman):

```cpp
int kierunki[8] = { 8, -8, 1, -1, 9, -9, 7, -7 };
```

Zabezpieczenie przed „zawijaniem się" promienia przez krawędź planszy realizowane jest poprzez porównanie współrzędnych x/y kolejnych pól, a nie tylko surowego indeksu tablicy.

###  Negamax z Alfa-Beta Pruning

Przeszukiwanie drzewa gry oparte jest o algorytm **Negamax** - uproszczoną wersję Minimax wykorzystującą symetrię gry o sumie zerowej (`ocena(gracz) = -ocena(przeciwnik)`), rozszerzoną o obcinanie gałęzi niemogących wpłynąć na ostateczny wynik:

```cpp
int wynik = - negamax(przeciwnyKolor(kolor), depth - 1, polRuch + 1, -beta, -alpha);
...
if (wynik > alpha) alpha = wynik;
if (alpha >= beta) break; // odcięcie gałęzi
```

Wartość mata jest skalowana głębokością posunięcia (`polRuch`), dzięki czemu silnik zawsze preferuje **najszybszy możliwy mat**, a nie dowolne, odległe w czasie zakończenie gry:

```cpp
if (szach) return -(MatWartosc - polRuch);
```

###  Iterative Deepening i Bezpieczne Przerywanie w Czasie

Zamiast przeszukiwać od razu do stałej głębokości, silnik pogłębia analizę iteracyjnie (`gleb = 1, 2, 3, …`), zachowując po każdej **w pełni ukończonej** iteracji najlepszy dotąd znaleziony ruch. Jeśli limit czasu zostanie przekroczony w trakcie głębszej iteracji, jej niedokończony wynik jest odrzucany, a silnik zwraca ruch z ostatniej ukończonej głębokości - gwarantuje to, że bot nigdy nie zwróci ruchu wybranego na podstawie niepełnej analizy:

```cpp
if (przerwane) break;
najlepszyOgolem = najlepszyTejGlebokosci;
```

Sprawdzanie zegara odbywa się rzadko (co 2048 węzłów, `wezly & 2047`) zamiast po każdym węźle, co eliminuje zbędny narzut wywołań `steady_clock::now()` na najgorętszej ścieżce silnika.

###  Protokół UCI - Komunikacja ze Światem Zewnętrznym

Silnik implementuje pętlę komend zgodną ze standardem UCI, tłumacząc tekstowe polecenia (np. `position startpos moves e2e4 e7e5`) na wewnętrzne operacje na `Board`/`MoveGen`, oraz konwertując znalezione ruchy z powrotem na notację algebraiczną wymaganą przez protokół:

```cpp
std::string UCI::ruchNaTekst(const Move& m) {
    std::string s = poleNaTekst(m.zPola) + poleNaTekst(m.naPole);
    if (m.promotedPiece != EMPTY) s += /* litera promocji */;
    return s;
}
```

Obsługa komendy `go` automatycznie wybiera tryb pracy silnika w zależności od otrzymanych parametrów - stała głębokość (`depth`), stały czas na ruch (`movetime`) lub podział pozostałego czasu partii (`wtime`/`btime`, z rezerwą minimalną 50 ms na wypadek bardzo niskiego stanu zegara).

###  Punkt Wejścia (`main.cpp`)

Uruchomienie silnika sprowadza się do złożenia czterech głównych komponentów w jedną, gotową do pracy jednostkę - `Board` jako źródło stanu gry, `MoveGen` i `Search` operujące na nim przez referencję, oraz `UCI` spinający całość w pętlę komend sterowaną ze standardowego wejścia:

```cpp
Board board;
MoveGen gen(board);
Search ser(board, gen);
UCI uci(board, gen, ser);
uci.uruchomPetle();
```

Taki układ referencji (a nie kopii) gwarantuje, że wszystkie komponenty zawsze operują na dokładnie tym samym stanie planszy - bez ryzyka rozjazdu opisanego wcześniej przy okazji architektury `MoveGen`.

---

##  Struktura Klas

* **`Board`**: Przechowuje stan planszy (`szachownica[64]`), flagi roszad, pole en passant oraz historię pozycji (`historiaPozycji`) i licznik posunięć do reguły 50 ruchów (`polRuchow50`). Odpowiada za inicjalizację pozycji startowej i wczytywanie z FEN, wykonywanie i cofanie ruchów (`MakeMove`/`UnmakeMove`), detekcję ataku na pole (`czyPoleJestAtakowane`), lokalizację króla (`znajdzKrola`) oraz wykrywanie remisów przez zapisywanie i porównywanie migawek pozycji (`zapiszPozycje`, `wycofajPozycje`, `czyPowtorzenieTrzykrotne`).
* **`MoveGen`**: Generator ruchów operujący na referencji do `Board`. Dzieli logikę na osobne metody per typ figury (`generujRuchyKnight`, `generujRuchyKing`, `generujRuchyPawn`, `generujRuchySliding`), zapisujące wygenerowane ruchy bezpośrednio do przekazanego wektora wyjściowego, oraz filtr `generateLegal`, odsiewający ruchy zostawiające własnego króla w szachu.
* **`Search`**: Silnik przeszukujący drzewo gry algorytmem Negamax z alfa-beta pruning. Udostępnia dwa tryby pracy: przeszukiwanie do stałej głębokości (`szukajNajlepszegoRuchu`) oraz iterative deepening z limitem czasowym (`szukajRuchWCzasie`). Zawiera statyczną funkcję oceny materiałowej (`ocenPozycje`) oraz mechanizm bezpiecznego przerywania obliczeń po przekroczeniu limitu czasu (`minalCzas`).
* **`UCI`**: Warstwa komunikacji implementująca protokół Universal Chess Interface - parsuje komendy tekstowe ze standardowego wejścia, tłumaczy notację ruchów (np. `e2e4`) na obiekty `Move`, oraz spina ze sobą `Board`, `MoveGen` i `Search` w jedną, sterowalną z zewnątrz pętlę gry.
* **`Move`**: Struktura danych opisująca pojedynczy ruch wraz z pełnym kontekstem potrzebnym do jego odwrócenia (zbita figura, poprzednie prawa do roszady, poprzednie pole en passant).

---

##  Status projektu 

Silnik posiada w pełni zweryfikowaną (testami **perft**) reprezentację planszy i generator ruchów, działające przeszukiwanie Negamax z alfa-beta oraz pełne podłączenie do protokołu UCI - może już grać partie w zewnętrznych GUI. Kolejne etapy rozwoju skupiają się na sile gry:

- [ ] Piece-Square Tables - ocena pozycyjna figur, nie tylko materiał
- [ ] Tablica transpozycji (Zobrist hashing)
- [ ] Null-move pruning i Late Move Reduction (LMR)
