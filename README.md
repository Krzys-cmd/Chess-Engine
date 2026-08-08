<h3 align="center">Silnik szachowy(C++)</h3>

<p align="center">
Silnik szachowy pisany od zera w **C++17**, oparty na płaskiej reprezentacji planszy (tablica 64 pól, tzw. mailbox) z walidacją granic przez współrzędne x/y, oraz generatorze ruchów zgodnym z pełnymi zasadami gry — łącznie z roszadą, biciem w przelocie i promocją.
</p>

##  Główne funkcje

| Funkcja | Opis |
|---|---|
| **Reprezentacja planszy** | Tablica jednowymiarowa `int[64]` (indeksy A1–H8) przechowująca stan figur, z pełnym zestawem flag stanu gry: prawa do roszady (osobno dla obu stron i skrzydeł) oraz pole bicia w przelocie (`enPassantSquare`). |
| **Generator pseudo-legalnych ruchów** | Osobne funkcje generujące ruchy dla każdego typu figury — skoczka i króla poprzez tablice przesunięć indeksu, gońca/wieży/hetmana poprzez iteracyjne „ślizganie się" w 8 kierunkach aż do napotkania przeszkody lub krawędzi planszy. |
| **Filtr ruchów legalnych** | Weryfikacja każdego pseudo-legalnego ruchu metodą *make–check–unmake*: ruch jest wykonywany na planszy, sprawdzana jest ekspozycja własnego króla na szach, a następnie ruch jest cofany — eliminuje to ruchy odsłaniające króla (spinki). Generator zapisuje wyniki do przekazanego z zewnątrz wektora (`out`-parametr) zamiast zwracać nową kopię listy przy każdym wywołaniu, co eliminuje zbędne alokacje pamięci w gorącej ścieżce przeszukiwania. |
| **Detekcja atakowanych pól** | Funkcja `czyPoleJestAtakowane` sprawdza zagrożenie danego pola przez dowolną figurę przeciwnika — osobna logika dla skoczka, króla, pionów (asymetryczny kierunek bicia biały/czarny) oraz figur ślizgających się (goniec/wieża/hetman) z zatrzymaniem na pierwszej napotkanej przeszkodzie. |
| **Pełna obsługa ruchów specjalnych** | Roszada (krótka i długa, z weryfikacją pustych pól oraz braku ataku na pola przejścia króla), bicie w przelocie (en passant) z poprawnym wyznaczaniem pola docelowego na podstawie ostatniego ruchu piona o dwa pola, oraz promocja piona do każdej z czterech figur. |
| **Odwracalność ruchów (Make/Unmake)** | Każdy ruch niesie ze sobą pełny kontekst potrzebny do cofnięcia stanu: zbitą figurę, poprzednie prawa do roszady oraz poprzednie pole en passant — pozwala to na przeszukiwanie drzewa gry bez kopiowania całej planszy przy każdym ruchu. |
| **Przeszukiwanie drzewa gry (Negamax + Alfa-Beta)** | Rekurencyjne przeszukiwanie wariantów z obcinaniem gałęzi (alfa-beta pruning), skalowaniem wartości mata względem głębokości oraz wykrywaniem remisów (reguła 50 posunięć, trzykrotne powtórzenie pozycji). |
| **Iterative Deepening + zarządzanie czasem** | Silnik pogłębia przeszukiwanie stopniowo (1, 2, 3…), publikując na bieżąco najlepszy dotychczas znaleziony ruch — dzięki temu zawsze dysponuje sensownym ruchem, nawet jeśli czas się skończy w trakcie głębszej iteracji. Limit czasu sprawdzany jest co 2048 węzłów (optymalizacja bitowa) bez zbędnego narzutu na każdą iterację. |
| **Protokół UCI** | Pełna obsługa podstawowych komend interfejsu **Universal Chess Interface** (`uci`, `isready`, `position`, `go`, `ucinewgame`) — silnik podłącza się bezpośrednio do zewnętrznych GUI (Arena, CuteChess) oraz platform online obsługujących silniki UCI. |
| **Obsługa FEN** | Wczytywanie dowolnej pozycji z notacji FEN (`wczytajFEN`), co pozwala testować silnik na konkretnych scenariuszach taktycznych, a nie tylko od pozycji startowej. |
| **Detekcja remisu przez powtórzenie pozycji** | Po każdym ruchu silnik zapisuje migawkę stanu planszy (`zapiszPozycje`) do historii — porównanie kolejnych migawek pozwala wykryć trzykrotne powtórzenie tej samej pozycji (`czyPowtorzenieTrzykrotne`), niezależnie od licznika reguły 50 posunięć. |
| **Testy generatora ruchów (Perft)** | Poprawność generatora zweryfikowana algorytmem **perft** — zliczaniem wszystkich węzłów drzewa gry do zadanej głębokości i porównaniem z wartościami referencyjnymi, co jest standardową metodą wykrywania błędów w logice ruchów specjalnych (roszada, en passant, promocja) niewidocznych przy zwykłej grze testowej. |


