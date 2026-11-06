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
    void Panou(float x, float y, float w, float h) {
        DrawRectangleRounded(Rectangle{x, y, w, h}, 0.06f, 8, Fade(BLACK, 0.45f));
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
    const float x = 16, w = 340;

    // ---------- panel 1: live numbers + frame-time graph ----------
    float y = 16, h = 330;
    Panou(x, y, w, h);
    DrawRectangleRounded(Rectangle{x, y + 14, 5, 34}, 1.0f, 4, accent);
    DrawText(NumeAlgoritm(d.algoritm), (int)x + 20, (int)y + 16, 28, accent);
    if (d.layout) DrawText(d.layout, (int)x + 20, (int)y + 46, 14, Tema::TextGri);

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

    DrawText("Collision time", (int)x + 20, (int)ry + 4, 20, Tema::TextGri);
    TextDreapta(TextFormat("%.2f ms", d.timpMs), x + w - 20, ry + 4, 20, WHITE);
    float ratio = std::min(1.0f, d.timpMs / 8.3f);             // 8.3 ms = budget of one 120 Hz step
    Color barColor = ratio < 0.4f ? Color{106, 230, 160, 255} : ratio < 0.75f ? Color{255, 200, 90, 255} : Tema::Rosu;
    Rectangle bar = {x + 20, ry + 32, w - 40, 8};
    DrawRectangleRounded(bar, 1.0f, 6, Fade(WHITE, 0.12f));
    if (ratio > 0.01f) DrawRectangleRounded(Rectangle{bar.x, bar.y, bar.width * ratio, bar.height}, 1.0f, 6, barColor);

    // graph: one line segment per step, coloured by the algorithm that produced it
    if (d.istoric) {
        const Istoric& ist = *d.istoric;
        float mx = 1.0f;
        for (int k = 0; k < ist.numar; k++) mx = std::max(mx, ist.ms[k]);
        mx *= 1.1f;

        DrawText(TextFormat("ms per step, last 2 s (max %.1f)", mx), (int)x + 20, (int)ry + 50, 14, Tema::TextGri);
        Rectangle g = {x + 20, ry + 68, w - 40, 50};
        DrawRectangleRounded(g, 0.1f, 6, Fade(WHITE, 0.06f));
        float dx = g.width / (Istoric::N - 1);
        auto valoare = [&](int k) { return (ist.idx - ist.numar + k + Istoric::N) % Istoric::N; };
        for (int k = 0; k + 1 < ist.numar; k++) {
            int i0 = valoare(k), i1 = valoare(k + 1);
            float x0 = g.x + g.width - (ist.numar - 1 - k) * dx, x1 = x0 + dx;
            float y0 = g.y + g.height - ist.ms[i0] / mx * g.height;
            float y1 = g.y + g.height - ist.ms[i1] / mx * g.height;
            DrawLineEx(Vector2{x0, y0}, Vector2{x1, y1}, 2.0f, CuloareAlgoritm((Algoritm)ist.algo[i1]));
        }
    }

    // ---------- panel 2: last measurement of every algorithm ----------
    float y2 = y + h + 12, h2 = 146;
    Panou(x, y2, w, h2);
    DrawText("Last measured (keys 1-4)", (int)x + 20, (int)y2 + 10, 18, Tema::TextGri);
    for (int a = 0; a < 4; a++) {
        float py = y2 + 38 + a * 26;
        Algoritm alg = (Algoritm)a;
        const StatisticiAlgoritm& st = d.statistici[a];
        bool curent = (alg == d.algoritm);
        float alpha = (!st.valid) ? 0.30f : (st.obiecte == d.obiecte ? 1.0f : 0.45f);   // dim = measured at another object count
        Color c = CuloareAlgoritm(alg);
        if (curent) DrawRectangleRounded(Rectangle{x + 8, py - 3, w - 16, 26}, 0.4f, 6, Fade(c, 0.18f));
        DrawText(NumeAlgoritm(alg), (int)x + 20, (int)py, 18, Fade(c, alpha));
        if (st.valid) {
            if (st.obiecte != d.obiecte) DrawText(TextFormat("@%d", st.obiecte), (int)x + 20 + MeasureText(NumeAlgoritm(alg), 18) + 6, (int)py + 3, 14, Fade(Tema::TextGri, alpha));
            TextDreapta(TextFormat("%.2f ms", st.ms), x + 232, py, 18, Fade(WHITE, alpha));
            TextDreapta(TextFormat("%d", st.comparatii), x + w - 16, py, 18, Fade(Tema::TextGri, alpha));
        } else {
            TextDreapta("-", x + w - 16, py, 18, Fade(Tema::TextGri, alpha));
        }
    }

    // ---------- panel 3: toggles ----------
    float y3 = y2 + h2 + 12, h3 = 134;
    Panou(x, y3, w, h3);
    struct Chip { const char* text; bool activ; };
    Chip chips[6] = {
        {"H  Heatmap", d.comut.heatmap},       {"P  Pairs", d.comut.perechi},
        {"V  Speed colours", d.comut.viteza},  {"K  Trails", d.comut.trails},
        {"G  Gravity", d.comut.gravitatie},    {nullptr, d.comut.restitutie < 0.999f},
    };
    for (int i = 0; i < 6; i++) {
        Rectangle r = {x + 14 + (i % 2) * 160.0f, y3 + 12 + (i / 2) * 38.0f, 152, 30};
        DrawRectangleRounded(r, 0.5f, 8, chips[i].activ ? Fade(accent, 0.38f) : Fade(WHITE, 0.07f));
        const char* t = chips[i].text ? chips[i].text : TextFormat("B  Bounce %.1f", d.comut.restitutie);
        DrawText(t, (int)r.x + 12, (int)r.y + 6, 18, chips[i].activ ? WHITE : Tema::TextGri);
    }
}

