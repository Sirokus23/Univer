#pragma once
#include "Collision.h"
#include "Object.h"
#include "UI.h"
#include <string>
#include <utility>
#include <vector>

// =====================================================================
//  "How it works": a fixed demo scene and a pre-computed list of steps
//  per algorithm. Drawing is a pure function of (algorithm, step), so
//  stepping forwards and backwards is free.
// =====================================================================
namespace Explicatie {
    constexpr float LatimeScena = 1240.0f, InaltimeScena = 760.0f;
    constexpr float OrigineX = 50.0f, OrigineY = 130.0f;
    constexpr float CelulaDemo = 100.0f;
    constexpr int NrBile = 22;
}

class Explicator {
public:
    using Pereche = std::pair<int, int>;

    struct Pas {
        int faza = 1;                       // 0 = build (insert / sort), 1 = detect, 2 = summary
        int activ = -1;                     // ball in focus
        std::vector<Pereche> verificate;    // pairs tested in this step
        std::vector<int> candidati;         // balls the broad phase handed over
        int cel = -1;                       // grid: current cell
        std::vector<int> vecine;            // grid: forward neighbour cells
        int inserate = 0;                   // grid / quadtree: how many balls are in the structure
        std::vector<Rectangle> frunze;      // quadtree: leaves at this moment
        Rectangle zona{0, 0, 0, 0};         // quadtree: query box
        float x0 = 0, x1 = 0;               // sweep: x-range of the active ball
        int pozSortat = -1, oprit = -1;     // sweep: rank of the active ball, ball that stopped the sweep
        int comparatii = 0, nrGasite = 0;   // cumulative
        std::string text;
    };

    Explicator();
    void Seteaza(Algoritm a);               // (re)builds the step list and restarts
    bool Update(float dt);                  // false = the user left the screen
    void Deseneaza(const Texture2D& cerc) const;

    // inspection (used by the tests)
    const Obiecte& Scena() const { return scena; }
    int NrPasi() const { return (int)pasi.size(); }
    const Pas& UltimulPas() const { return pasi.back(); }
    int CiocniriGasite() const { return (int)ordineGasite.size(); }
    int CiocniriAdevarate() const { return (int)adevarate.size(); }

private:
    Vector2 P(int i) const { return {Explicatie::OrigineX + scena.poz[i].x, Explicatie::OrigineY + scena.poz[i].y}; }
    Rectangle Tab(int i) const { return {50.0f + i * 215.0f, 66.0f, 200.0f, 40.0f}; }

    void GenereazaScena();
    void GenereazaBrute();
    void GenereazaGrid();
    void GenereazaQuadtree();
    void GenereazaSweep();
    void AdaugaRezumat(int comparatii);
    void Inregistreaza(Pas& p, const std::vector<Pereche>& perechi);   // counts + remembers collisions

    void DeseneazaBila(const Texture2D& cerc, int i, Color c, float alpha) const;
    std::vector<char> BileRosii(const Pas& p) const;
    void DeseneazaBrute(const Texture2D& cerc, const Pas& p, float frac) const;
    void DeseneazaGrid(const Texture2D& cerc, const Pas& p) const;
    void DeseneazaQuadtree(const Texture2D& cerc, const Pas& p) const;
    void DeseneazaSweep(const Texture2D& cerc, const Pas& p) const;
    void DeseneazaPanou(const Pas& p) const;

    Obiecte scena;
    std::vector<Pereche> adevarate;         // ground truth (brute force over the scene)
    std::vector<Pereche> ordineGasite;      // collisions in the order the algorithm found them
    std::vector<Pas> pasi;
    std::vector<int> celulaDe;              // grid: cell of every ball
    std::vector<int> ordineSweep;           // sweep: balls sorted by left edge
    int colGrid = 0, rowGrid = 0;

    Algoritm algoritm = Algoritm::BruteForce;
    int pas = 0;
    bool joaca = true;
    float acum = 0.0f;
    int idxViteza = 1;
};

// =====================================================================
//  Automatic benchmark: every algorithm x several object counts,
//  results shown as a chart and saved to benchmark_results.csv
// =====================================================================
struct RezultatBench {
    Algoritm algoritm;
    int n;
    int pasi;
    float ms, comparatii, perechi;          // averages per step
};

class Benchmark {
public:
    // msTinta / pasiMin / pasiMax: how long each configuration is measured (small values = quick tests)
    explicit Benchmark(bool threads, float msTinta = 1500.0f, int pasiMin = 20, int pasiMax = 300);

    bool Update();                          // false = the user pressed ESC
    void Deseneaza() const;
    bool Terminat() const { return idx >= (int)joburi.size(); }
    bool Salveaza(const char* fisier) const;
    const std::vector<RezultatBench>& Rezultate() const { return rezultate; }

    static constexpr int Numere[5] = {100, 500, 1000, 2000, 5000};

private:
    struct Job { Algoritm a; int n; };
    void PasUnic();

    std::vector<Job> joburi;
    std::vector<RezultatBench> rezultate;
    int idx = 0;
    bool pregatit = false;
    bool threads;
    float msTinta;
    int pasiMin, pasiMax;

    Obiecte obiecte;
    SistemColiziuni sistem;
    int incalzire = 0, pasi = 0;
    double sumMs = 0, sumComp = 0, sumPer = 0;
    bool salvat = false;
};