---

##  Architektura i Logika Projektu

Projekt stawia na czytelny podział odpowiedzialności między reprezentacją stanu gry (`Board`) a logiką generowania ruchów (`MoveGen`), co ułatwia dalszą rozbudowę o przeszukiwanie drzewa gry (Minimax/Negamax) bez ingerencji w rdzeń silnika.

###  Reprezentacja Ruchu i Stanu Odwracalnego

Struktura `Move` przechowuje nie tylko pole startowe i docelowe, ale też pełen kontekst niezbędny do bezstratnego cofnięcia operacji na planszy — kluczowe dla przeszukiwania typu *depth-first* bez alokowania nowej kopii planszy na każdym poziomie rekurencji:

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

Pole en passant jest wyliczane dynamicznie po każdym ruchu piona o dwa pola do przodu — nie jest to flaga statyczna, lecz aktywnie odświeżane pole ważne wyłącznie przez jeden półruch:

```cpp
if (ruch.ktoraFigura == W_PAWN && (ruch.naPole - ruch.zPola == 16))
    enPassantSquare = ruch.zPola + 8;
```

Generator pionów sprawdza to pole niezależnie od standardowych bić po skosie, co pozwala poprawnie obsłużyć jeden z najczęściej błędnie implementowanych ruchów w silnikach szachowych.

###  Weryfikacja Legalności przez Symulację (Make–Check–Unmake)

Zamiast utrzymywać osobną, kosztowną strukturę do wykrywania spinek (pinned pieces), silnik weryfikuje legalność każdego ruchu brute-force — wykonuje go, sprawdza czy własny król jest atakowany, po czym cofa:

```cpp
plansza.MakeMove(m);
int poleKrol = plansza.znajdzKrola(kolor);
if (!plansza.czyPoleJestAtakowane(poleKrol, wrog))
    legalneRuchy.push_back(m);
plansza.UnmakeMove(m);
```

Rozwiązanie mniej wydajne niż podejście oparte na bitboardach i preliczonych liniach ataku, ale znacznie prostsze koncepcyjnie i mniej podatne na błędy przy pierwszej implementacji. Zarówno `generatePseudoLegal`, jak i `generateLegal` przyjmują docelowy wektor jako parametr wyjściowy (`std::vector<Move>& out`) zamiast go zwracać — pozwala to wywołującemu (np. `Search`) wielokrotnie reużywać ten sam bufor pamięci zamiast alokować nowy przy każdym węźle drzewa przeszukiwania.

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

Dla figur ślizgających się (goniec, wieża, hetman) silnik iteruje w 8 możliwych kierunkach z tablicy przesunięć, zatrzymując się na pierwszej napotkanej figurze — rozróżniając kierunki „proste" (indeksy 0–3: pion/wieża/hetman) od „skośnych" (indeksy 4–7: goniec/hetman):

```cpp
int kierunki[8] = { 8, -8, 1, -1, 9, -9, 7, -7 };
```

Zabezpieczenie przed „zawijaniem się" promienia przez krawędź planszy realizowane jest poprzez porównanie współrzędnych x/y kolejnych pól, a nie tylko surowego indeksu tablicy.

###  Negamax z Alfa-Beta Pruning

Przeszukiwanie drzewa gry oparte jest o algorytm **Negamax** — uproszczoną wersję Minimax wykorzystującą symetrię gry o sumie zerowej (`ocena(gracz) = -ocena(przeciwnik)`), rozszerzoną o obcinanie gałęzi niemogących wpłynąć na ostateczny wynik:

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

Zamiast przeszukiwać od razu do stałej głębokości, silnik pogłębia analizę iteracyjnie (`gleb = 1, 2, 3, …`), zachowując po każdej **w pełni ukończonej** iteracji najlepszy dotąd znaleziony ruch. Jeśli limit czasu zostanie przekroczony w trakcie głębszej iteracji, jej niedokończony wynik jest odrzucany, a silnik zwraca ruch z ostatniej ukończonej głębokości — gwarantuje to, że bot nigdy nie zwróci ruchu wybranego na podstawie niepełnej analizy:

```cpp
if (przerwane) break;
najlepszyOgolem = najlepszyTejGlebokosci;
```

Sprawdzanie zegara odbywa się rzadko (co 2048 węzłów, `wezly & 2047`) zamiast po każdym węźle, co eliminuje zbędny narzut wywołań `steady_clock::now()` na najgorętszej ścieżce silnika.

###  Protokół UCI — Komunikacja ze Światem Zewnętrznym

