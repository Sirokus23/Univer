#include "raylib.h"
#include "Object.h"
#include "Collision.h"
#include "UI.h"
#include <algorithm>
#include <random>

enum class Stare { Meniu, Simulare };

int main() {
    InitWindow(Config::Latime, Config::Inaltime, "Collision Lab");
    SetExitKey(KEY_NULL);                 // ESC goes back to the menu instead of closing the window
    SetTargetFPS(60);
    Texture2D cerc = CreeazaTexturaCerc();

    std::mt19937 gen{std::random_device{}()};
    Meniu meniu(gen);
    Obiecte obiecte;
    SistemColiziuni sistem(Config::Latime, Config::Inaltime, Config::MarimeCelula);

    Stare stare = Stare::Meniu;
    Algoritm mod = Algoritm::Grid;
    bool pauza = false;
    bool asteaptaEliberare = false;       // ignore the click that pressed START until it's released
    float acumulator = 0.0f, spawnAcum = 0.0f;
    float razaCamp = Config::RazaCampImplicit;

    while (!WindowShouldClose()) {
        float dtFrame = std::min(GetFrameTime(), 0.25f);

        // ================= start screen =================
        if (stare == Stare::Meniu) {
            RezultatMeniu r = meniu.Update(dtFrame);
            if (r == RezultatMeniu::Iesire) break;
            if (r == RezultatMeniu::Start) {
                obiecte = Obiecte{};
                GenereazaObiecte(obiecte, meniu.setari.NrObiecte(), Config::Latime, Config::Inaltime, gen);
                mod = meniu.setari.algoritm;
                sistem.SetMultithreading(meniu.setari.threads);
                pauza = false;
                acumulator = spawnAcum = 0.0f;
                asteaptaEliberare = true;
                stare = Stare::Simulare;
            }
            BeginDrawing();
                meniu.Deseneaza(cerc);
            EndDrawing();
            continue;
        }

        // ================= simulation: input =================
        if (IsKeyPressed(KEY_ESCAPE)) { meniu.Reintra(); stare = Stare::Meniu; continue; }
        if (IsKeyPressed(KEY_SPACE))  pauza = !pauza;
        if (IsKeyPressed(KEY_ONE))    mod = Algoritm::BruteForce;
        if (IsKeyPressed(KEY_TWO))    mod = Algoritm::Grid;
        if (IsKeyPressed(KEY_THREE))  mod = Algoritm::Quadtree;
        if (IsKeyPressed(KEY_FOUR))   mod = Algoritm::SweepAndPrune;
        if (IsKeyPressed(KEY_T))      sistem.SetMultithreading(!sistem.Multithreading());
        if (IsKeyPressed(KEY_C))      obiecte.EliminaUltimele(obiecte.Size());

        auto adauga = [&](int k) { GenereazaObiecte(obiecte, std::min(k, Config::ObiecteMax - obiecte.Size()), Config::Latime, Config::Inaltime, gen); };
        if (IsKeyPressed(KEY_UP))        adauga(Config::PasMic);
        if (IsKeyPressed(KEY_PAGE_UP))   adauga(Config::PasMare);
        if (IsKeyPressed(KEY_DOWN))      obiecte.EliminaUltimele(Config::PasMic);
        if (IsKeyPressed(KEY_PAGE_DOWN)) obiecte.EliminaUltimele(Config::PasMare);

        Vector2 mouse = GetMousePosition();
        razaCamp = std::clamp(razaCamp + GetMouseWheelMove() * 30.0f, Config::RazaCampMin, Config::RazaCampMax);

        if (asteaptaEliberare && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) asteaptaEliberare = false;
        bool repulsie = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !asteaptaEliberare && !pauza;

        // right mouse button: click = one ball, hold = a steady stream
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && obiecte.Size() < Config::ObiecteMax) {
            spawnAcum += dtFrame;
            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) spawnAcum = 1.0f / Config::SpawnPerSecunda;
            while (spawnAcum >= 1.0f / Config::SpawnPerSecunda && obiecte.Size() < Config::ObiecteMax) {
                GenereazaObiectLa(obiecte, mouse, gen);
                spawnAcum -= 1.0f / Config::SpawnPerSecunda;
            }
        } else {
            spawnAcum = 0.0f;
        }

        // ================= simulation: fixed timestep =================
        if (!pauza) {
            acumulator += dtFrame;
            int pasi = 0;
            while (acumulator >= Config::PasFix && pasi < Config::PasiMaxPerFrame) {
                if (repulsie) obiecte.ApplicaForta(mouse, razaCamp, Config::AcceleratieMouse, Config::PasFix);
                obiecte.Amortizeaza(Config::PasFix);
                obiecte.Update(Config::PasFix, Config::Latime, Config::Inaltime);
                sistem.Update(obiecte, mod);
                acumulator -= Config::PasFix;
                pasi++;
            }
            if (pasi == Config::PasiMaxPerFrame) acumulator = 0.0f;   // too slow to catch up: drop the backlog
        }

        // ================= draw =================
        BeginDrawing();
            DeseneazaFundal();
            sistem.DeseneazaDebug(mod);
            obiecte.Deseneaza(cerc);
            DeseneazaCamp(mouse, razaCamp, repulsie);

            DeseneazaHud(DateHud{mod, sistem.NumarComparatii(), sistem.NumarPerechi(), obiecte.Size(),
                                 sistem.NrThreaduri(), GetFPS(), sistem.TimpMs(),
                                 sistem.Multithreading(), pauza});
            DeseneazaAjutor();
            if (pauza) DeseneazaPauza();
        EndDrawing();
    }

    UnloadTexture(cerc);
    CloseWindow();
    return 0;
}