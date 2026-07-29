#pragma once
#include "Board.h"
#include "MoveGen.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>
#include <chrono>
#include <cstdint>


// Rekurencyjne zliczanie węzłów
uint64_t perft(int depth, Board& board, MoveGen& moveGen, int kolor) {
    if (depth == 0) return 1ULL;

    uint64_t nodes = 0;

    std::vector<Move> legalneRuchy;
    moveGen.generateLegal(kolor, legalneRuchy);

    for (Move& ruch : legalneRuchy) {
        board.MakeMove(ruch);
        nodes += perft(depth - 1, board, moveGen, przeciwnyKolor(kolor));
        board.UnmakeMove(ruch);
    }

    return nodes;
}

// Główna funkcja wykonująca test dla kolejnych głębokości
void uruchomPerftTest(int maxGlebokosc, Board& board, MoveGen& moveGen, int kolor) {
    for (int d = 1; d <= maxGlebokosc; ++d) {
        auto start = std::chrono::high_resolution_clock::now();

        uint64_t nodes = perft(d, board, moveGen, kolor);

        auto end = std::chrono::high_resolution_clock::now();
        double sekundy = std::chrono::duration<double>(end - start).count();

        // Wypisanie w wybranym formacie z czasem w sekundach
        std::cout << "Perft(" << d << ") = " << nodes
                  << "   (czas: " << std::fixed << std::setprecision(4) << sekundy << " s)\n";
    }
}

int Board::wczytajFEN(const std::string& fen) {
    // 1. Resetowanie planszy i praw
    for (int i = 0; i < 64; i++) szachownica[i] = EMPTY;
    prawaBialeKrotka = false;
    prawaBialeDluga = false;
    prawaCzarneKrotka = false;
    prawaCzarneDluga = false;
    enPassantSquare = 64;

    std::istringstream ss(fen);
    std::string ustawienieFigur, aktywnyKolor, prawaRoszad, poleEnPassant;
    ss >> ustawienieFigur >> aktywnyKolor >> prawaRoszad >> poleEnPassant;

    // 2. Rozmieszczenie figur (FEN czyta od rzêdu 8 do 1)
    int rank = 7; // Zaczynamy od 8. rzêdu (indeks 7)
    int file = 0; // Zaczynamy od kolumny a (indeks 0)

    for (char c : ustawienieFigur) {
        if (c == '/') {
            rank--;      // Przejœcie do rzêdu ni¿ej
            file = 0;    // Reset kolumny
        }
        else if (isdigit(c)) {
            file += (c - '0'); // Puste pola - przesuwamy wskaŸnik kolumny
        }
        else {
            int square = rank * 8 + file;
            int figura = EMPTY;

            // Mapowanie znaków na figury
            switch (c) {
                case 'P': figura = W_PAWN; break;
                case 'N': figura = W_KNIGHT; break;
                case 'B': figura = W_BISHOP; break;
                case 'R': figura = W_ROOK; break;
                case 'Q': figura = W_QUEEN; break;
                case 'K': figura = W_KING; break;
                case 'p': figura = B_PAWN; break;
                case 'n': figura = B_KNIGHT; break;
                case 'b': figura = B_BISHOP; break;
                case 'r': figura = B_ROOK; break;
                case 'q': figura = B_QUEEN; break;
                case 'k': figura = B_KING; break;
            }

            if (square >= 0 && square < 64) {
                szachownica[square] = figura;
            }
            file++;
        }
    }

    // 3. Prawa do roszady
    if (prawaRoszad != "-") {
        for (char znak : prawaRoszad) {
            if (znak == 'K') prawaBialeKrotka = true;
            else if (znak == 'Q') prawaBialeDluga = true;
            else if (znak == 'k') prawaCzarneKrotka = true;
            else if (znak == 'q') prawaCzarneDluga = true;
        }
    }

    // 4. Pole En Passant (np. "e3")
    if (poleEnPassant != "-") {
        int epFile = poleEnPassant[0] - 'a';
        int epRank = poleEnPassant[1] - '1';
        enPassantSquare = epRank * 8 + epFile;
    }

    // 5. Zwracamy kolor (w = WHITE / b = BLACK)
    return (aktywnyKolor == "w") ? WHITE : BLACK;
}
