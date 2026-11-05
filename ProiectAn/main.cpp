#include "raylib.h"
#include "Object.h"
#include "Collision.h"
#include "UI.h"
#include "Modes.h"
#include <algorithm>
#include <memory>
#include <random>

enum class Stare { Meniu, Simulare, Explicatie, Benchmark };

int main() {
    // The game always renders at a virtual Config::Latime x Config::Inaltime and is scaled (letterboxed) to the window,
    // so it fits any screen, can be resized, and the mouse stays accurate.
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "Collision Lab");
    {
        int monitor = GetCurrentMonitor();
        Vector2 origine = GetMonitorPosition(monitor);
        int mw = GetMonitorWidth(monitor), mh = GetMonitorHeight(monitor);
        float scaraInitiala = std::min(1.0f, std::min((mw - 80) / (float)Config::Latime, (mh - 140) / (float)Config::Inaltime));  // room for title bar + taskbar
        int w = (int)(Config::Latime * scaraInitiala), h = (int)(Config::Inaltime * scaraInitiala);
        SetWindowSize(w, h);
        SetWindowPosition((int)origine.x + (mw - w) / 2, (int)origine.y + std::max(30, (mh - h) / 2 - 20));
        SetWindowMinSize(640, 360);
    }
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_BILINEAR);   // smoother text when scaled down
    SetExitKey(0);                        // ESC goes back to the menu instead of closing the window
    SetTargetFPS(60);
    Texture2D cerc = CreeazaTexturaCerc();
    RenderTexture2D trail = LoadRenderTexture(Config::Latime, Config::Inaltime);

    std::mt19937 gen{std::random_device{}()};
    Meniu meniu(gen);
    Explicator explicator;
    std::unique_ptr<Benchmark> benchmark;
    Obiecte obiecte;
    SistemColiziuni sistem(Config::Latime, Config::Inaltime, Config::MarimeCelula);

    Stare stare = Stare::Meniu;
    Algoritm mod = Algoritm::Grid;
    bool pauza = false;
    bool asteaptaEliberare = false;       // ignore the click that pressed START until it's released
    float acumulator = 0.0f, spawnAcum = 0.0f;
    float razaCamp = Config::RazaCampImplicit;

    Comutatoare comut;                    // heatmap / pairs / speed colours / trails / gravity / bounce
    const float restitutii[4] = {1.0f, 0.8f, 0.5f, 0.2f};
    int idxRestitutie = 0;
    Istoric istoric;
    StatisticiAlgoritm statistici[4];

    Camera2D camera{};
    auto actualizeazaCamera = [&] {
        float sw = (float)GetScreenWidth(), sh = (float)GetScreenHeight();
        float sc = std::min(sw / Config::Latime, sh / Config::Inaltime);
        float ox = (sw - Config::Latime * sc) / 2.0f, oy = (sh - Config::Inaltime * sc) / 2.0f;
        camera.zoom = sc;
        camera.offset = Vector2{ox, oy};
        camera.target = Vector2{0, 0};
        SetMouseOffset((int)-ox, (int)-oy);                 // mouse coordinates in virtual (1920x1080) space
        SetMouseScale(1.0f / sc, 1.0f / sc);
    };
    auto incepe = [&] { BeginDrawing(); ClearBackground(Color{8, 10, 22, 255}); BeginMode2D(camera); };
    auto termina = [&] { EndMode2D(); EndDrawing(); };      // raylib also reads the keyboard here

    // raylib reads the keyboard inside EndDrawing(). Leaving a screen with a bare "continue" would skip that,
    // and the ESC that closed it would still look pressed in the next screen (and quit the menu).
    auto cadruGol = [&] { incepe(); DeseneazaFundal(); termina(); };

    while (!WindowShouldClose()) {
        float dtFrame = std::min(GetFrameTime(), 0.25f);
        actualizeazaCamera();
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

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
                comut = Comutatoare{};
                idxRestitutie = 0;
                istoric = Istoric{};
                for (auto& s : statistici) s = StatisticiAlgoritm{};
                sistem.SetInregistreazaCandidati(false);
                stare = Stare::Simulare;
            } else if (r == RezultatMeniu::Explica) {
                explicator.Seteaza(meniu.setari.algoritm);
                stare = Stare::Explicatie;
            } else if (r == RezultatMeniu::Benchmark) {
                benchmark = std::make_unique<Benchmark>(meniu.setari.threads);
                stare = Stare::Benchmark;
            }
            incepe();
                meniu.Deseneaza(cerc);
            termina();
            continue;
        }

        // ================= "how it works" =================
        if (stare == Stare::Explicatie) {
            if (!explicator.Update(dtFrame)) { meniu.Reintra(); stare = Stare::Meniu; cadruGol(); continue; }
            incepe();
                explicator.Deseneaza(cerc);
            termina();
            continue;
        }

        // ================= benchmark =================
        if (stare == Stare::Benchmark) {
            if (!benchmark->Update()) { benchmark.reset(); meniu.Reintra(); stare = Stare::Meniu; cadruGol(); continue; }
            incepe();
                benchmark->Deseneaza();
            termina();
            continue;
        }

        // ================= simulation: input =================
        if (IsKeyPressed(KEY_ESCAPE)) { meniu.Reintra(); stare = Stare::Meniu; cadruGol(); continue; }
        if (IsKeyPressed(KEY_SPACE))  pauza = !pauza;
        if (IsKeyPressed(KEY_ONE))    mod = Algoritm::BruteForce;
        if (IsKeyPressed(KEY_TWO))    mod = Algoritm::Grid;
        if (IsKeyPressed(KEY_THREE))  mod = Algoritm::Quadtree;
        if (IsKeyPressed(KEY_FOUR))   mod = Algoritm::SweepAndPrune;
        if (IsKeyPressed(KEY_T))      sistem.SetMultithreading(!sistem.Multithreading());
        if (IsKeyPressed(KEY_C))      obiecte.EliminaUltimele(obiecte.Size());

        if (IsKeyPressed(KEY_H)) comut.heatmap = !comut.heatmap;
        if (IsKeyPressed(KEY_P)) { comut.perechi = !comut.perechi; sistem.SetInregistreazaCandidati(comut.perechi); }
        if (IsKeyPressed(KEY_V)) comut.viteza = !comut.viteza;
        if (IsKeyPressed(KEY_G)) { comut.gravitatie = !comut.gravitatie; obiecte.gravitatie = comut.gravitatie ? Config::Gravitatie : 0.0f; }
        if (IsKeyPressed(KEY_B)) { idxRestitutie = (idxRestitutie + 1) % 4; comut.restitutie = restitutii[idxRestitutie]; obiecte.restitutie = comut.restitutie; }
        if (IsKeyPressed(KEY_K)) {
            comut.trails = !comut.trails;
            if (comut.trails) { BeginTextureMode(trail); DeseneazaFundal(); EndTextureMode(); }   // start from a clean canvas
        }
        obiecte.gravitatie = comut.gravitatie ? Config::Gravitatie : 0.0f;       // also after "clear", which keeps the struct
        obiecte.restitutie = comut.restitutie;

        auto adauga = [&](int k) { GenereazaObiecte(obiecte, std::min(k, Config::ObiecteMax - obiecte.Size()), Config::Latime, Config::Inaltime, gen); };
        if (IsKeyPressed(KEY_UP))        adauga(Config::PasMic);
        if (IsKeyPressed(KEY_PAGE_UP))   adauga(Config::PasMare);
        if (IsKeyPressed(KEY_DOWN))      obiecte.EliminaUltimele(Config::PasMic);
        if (IsKeyPressed(KEY_PAGE_DOWN)) obiecte.EliminaUltimele(Config::PasMare);

        Vector2 mouse = GetMousePosition();
        if (IsKeyPressed(KEY_M) && obiecte.Size() < Config::ObiecteMax) GenereazaGigant(obiecte, mouse, gen);
        razaCamp = std::clamp(razaCamp + GetMouseWheelMove() * 30.0f, Config::RazaCampMin, Config::RazaCampMax);

        if (asteaptaEliberare && !IsMouseButtonDown(MOUSE_BUTTON_LEFT)) asteaptaEliberare = false;
        bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        bool lmb = IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !asteaptaEliberare;
        bool atrage = !pauza && (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || (lmb && shift));
        bool repulsie = !pauza && lmb && !shift;

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
                if (atrage)   obiecte.ApplicaForta(mouse, razaCamp, -0.7f * Config::AcceleratieMouse, Config::PasFix);
                obiecte.Amortizeaza(Config::PasFix);
                obiecte.Update(Config::PasFix, Config::Latime, Config::Inaltime);
                sistem.Update(obiecte, mod);

                istoric.Adauga(sistem.UltimulMs(), mod);
                statistici[(int)mod] = {true, sistem.TimpMs(), sistem.NumarComparatii(), obiecte.Size()};
                acumulator -= Config::PasFix;
                pasi++;
            }
            if (pasi == Config::PasiMaxPerFrame) acumulator = 0.0f;   // too slow to catch up: drop the backlog
        }

        // ================= draw =================
        if (comut.trails) {
            // balls are painted into an off-screen canvas that is only faded a little each frame
            BeginTextureMode(trail);
                DrawRectangleGradientV(0, 0, Config::Latime, Config::Inaltime, Fade(Tema::FundalSus, 0.16f), Fade(Tema::FundalJos, 0.16f));
                obiecte.Deseneaza(cerc, 1.0f, comut.viteza);
            EndTextureMode();
        }

        incepe();
            if (comut.trails) DrawTextureRec(trail.texture, Rectangle{0, 0, (float)Config::Latime, -(float)Config::Inaltime}, Vector2{0, 0}, WHITE);
            else DeseneazaFundal();
            if (comut.heatmap) sistem.DeseneazaHeatmap(obiecte);
            sistem.DeseneazaDebug(mod);
            if (comut.perechi) sistem.DeseneazaCandidati(obiecte);
            if (!comut.trails) obiecte.Deseneaza(cerc, 1.0f, comut.viteza);
            DeseneazaCamp(mouse, razaCamp, repulsie || atrage, atrage);

            DateHud hud;
            hud.algoritm = mod;
            hud.comparatii = sistem.NumarComparatii();
            hud.perechi = sistem.NumarPerechi();
            hud.obiecte = obiecte.Size();
            hud.nrThreaduri = sistem.NrThreaduri();
            hud.fps = GetFPS();
            hud.timpMs = sistem.TimpMs();
            hud.threads = sistem.Multithreading();
            hud.pauza = pauza;
            hud.istoric = &istoric;
            hud.statistici = statistici;
            hud.comut = comut;
            DeseneazaHud(hud);
            DeseneazaAjutor();
            if (pauza) DeseneazaPauza();
        termina();            // raylib itself saves a screenshot on F12
    }

    UnloadRenderTexture(trail);
    UnloadTexture(cerc);
    CloseWindow();
    return 0;
}