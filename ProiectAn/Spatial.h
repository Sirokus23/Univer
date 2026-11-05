#pragma once
#include "raylib.h"
#include <vector>

// Uniform grid stored as flat arrays (counting sort): no per-cell vectors,
// no allocation after the first frame, and objects of one cell sit next to each other in memory.
class SpatialGrid {
public:
    SpatialGrid(float marimeCelula, int latime, int inaltime);

    void Build(const std::vector<Vector2>& pozitii);

    int Coloane() const { return coloane; }
    int Rinduri() const { return rinduri; }
    int NrCelule() const { return coloane * rinduri; }
    float MarimeCelula() const { return marimeCelula; }
    void Reconfigureaza(float marimeNoua);       // change cell size (used when a giant ball appears)
    int CelulaX(float x) const;
    int CelulaY(float y) const;

    // Objects of cell c are Element(k) for k in [Inceput(c), Sfarsit(c))
    int Inceput(int c) const { return start[c]; }
    int Sfarsit(int c) const { return start[c + 1]; }
    int Element(int k) const { return elemente[k]; }

    void Deseneaza() const;

private:
    float marimeCelula;
    int latime, inaltime;
    int coloane, rinduri;
    std::vector<int> start;       // size NrCelule + 1
    std::vector<int> elemente;    // object ids grouped by cell
    std::vector<int> celulaDe;    // scratch
    std::vector<int> cursor;      // scratch
};

// Point quadtree stored in a flat node pool. Clear() only resets counters,
// so rebuilding every frame does no heap allocation once the pool has grown.
class Quadtree {
public:
    Quadtree(Rectangle limite, int capacitate, int adancimeMax);

    void Clear();
    void Insert(int id, Vector2 pozitie);
    void Interogare(const Rectangle& zona, std::vector<int>& rezultat) const;
    void Deseneaza() const;
    void Frunze(std::vector<Rectangle>& rezultat) const;     // rectangles of all leaves (used by the explainer)

private:
    struct Intrare { int id; Vector2 p; };
    struct Nod {
        Rectangle limite{};
        int primCopil = -1;                 // -1 = leaf; children are primCopil..primCopil+3
        std::vector<Intrare> intrari;
    };

    void InserareLa(int nod, int adancime, const Intrare& e);
    void Subdivide(int nod);
    int CopilPentru(int nod, Vector2 p) const;
    void InterogareLa(int nod, const Rectangle& zona, std::vector<int>& rezultat) const;
    void DeseneazaLa(int nod) const;
    void FrunzeLa(int nod, std::vector<Rectangle>& rezultat) const;

    std::vector<Nod> noduri;
    int nrNoduri = 1;
    int capacitate;
    int adancimeMax;
};