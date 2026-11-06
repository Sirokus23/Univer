#pragma once
#include "Collision.h"
#include "Object.h"
#include "raylib.h"
#include <random>

namespace Tema {
    inline const Color FundalSus{12, 16, 34, 255};
    inline const Color FundalJos{34, 22, 64, 255};
    inline const Color TextGri{150, 160, 190, 255};
    inline const Color Rosu{255, 107, 129, 255};
}

Color CuloareAlgoritm(Algoritm a);
void DeseneazaFundal();

// ---------- in-game HUD ----------
struct Istoric {                                  // ring buffer: collision time of the last N steps
    static constexpr int N = 240;                 // 240 steps at 120 Hz = 2 seconds
    float ms[N] = {};
    unsigned char algo[N] = {};
    int idx = 0, numar = 0;
    void Adauga(float v, Algoritm a) {
        ms[idx] = v; algo[idx] = (unsigned char)a;
        idx = (idx + 1) % N;
        if (numar < N) numar++;
    }
};

struct StatisticiAlgoritm {                       // last measurement per algorithm
    bool valid = false;
    float ms = 0.0f;
    int comparatii = 0, obiecte = 0;
};

struct Comutatoare {
    bool heatmap = false, perechi = false, viteza = false, trails = false, gravitatie = false;
    float restitutie = 1.0f;
};

struct DateHud {
    Algoritm algoritm = Algoritm::Grid;
    int comparatii = 0, perechi = 0, obiecte = 0, nrThreaduri = 1, fps = 0;
    float timpMs = 0.0f;
    bool threads = true, pauza = false;
    const Istoric* istoric = nullptr;
    const StatisticiAlgoritm* statistici = nullptr;   // array of 4, indexed by Algoritm
    Comutatoare comut;
    const char* layout = nullptr;                    // e.g. "Even: best case for the grid"
};

void DeseneazaHud(const DateHud& d);
void DeseneazaAjutor();
void DeseneazaCamp(Vector2 mouse, float raza, bool activ, bool atragere);   // force-field ring around the cursor
void DeseneazaPauza();

// ---------- start screen ----------
enum class RezultatMeniu { Nimic, Start, Explica, Benchmark, Iesire };

inline constexpr int OptiuniNr[] = {50, 150, 400, 1000, 2000};
inline constexpr int NrOptiuni = (int)(sizeof(OptiuniNr) / sizeof(OptiuniNr[0]));

struct SetariJoc {
    Algoritm algoritm = Algoritm::Grid;
    int indexNr = 1;
    bool threads = true;
    int NrObiecte() const { return OptiuniNr[indexNr]; }
};

class Meniu {
public:
    explicit Meniu(std::mt19937& gen);

    RezultatMeniu Update(float dt);
    void Deseneaza(const Texture2D& cerc) const;
    void Reintra() { timp = 0.0f; selectat = 3; }

    SetariJoc setari;

private:
    // rows: 0 algorithm, 1 objects, 2 threads, 3 START, 4 HOW IT WORKS, 5 BENCHMARK, 6 QUIT
    static constexpr int NrRanduri = 7;
    Rectangle RandRect(int i) const;
    void Schimba(int rand, int directie);

    Obiecte fundal;                         // decorative balls (no collisions)
    int selectat = 3;
    float timp = 0.0f;
};