Silnik implementuje pętlę komend zgodną ze standardem UCI, tłumacząc tekstowe polecenia (np. `position startpos moves e2e4 e7e5`) na wewnętrzne operacje na `Board`/`MoveGen`, oraz konwertując znalezione ruchy z powrotem na notację algebraiczną wymaganą przez protokół:

```cpp
std::string UCI::ruchNaTekst(const Move& m) {
    std::string s = poleNaTekst(m.zPola) + poleNaTekst(m.naPole);
    if (m.promotedPiece != EMPTY) s += /* litera promocji */;
    return s;
}
```

Obsługa komendy `go` automatycznie wybiera tryb pracy silnika w zależności od otrzymanych parametrów — stała głębokość (`depth`), stały czas na ruch (`movetime`) lub podział pozostałego czasu partii (`wtime`/`btime`, z rezerwą minimalną 50 ms na wypadek bardzo niskiego stanu zegara).

###  Punkt Wejścia (`main.cpp`)

Uruchomienie silnika sprowadza się do złożenia czterech głównych komponentów w jedną, gotową do pracy jednostkę — `Board` jako źródło stanu gry, `MoveGen` i `Search` operujące na nim przez referencję, oraz `UCI` spinający całość w pętlę komend sterowaną ze standardowego wejścia:

```cpp
Board board;
MoveGen gen(board);
Search ser(board, gen);
UCI uci(board, gen, ser);
uci.uruchomPetle();
```

Taki układ referencji (a nie kopii) gwarantuje, że wszystkie komponenty zawsze operują na dokładnie tym samym stanie planszy — bez ryzyka rozjazdu opisanego wcześniej przy okazji architektury `MoveGen`.

---

##  Struktura Klas

* **`Board`**: Przechowuje stan planszy (`szachownica[64]`), flagi roszad, pole en passant oraz historię pozycji (`historiaPozycji`) i licznik posunięć do reguły 50 ruchów (`polRuchow50`). Odpowiada za inicjalizację pozycji startowej i wczytywanie z FEN, wykonywanie i cofanie ruchów (`MakeMove`/`UnmakeMove`), detekcję ataku na pole (`czyPoleJestAtakowane`), lokalizację króla (`znajdzKrola`) oraz wykrywanie remisów przez zapisywanie i porównywanie migawek pozycji (`zapiszPozycje`, `wycofajPozycje`, `czyPowtorzenieTrzykrotne`).
* **`MoveGen`**: Generator ruchów operujący na referencji do `Board`. Dzieli logikę na osobne metody per typ figury (`generujRuchyKnight`, `generujRuchyKing`, `generujRuchyPawn`, `generujRuchySliding`), zapisujące wygenerowane ruchy bezpośrednio do przekazanego wektora wyjściowego, oraz filtr `generateLegal`, odsiewający ruchy zostawiające własnego króla w szachu.
* **`Search`**: Silnik przeszukujący drzewo gry algorytmem Negamax z alfa-beta pruning. Udostępnia dwa tryby pracy: przeszukiwanie do stałej głębokości (`szukajNajlepszegoRuchu`) oraz iterative deepening z limitem czasowym (`szukajRuchWCzasie`). Zawiera statyczną funkcję oceny materiałowej (`ocenPozycje`) oraz mechanizm bezpiecznego przerywania obliczeń po przekroczeniu limitu czasu (`minalCzas`).
* **`UCI`**: Warstwa komunikacji implementująca protokół Universal Chess Interface — parsuje komendy tekstowe ze standardowego wejścia, tłumaczy notację ruchów (np. `e2e4`) na obiekty `Move`, oraz spina ze sobą `Board`, `MoveGen` i `Search` w jedną, sterowalną z zewnątrz pętlę gry.
* **`Move`**: Struktura danych opisująca pojedynczy ruch wraz z pełnym kontekstem potrzebnym do jego odwrócenia (zbita figura, poprzednie prawa do roszady, poprzednie pole en passant).

---

##  Status projektu i plany rozwoju

Silnik posiada w pełni zweryfikowaną (testami **perft**) reprezentację planszy i generator ruchów, działające przeszukiwanie Negamax z alfa-beta oraz pełne podłączenie do protokołu UCI — może już grać partie w zewnętrznych GUI. Kolejne etapy rozwoju skupiają się na sile gry:

- [ ] **Quiescence search** — rozszerzenie oceny liścia drzewa o dogrywanie bić, eliminujące efekt horyzontu przy taktycznych wymianach
- [ ] Move ordering (sortowanie ruchów, np. MVV-LVA) dla zwiększenia skuteczności obcinania alfa-beta
- [ ] Piece-Square Tables — ocena pozycyjna figur, nie tylko materiał
- [ ] Tablica transpozycji (Zobrist hashing)
- [ ] Null-move pruning i Late Move Reduction (LMR)
