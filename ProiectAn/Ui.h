#pragma once
#include "Collision.h"
#include "Object.h"
#include "raylib.h"
#include <random>

namespace Tema {
    inline const Color FundalSus{12, 16, 34, 255};
    inline const Color FundalJos{34, 22, 64, 255};
    inline const Color TextGri{150, 160, 190, 255};
}

Color CuloareAlgoritm(Algoritm a);
void DeseneazaFundal();

// ---------- in-game HUD ----------
struct DateHud {
    Algoritm algoritm;
    int comparatii, perechi, obiecte, nrThreaduri, fps;
    float timpMs;
    bool threads, pauza;
};
void DeseneazaHud(const DateHud& d);
void DeseneazaAjutor();
void DeseneazaCamp(Vector2 mouse, float raza, bool activ);   // force-field ring around the cursor
void DeseneazaPauza();

// ---------- start screen ----------
enum class RezultatMeniu { Nimic, Start, Iesire };

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
    static constexpr int NrRanduri = 5;     // algorithm, objects, threads, START, QUIT
    Rectangle RandRect(int i) const;
    void Schimba(int rand, int directie);

    Obiecte fundal;                         // decorative balls (no collisions)
    int selectat = 3;
    float timp = 0.0f;
};