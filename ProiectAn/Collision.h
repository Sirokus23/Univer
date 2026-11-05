#pragma once
#include "Object.h"
#include "Spatial.h"
#include <utility>
#include <vector>

enum class Algoritm { BruteForce, Grid, Quadtree, SweepAndPrune };

inline const char* NumeAlgoritm(Algoritm a) {
    switch (a) {
        case Algoritm::BruteForce:     return "Brute Force";
        case Algoritm::Grid:           return "Spatial Grid";
        case Algoritm::Quadtree:       return "Quadtree";
        case Algoritm::SweepAndPrune:  return "Sweep and Prune";
    }
    return "?";
}

// One step = build broad-phase structure -> detect pairs (optionally multithreaded, read-only)
// -> resolve all pairs sequentially. Detection never mutates objects, so results don't depend
// on loop order and threads can't race.
class SistemColiziuni {
public:
    SistemColiziuni(int latime, int inaltime, float marimeCelula);

    void Update(Obiecte& obiecte, Algoritm algoritm);
    void DeseneazaDebug(Algoritm algoritm) const;
    void DeseneazaHeatmap(const Obiecte& obiecte);            // object density per grid cell
    void DeseneazaCandidati(const Obiecte& obiecte) const;    // every pair the broad phase handed to the narrow phase
    void SetInregistreazaCandidati(bool pornit) { inregCandidati = pornit; }

    int NumarComparatii() const { return comparatii; }
    int NumarPerechi() const { return perechiGasite; }
    float TimpMs() const { return timpMs; }          // smoothed, includes build + detect + resolve
    float UltimulMs() const { return ultimulMs; }    // raw time of the last step
    float CelulaGrid() const { return grid.MarimeCelula(); }

    void SetMultithreading(bool pornit) { multithreading = pornit; }
    bool Multithreading() const { return multithreading; }
    int NrThreaduri() const { return nrThreaduri; }

private:
    using Pereche = std::pair<int, int>;
    struct Candidat { int a, b; bool lovit; };
    struct BufferThread {
        std::vector<Pereche> perechi;
        std::vector<int> candidati;
        std::vector<Candidat> examinate;    // only filled while "show pairs" is on (capped)
        bool inregistreaza = false;
        int comparatii = 0;
    };

    static void Testeaza(const Obiecte& o, int a, int b, BufferThread& buf);

    void PregatesteGrid(const Obiecte& o);
    void PregatesteQuadtree(const Obiecte& o);
    void PregatesteSweep(const Obiecte& o);

    // Each "unit" is an independent chunk of work (one object or one cell); threads take strided units.
    void UnitateBruteForce(const Obiecte& o, int i, BufferThread& buf) const;
    void UnitateGrid(const Obiecte& o, int celula, BufferThread& buf) const;
    void UnitateQuadtree(const Obiecte& o, int i, BufferThread& buf) const;
    void UnitateSweep(const Obiecte& o, int k, BufferThread& buf) const;

    int comparatii = 0;
    int perechiGasite = 0;
    float timpMs = 0.0f;
    float ultimulMs = 0.0f;
    bool inregCandidati = false;
    float celulaBaza;
    std::vector<int> heat;

    bool multithreading = true;
    int nrThreaduri = 1;
    std::vector<BufferThread> buffere;

    SpatialGrid grid;
    Quadtree quadtree;
    float razaMax = 0.0f;
    std::vector<int> ordine;        // Sweep and Prune: object ids sorted by left edge (kept between frames)
};