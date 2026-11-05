#include "UI.h"
#include <algorithm>
#include <cmath>

namespace {
    void TextCentrat(const char* t, float cx, float y, int size, Color c) {
        DrawText(t, (int)(cx - MeasureText(t, size) / 2.0f), (int)y, size, c);
    }
    void TextDreapta(const char* t, float xDr, float y, int size, Color c) {
        DrawText(t, (int)(xDr - MeasureText(t, size)), (int)y, size, c);
    }
    void TextUmbra(const char* t, float cx, float y, int size, Color c) {
        TextCentrat(t, cx + 3, y + 4, size, Fade(BLACK, 0.45f * c.a / 255.0f));
        TextCentrat(t, cx, y, size, c);
    }
}

Color CuloareAlgoritm(Algoritm a) {
    switch (a) {
        case Algoritm::BruteForce:     return Color{255, 107, 129, 255};
        case Algoritm::Grid:           return Color{106, 230, 160, 255};
        case Algoritm::Quadtree:       return Color{ 84, 195, 255, 255};
        case Algoritm::SweepAndPrune:  return Color{201, 139, 255, 255};
    }
    return WHITE;
}

void DeseneazaFundal() {
    DrawRectangleGradientV(0, 0, Config::Latime, Config::Inaltime, Tema::FundalSus, Tema::FundalJos);
}

// ======================= HUD =======================

void DeseneazaHud(const DateHud& d) {
    Color accent = CuloareAlgoritm(d.algoritm);
    const float x = 16, y = 16, w = 340, h = 232;

    DrawRectangleRounded(Rectangle{x, y, w, h}, 0.08f, 8, Fade(BLACK, 0.45f));
    DrawRectangleRounded(Rectangle{x, y + 14, 5, 34}, 1.0f, 4, accent);

    DrawText(NumeAlgoritm(d.algoritm), (int)x + 20, (int)y + 16, 28, accent);

    // TextFormat only keeps a few rotating buffers, so each value is drawn right after it is formatted.
    float ry = y + 62;
    auto rand = [&](const char* eticheta, const char* valoare) {
        DrawText(eticheta, (int)x + 20, (int)ry, 20, Tema::TextGri);
        TextDreapta(valoare, x + w - 20, ry, 20, WHITE);
        ry += 26;
    };
    rand("Comparisons / step", TextFormat("%d", d.comparatii));
    rand("Collisions",         TextFormat("%d", d.perechi));
    rand("Objects",            TextFormat("%d", d.obiecte));
    rand("Threads",            d.threads ? TextFormat("%d", d.nrThreaduri) : "off");
    rand("FPS",                TextFormat("%d", d.fps));

    // collision time bar (budget: 8.3 ms per 120 Hz physics step)
    DrawText("Collision time", (int)x + 20, (int)ry + 4, 20, Tema::TextGri);
    TextDreapta(TextFormat("%.2f ms", d.timpMs), x + w - 20, ry + 4, 20, WHITE);
    float ratio = std::min(1.0f, d.timpMs / 8.3f);
    Color barColor = ratio < 0.4f ? Color{106, 230, 160, 255} : ratio < 0.75f ? Color{255, 200, 90, 255} : Color{255, 107, 129, 255};
    Rectangle bar = {x + 20, ry + 32, w - 40, 8};
    DrawRectangleRounded(bar, 1.0f, 6, Fade(WHITE, 0.12f));
    if (ratio > 0.01f) DrawRectangleRounded(Rectangle{bar.x, bar.y, bar.width * ratio, bar.height}, 1.0f, 6, barColor);
}

void DeseneazaAjutor() {
    const char* t = "LMB repel   |   RMB spawn   |   Wheel field size   |   1-4 algorithm   |   T threads   |   UP/DOWN +-100   PGUP/PGDN +-1000   |   SPACE pause   |   C clear   |   ESC menu";
    int size = 18;
    int w = MeasureText(t, size);
    float x = (Config::Latime - w) / 2.0f, y = Config::Inaltime - 46.0f;
    DrawRectangleRounded(Rectangle{x - 20, y - 8, (float)w + 40, 36}, 0.5f, 8, Fade(BLACK, 0.40f));
    DrawText(t, (int)x, (int)y, size, Tema::TextGri);
}

void DeseneazaCamp(Vector2 mouse, float raza, bool activ) {
    DrawCircleV(mouse, raza, Fade(WHITE, activ ? 0.07f : 0.025f));
    DrawRing(mouse, raza - 2.5f, raza, 0, 360, 72, Fade(WHITE, activ ? 0.60f : 0.18f));
    if (activ) {
        // expanding pulse so the repulsion is visible
        float fase = (float)fmod(GetTime() * 1.6, 1.0);
        float r = raza * fase;
        DrawRing(mouse, std::max(0.0f, r - 2.0f), r, 0, 360, 72, Fade(WHITE, 0.35f * (1.0f - fase)));
    }
}

void DeseneazaPauza() {
    DrawRectangle(0, 0, Config::Latime, Config::Inaltime, Fade(BLACK, 0.35f));
    TextUmbra("PAUSED", Config::Latime / 2.0f, Config::Inaltime / 2.0f - 50, 100, WHITE);
    TextCentrat("press SPACE to continue", Config::Latime / 2.0f, Config::Inaltime / 2.0f + 70, 26, Tema::TextGri);
}