void DeseneazaAjutor() {
    const char* l1 = "LMB repel   |   Shift+LMB or MMB attract   |   RMB spawn   |   M giant ball   |   L layout presets   |   Wheel field size   |   UP/DOWN +-100   PGUP/PGDN +-1000   |   C clear";
    const char* l2 = "1-4 algorithm   |   T threads   |   H heatmap   P pairs   V speed colours   K trails   G gravity   B bounce   |   SPACE pause   |   F12 screenshot   |   ESC menu";
    int size = 17;
    int w = std::max(MeasureText(l1, size), MeasureText(l2, size));
    float x = (Config::Latime - w) / 2.0f, y = Config::Inaltime - 72.0f;
    DrawRectangleRounded(Rectangle{x - 20, y - 8, (float)w + 40, 56}, 0.3f, 8, Fade(BLACK, 0.40f));
    DrawText(l1, (int)x, (int)y, size, Tema::TextGri);
    DrawText(l2, (int)x, (int)y + 26, size, Tema::TextGri);
}

void DeseneazaCamp(Vector2 mouse, float raza, bool activ, bool atragere) {
    Color c = atragere ? Color{255, 200, 90, 255} : WHITE;
    DrawCircleV(mouse, raza, Fade(c, activ ? 0.07f : 0.025f));
    DrawRing(mouse, raza - 2.5f, raza, 0, 360, 72, Fade(c, activ ? 0.60f : 0.18f));
    if (activ) {
        // pulse travels outwards when repelling, inwards when attracting
        float fase = (float)fmod(GetTime() * 1.6, 1.0);
        float r = raza * (atragere ? 1.0f - fase : fase);
        DrawRing(mouse, std::max(0.0f, r - 2.0f), r, 0, 360, 72, Fade(c, 0.35f * (1.0f - fase)));
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
    const float w = 640, h = 54;
    return Rectangle{(Config::Latime - w) / 2.0f, 405.0f + i * 66.0f + (i >= 3 ? 14.0f : 0.0f), w, h};
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

    auto actiune = [&](int rand) {
        if (rand == 3) return RezultatMeniu::Start;
        if (rand == 4) return RezultatMeniu::Explica;
        if (rand == 5) return RezultatMeniu::Benchmark;
        return RezultatMeniu::Iesire;
    };

    if (IsKeyPressed(KEY_ESCAPE)) return RezultatMeniu::Iesire;
    if (IsKeyPressed(KEY_UP))    selectat = (selectat + NrRanduri - 1) % NrRanduri;
    if (IsKeyPressed(KEY_DOWN))  selectat = (selectat + 1) % NrRanduri;
    if (IsKeyPressed(KEY_LEFT))  Schimba(selectat, -1);
    if (IsKeyPressed(KEY_RIGHT)) Schimba(selectat, +1);
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) || IsKeyPressed(KEY_SPACE)) {
        if (selectat >= 3) return actiune(selectat);
        Schimba(selectat, +1);
    }

    for (int i = 0; i < NrRanduri; i++) {
        if (!CheckCollisionPointRec(m, RandRect(i))) continue;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selectat = i;
            if (i < 3) Schimba(i, +1);
            else return actiune(i);
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
    Color accent = CuloareAlgoritm(setari.algoritm);

    float bob = sinf(timp * 1.6f) * 6.0f;
    TextUmbra("COLLISION LAB", cx, 120 + bob, 110, Fade(WHITE, a));
    TextCentrat("Broad-phase collision detection, visualised", cx, 245, 30, Fade(Tema::TextGri, a));
    TextCentrat("Brute Force   -   Spatial Grid   -   Quadtree   -   Sweep and Prune", cx, 288, 22, Fade(accent, a));

    const char* etichete[3] = {"Algorithm", "Objects", "Multithreading"};
    const char* butoane[4] = {"START", "HOW IT WORKS", "BENCHMARK", "QUIT"};
    for (int i = 0; i < NrRanduri; i++) {
        Rectangle r = RandRect(i);
        bool sel = (i == selectat);

        if (i >= 3) {
            bool principal = (i == 3);
            Color fill = principal ? Fade(accent, (sel ? 1.0f : 0.70f) * a) : Fade(WHITE, (sel ? 0.16f : 0.06f) * a);
            Color text = principal ? Fade(Color{12, 16, 34, 255}, a) : Fade(WHITE, a);
            DrawRectangleRounded(r, 0.35f, 10, fill);
            TextCentrat(butoane[i - 3], cx, r.y + (principal ? 10 : 13), principal ? 32 : 28, text);
        } else {
            DrawRectangleRounded(r, 0.35f, 10, Fade(WHITE, (sel ? 0.16f : 0.06f) * a));
            if (sel) DrawRectangleRounded(Rectangle{r.x, r.y + 10, 6, r.height - 20}, 1.0f, 4, Fade(accent, a));
            DrawText(etichete[i], (int)r.x + 28, (int)r.y + 14, 26, Fade(sel ? WHITE : Tema::TextGri, a));

            const char* val = (i == 0) ? NumeAlgoritm(setari.algoritm)
                            : (i == 1) ? TextFormat("%d", setari.NrObiecte())
                                       : (setari.threads ? "ON" : "OFF");
            const char* txt = TextFormat(sel ? "<   %s   >" : "%s", val);
            TextDreapta(txt, r.x + r.width - 28, r.y + 14, 26, Fade(sel ? accent : WHITE, a));
        }
    }

    TextCentrat("UP / DOWN select    LEFT / RIGHT change    ENTER confirm    (mouse works too: left click = next, right click = previous)",
                cx, Config::Inaltime - 56.0f, 20, Fade(Tema::TextGri, a));
}