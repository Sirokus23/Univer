#pragma once
#include "raylib.h"
#include <random>
#include <vector>

namespace Config {
    constexpr int Latime = 1920;
    constexpr int Inaltime = 1080;

    constexpr int PasMic = 100;          // UP / DOWN
    constexpr int PasMare = 1000;        // PAGE UP / PAGE DOWN
    constexpr int ObiecteMax = 20000;

    // Ball size. Make these bigger/smaller to taste (cell size must stay >= 2 * RazaMax).
    constexpr float RazaMin = 8.0f;
    constexpr float RazaMax = 16.0f;
    constexpr float VitezaMax = 110.0f;   // initial speed per axis, +-

    constexpr float MarimeCelula = 50.0f;
    static_assert(MarimeCelula >= 2.0f * RazaMax, "grid cell must be at least one ball diameter");

    constexpr int QuadCapacitate = 4;
    constexpr int QuadAdancimeMax = 8;

    constexpr float PasFix = 1.0f / 120.0f;   // fixed physics timestep
    constexpr int PasiMaxPerFrame = 4;        // avoids the "spiral of death" when the sim is slow
    constexpr int ThreaduriMax = 8;

    // Mouse force field
    constexpr float AcceleratieMouse = 7000.0f;   // px/s^2 at the cursor, fades linearly to 0 at the edge
    constexpr float RazaCampImplicit = 180.0f;
    constexpr float RazaCampMin = 60.0f;
    constexpr float RazaCampMax = 500.0f;
    constexpr float SpawnPerSecunda = 45.0f;      // holding right mouse button

    // Speed limits: flung balls bleed off energy back to a calm speed
    constexpr float VitezaCalm = 160.0f;
    constexpr float VitezaCap = 900.0f;
    constexpr float Amortizare = 1.2f;            // 1/s
}

// Structure of arrays: collision loops only touch poz / raza (hot data),
// colours and flash stay out of the cache lines they read.
struct Obiecte {
    std::vector<Vector2> poz, viteza;
    std::vector<float> raza, masa;          // masa ~ area (r^2)
    std::vector<Color> culoare;
    std::vector<float> flash;               // 1 right after a collision, fades to 0 (drawn towards white)

    int Size() const { return (int)poz.size(); }
    void Adauga(Vector2 p, Vector2 v, float r, Color c);
    void EliminaUltimele(int k);

    void Update(float dt, int latimeEcran, int inaltimeEcran);
    bool Ciocnire(int a, int b) const;      // read-only overlap test (thread-safe)
    void Rezolva(int a, int b);             // impulse + positional correction

    void ApplicaForta(Vector2 centru, float razaCamp, float acceleratie, float dt);   // repel from a point
    void Amortizeaza(float dt);                                                        // bleed fast balls back to a calm speed

    void Deseneaza(const Texture2D& cerc, float alpha = 1.0f) const;
};

void GenereazaObiecte(Obiecte& obiecte, int nr, int latimeEcran, int inaltimeEcran, std::mt19937& gen);
void GenereazaObiectLa(Obiecte& obiecte, Vector2 pozitie, std::mt19937& gen);

// Pre-rendered shaded ball: one textured quad is much cheaper than a 36-segment DrawCircleV.
// Call after InitWindow().
Texture2D CreeazaTexturaCerc();