// ======================= Start screen =======================

Meniu::Meniu(std::mt19937& gen) {
    GenereazaObiecte(fundal, 45, Config::Latime, Config::Inaltime, gen);
}

Rectangle Meniu::RandRect(int i) const {
    const float w = 640, h = 56;
    return Rectangle{(Config::Latime - w) / 2.0f, 430.0f + i * 72.0f + (i >= 3 ? 16.0f : 0.0f), w, h};
}

void Meniu::Schimba(int rand, int dir) {
    if (rand == 0) {
        int a = ((int)setari.algoritm + dir + 4) % 4;
        setari.algoritm = (Algoritm)a;
    } else if (rand == 1) {
        setari.indexNr = (setari.indexNr + dir + NrOptiuni) % NrOptiuni;
    } else if (rand == 2) {
        setari.threads = !setari.threads;
    }
}

RezultatMeniu Meniu::Update(float dt) {
    timp += dt;
    fundal.Update(dt, Config::Latime, Config::Inaltime);

    Vector2 m = GetMousePosition();
    Vector2 delta = GetMouseDelta();
    if (delta.x != 0 || delta.y != 0)
        for (int i = 0; i < NrRanduri; i++)
            if (CheckCollisionPointRec(m, RandRect(i))) selectat = i;

    if (IsKeyPressed(KEY_ESCAPE)) return RezultatMeniu::Iesire;
    if (IsKeyPressed(KEY_UP))    selectat = (selectat + NrRanduri - 1) % NrRanduri;
    if (IsKeyPressed(KEY_DOWN))  selectat = (selectat + 1) % NrRanduri;
    if (IsKeyPressed(KEY_LEFT))  Schimba(selectat, -1);
    if (IsKeyPressed(KEY_RIGHT)) Schimba(selectat, +1);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (selectat == 3) return RezultatMeniu::Start;
        if (selectat == 4) return RezultatMeniu::Iesire;
        Schimba(selectat, +1);
    }

    for (int i = 0; i < NrRanduri; i++) {
        if (!CheckCollisionPointRec(m, RandRect(i))) continue;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectat = i;
            if (i < 3) Schimba(i, +1);
            else if (i == 3) return RezultatMeniu::Start;
            else return RezultatMeniu::Iesire;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && i < 3) Schimba(i, -1);
    }
    return RezultatMeniu::Nimic;
}

void Meniu::Deseneaza(const Texture2D& cerc) const {
    DeseneazaFundal();
    fundal.Deseneaza(cerc, 0.30f);

    float a = std::min(1.0f, timp / 0.6f);          // fade-in
    float cx = Config::Latime / 2.0f;

    // title
    float bob = sinf(timp * 1.6f) * 6.0f;
    TextUmbra("COLLISION LAB", cx, 140 + bob, 110, Fade(WHITE, a));
    TextCentrat("Broad-phase collision detection, visualised", cx, 270, 30, Fade(Tema::TextGri, a));
    TextCentrat("Brute Force   -   Spatial Grid   -   Quadtree   -   Sweep and Prune", cx, 312, 22, Fade(CuloareAlgoritm(setari.algoritm), a));

    const char* etichete[3] = {"Algorithm", "Objects", "Multithreading"};
    for (int i = 0; i < NrRanduri; i++) {
        Rectangle r = RandRect(i);
        bool sel = (i == selectat);
        Color accent = CuloareAlgoritm(setari.algoritm);

        if (i == 3) {
            DrawRectangleRounded(r, 0.35f, 10, Fade(accent, (sel ? 1.0f : 0.70f) * a));
            TextCentrat("START", cx, r.y + 12, 32, Fade(Color{12, 16, 34, 255}, a));
        } else if (i == 4) {
            DrawRectangleRounded(r, 0.35f, 10, Fade(WHITE, (sel ? 0.16f : 0.06f) * a));
            TextCentrat("QUIT", cx, r.y + 14, 28, Fade(WHITE, a));
        } else {
            DrawRectangleRounded(r, 0.35f, 10, Fade(WHITE, (sel ? 0.16f : 0.06f) * a));
            if (sel) DrawRectangleRounded(Rectangle{r.x, r.y + 10, 6, r.height - 20}, 1.0f, 4, Fade(accent, a));
            DrawText(etichete[i], (int)r.x + 28, (int)r.y + 16, 26, Fade(sel ? WHITE : Tema::TextGri, a));

            const char* val = (i == 0) ? NumeAlgoritm(setari.algoritm)
                            : (i == 1) ? TextFormat("%d", setari.NrObiecte())
                                       : (setari.threads ? "ON" : "OFF");
            const char* txt = TextFormat(sel ? "<   %s   >" : "%s", val);
            TextDreapta(txt, r.x + r.width - 28, r.y + 16, 26, Fade(sel ? accent : WHITE, a));
        }
    }

    TextCentrat("UP / DOWN select    LEFT / RIGHT change    ENTER confirm    (mouse works too: left click = next, right click = previous)",
                cx, Config::Inaltime - 56.0f, 20, Fade(Tema::TextGri, a));
}