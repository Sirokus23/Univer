#include "Modes.h"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <sstream>

using namespace Explicatie;

namespace {
    const Color Neutru = {140, 155, 195, 255};
    const Color Galben = {255, 214, 102, 255};
    const Color Cyan   = {84, 220, 240, 255};
    const Color Albastru = {84, 150, 255, 255};
    const Color Verde  = {106, 230, 160, 255};

    std::string Fmt(const char* f, ...) {
        char b[320];
        va_list a;
        va_start(a, f);
        vsnprintf(b, sizeof b, f, a);
        va_end(a);
        return b;
    }

    // greedy word wrap using the real text width
    std::vector<std::string> Imparte(const std::string& text, int size, int latime) {
        std::vector<std::string> linii;
        std::istringstream in(text);
        std::string cuvant, linie;
        while (in >> cuvant) {
            std::string incercare = linie.empty() ? cuvant : linie + " " + cuvant;
            if (!linie.empty() && MeasureText(incercare.c_str(), size) > latime) {
                linii.push_back(linie);
                linie = cuvant;
            } else {
                linie = incercare;
            }
        }
        if (!linie.empty()) linii.push_back(linie);
        return linii;
    }

    float TextImpartit(const std::string& text, float x, float y, int size, int latime, Color c, float spatiere) {
        for (const std::string& l : Imparte(text, size, latime)) {
            DrawText(l.c_str(), (int)x, (int)y, size, c);
            y += spatiere;
        }
        return y;
    }

    void Inel(Vector2 p, float raza, float grosime, Color c) {
        DrawRing(p, raza + 3, raza + 3 + grosime, 0, 360, 48, c);
    }

    // arrow from the edge of ball A to the edge of ball B; fractie < 1 = still growing
    void Sageata(Vector2 a, Vector2 b, float ra, float rb, float grosime, Color c, float fractie = 1.0f) {
        float dx = b.x - a.x, dy = b.y - a.y;
        float d = sqrtf(dx * dx + dy * dy);
        if (d - ra - rb <= 1.0f) return;
        float ux = dx / d, uy = dy / d;
        Vector2 s = {a.x + ux * ra, a.y + uy * ra};
        Vector2 e = {b.x - ux * rb, b.y - uy * rb};
        Vector2 vf = {s.x + (e.x - s.x) * fractie, s.y + (e.y - s.y) * fractie};
        DrawLineEx(s, vf, grosime, c);
        if (fractie < 0.999f) return;
        float ah = 10.0f + grosime * 2.0f, ca = cosf(0.45f), sa = sinf(0.45f);
        Vector2 st = {vf.x - ah * (ux * ca - uy * sa), vf.y - ah * (uy * ca + ux * sa)};
        Vector2 dr = {vf.x - ah * (ux * ca + uy * sa), vf.y - ah * (uy * ca - ux * sa)};
        DrawLineEx(vf, st, grosime, c);
        DrawLineEx(vf, dr, grosime, c);
    }

    const float ViteziRedare[6] = {0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f};

    float IntervalPas(Algoritm a) { return a == Algoritm::BruteForce ? 0.18f : 1.0f; }

    std::vector<std::string> Descriere(Algoritm a) {
        switch (a) {
            case Algoritm::BruteForce: return {
                "Compare every ball with every other ball, one pair at a time.",
                "n balls need n(n-1)/2 comparisons, so the work grows with n squared.",
                "Dead simple and always correct, but 1000 balls already mean 499,500 tests per step."};
            case Algoritm::Grid: return {
                "Cut the world into equal square cells.",
                "1. Put every ball into the cell that contains its centre.",
                "2. Two balls can only touch if they share a cell or sit in neighbouring cells, so only those are compared. Each cell checks itself plus 4 forward neighbours, so no pair is tested twice.",
                "Fast for evenly spread balls. A cell must be at least one ball diameter wide."};
            case Algoritm::Quadtree: return {
                "Recursively split space into 4 quadrants wherever it gets crowded.",
                "1. Insert the balls. When a leaf holds too many, it splits into 4 children.",
                "2. For each ball, ask the tree for everything inside a box around it (own radius + largest radius). Only those balls are candidates.",
                "Adapts to clumps: crowded areas get fine cells, empty areas stay coarse."};
            case Algoritm::SweepAndPrune: return {
                "Project every ball onto the x axis and sort by left edge.",
                "Sweep through the sorted list: a ball can only touch the following balls that start before its own right edge.",
                "The moment one starts further right, stop: everything after it starts even later.",
                "Very cheap when the order hardly changes between frames, so it is kept sorted with insertion sort."};
        }
        return {};
    }

    const char* Complexitate(Algoritm a) {
        switch (a) {
            case Algoritm::BruteForce:    return "O(n^2)  -  every pair";
            case Algoritm::Grid:          return "~O(n)  -  evenly spread balls";
            case Algoritm::Quadtree:      return "~O(n log n)  -  adapts to clumps";
            case Algoritm::SweepAndPrune: return "~O(n) when nearly sorted";
        }
        return "";
    }
}

// =====================================================================
//  Explicator: scene + steps
// =====================================================================

Explicator::Explicator() {
    GenereazaScena();
    Seteaza(Algoritm::BruteForce);
}

void Explicator::GenereazaScena() {
    std::mt19937 gen(2024);                 // fixed seed: the demo is always the same
    std::uniform_real_distribution<float> dx(40.0f, LatimeScena - 40.0f), dy(40.0f, InaltimeScena - 40.0f);
    std::uniform_real_distribution<float> dr(16.0f, 26.0f), unghi(0.0f, 6.2831853f);

    scena = Obiecte{};
    auto liber = [&](Vector2 p, float r, float gol, int ignora) {
        for (int k = 0; k < scena.Size(); k++) {
            if (k == ignora) continue;
            float ex = p.x - scena.poz[k].x, ey = p.y - scena.poz[k].y;
            float s = r + scena.raza[k] + gol;
            if (ex * ex + ey * ey < s * s) return false;
        }
        return true;
    };

    for (int incercari = 0; scena.Size() < NrBile - 5 && incercari < 20000; incercari++) {
        Vector2 p = {dx(gen), dy(gen)};
        float r = dr(gen);
        if (liber(p, r, 12.0f, -1)) scena.Adauga(p, {0, 0}, r, WHITE);
    }
    // a few partners placed so that they overlap an existing ball
    int partenere = 0;
    for (int incercari = 0; partenere < 5 && incercari < 20000; incercari++) {
        int a = (int)(gen() % (unsigned)scena.Size());
        float r = dr(gen), ang = unghi(gen), d = (scena.raza[a] + r) * 0.8f;
        Vector2 p = {scena.poz[a].x + cosf(ang) * d, scena.poz[a].y + sinf(ang) * d};
        if (p.x < r || p.x > LatimeScena - r || p.y < r || p.y > InaltimeScena - r) continue;
        if (!liber(p, r, 6.0f, a)) continue;
        scena.Adauga(p, {0, 0}, r, WHITE);
        partenere++;
    }

    adevarate.clear();
    for (int i = 0; i < scena.Size(); i++)
        for (int j = i + 1; j < scena.Size(); j++)
            if (scena.Ciocnire(i, j)) adevarate.emplace_back(i, j);
}

void Explicator::Seteaza(Algoritm a) {
    algoritm = a;
    pasi.clear();
    ordineGasite.clear();
    switch (a) {
        case Algoritm::BruteForce:    GenereazaBrute(); break;
        case Algoritm::Grid:          GenereazaGrid(); break;
        case Algoritm::Quadtree:      GenereazaQuadtree(); break;
        case Algoritm::SweepAndPrune: GenereazaSweep(); break;
    }
    pas = 0;
    acum = 0.0f;
    joaca = true;
    idxViteza = (a == Algoritm::BruteForce) ? 2 : 1;
}

// records the collisions of a step (keeping discovery order) and updates the cumulative counters
void Explicator::Inregistreaza(Pas& p, const std::vector<Pereche>& perechi) {
    int comp = pasi.empty() ? 0 : pasi.back().comparatii;
    p.verificate = perechi;
    for (const Pereche& pr : perechi)
        if (scena.Ciocnire(pr.first, pr.second)) ordineGasite.push_back(pr);
    p.comparatii = comp + (int)perechi.size();
    p.nrGasite = (int)ordineGasite.size();
}

void Explicator::AdaugaRezumat(int comparatii) {
    int n = scena.Size();
    Pas p;
    p.faza = 2;
    p.inserate = n;
    p.comparatii = comparatii;
    p.nrGasite = (int)ordineGasite.size();
    p.frunze = pasi.empty() ? std::vector<Rectangle>{} : pasi.back().frunze;
    p.text = Fmt("Done! %d comparisons found %d collisions. Brute force would have needed %d comparisons.",
                 comparatii, (int)ordineGasite.size(), n * (n - 1) / 2);
    pasi.push_back(p);
}

void Explicator::GenereazaBrute() {
    int n = scena.Size();
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            Pas p;
            p.faza = 1;
            p.activ = i;
            Inregistreaza(p, {{i, j}});
            float dx = scena.poz[i].x - scena.poz[j].x, dy = scena.poz[i].y - scena.poz[j].y;
            float d = sqrtf(dx * dx + dy * dy), s = scena.raza[i] + scena.raza[j];
            bool lovit = scena.Ciocnire(i, j);
            p.text = Fmt("Ball %d vs ball %d: centre distance %.1f %s radii sum %.1f  ->  %s",
                         i, j, d, lovit ? "<=" : ">", s, lovit ? "COLLISION" : "no collision");
            pasi.push_back(p);
        }
    }
    AdaugaRezumat(pasi.back().comparatii);
}

void Explicator::GenereazaGrid() {
    int n = scena.Size();
    SpatialGrid g(CelulaDemo, (int)LatimeScena, (int)InaltimeScena);     // the real grid class
    g.Build(scena.poz);
    colGrid = g.Coloane();
    rowGrid = g.Rinduri();
    celulaDe.assign(n, 0);
    for (int i = 0; i < n; i++) celulaDe[i] = g.CelulaY(scena.poz[i].y) * colGrid + g.CelulaX(scena.poz[i].x);

    for (int k = 0; k < n; k++) {
        Pas p;
        p.faza = 0;
        p.activ = k;
        p.cel = celulaDe[k];
        p.inserate = k + 1;
        p.text = Fmt("Insert ball %d: its centre is in cell (%d, %d).", k, p.cel % colGrid, p.cel / colGrid);
        pasi.push_back(p);
    }

    static const int ddx[4] = {1, -1, 0, 1}, ddy[4] = {0, 1, 1, 1};     // the 4 "forward" neighbours
    for (int c = 0; c < g.NrCelule(); c++) {
        int b0 = g.Inceput(c), b1 = g.Sfarsit(c);
        if (b0 == b1) continue;
        std::vector<Pereche> perechi;
        std::vector<int> vecine;
        int inauntru = 0;
        for (int k = b0; k < b1; k++)
            for (int m = k + 1; m < b1; m++) { perechi.emplace_back(g.Element(k), g.Element(m)); inauntru++; }
        for (int d = 0; d < 4; d++) {
            int nx = c % colGrid + ddx[d], ny = c / colGrid + ddy[d];
            if (nx < 0 || nx >= colGrid || ny >= rowGrid) continue;
            int nc = ny * colGrid + nx;
            if (g.Inceput(nc) == g.Sfarsit(nc)) continue;
            vecine.push_back(nc);
            for (int k = b0; k < b1; k++)
                for (int m = g.Inceput(nc); m < g.Sfarsit(nc); m++) perechi.emplace_back(g.Element(k), g.Element(m));
        }
        if (perechi.empty()) continue;      // a lonely ball in an empty neighbourhood: nothing to compare

        Pas p;
        p.faza = 1;
        p.cel = c;
        p.vecine = vecine;
        p.inserate = n;
        Inregistreaza(p, perechi);
        p.text = Fmt("Cell (%d, %d): %d tests inside the cell + %d with forward neighbours = %d comparisons.",
                     c % colGrid, c / colGrid, inauntru, (int)perechi.size() - inauntru, (int)perechi.size());
        pasi.push_back(p);
    }
    AdaugaRezumat(pasi.back().comparatii);
}

void Explicator::GenereazaQuadtree() {
    int n = scena.Size();
    Quadtree q(Rectangle{0, 0, LatimeScena, InaltimeScena}, Config::QuadCapacitate, Config::QuadAdancimeMax);   // the real quadtree
    std::vector<Rectangle> frunze;
    size_t inainte = 1;
    float razaMax = 0.0f;
    for (int k = 0; k < n; k++) {
        q.Insert(k, scena.poz[k]);
        razaMax = std::max(razaMax, scena.raza[k]);
        q.Frunze(frunze);
        Pas p;
        p.faza = 0;
        p.activ = k;
        p.inserate = k + 1;
        p.frunze = frunze;
        p.text = frunze.size() > inainte ? Fmt("Insert ball %d: its leaf was full, so it splits into 4 quadrants.", k)
                                         : Fmt("Insert ball %d into the leaf that contains its centre.", k);
        inainte = frunze.size();
        pasi.push_back(p);
    }

    for (int i = 0; i < n; i++) {
        float r = scena.raza[i] + razaMax;
        Rectangle caseta = {scena.poz[i].x - r, scena.poz[i].y - r, 2 * r, 2 * r};
        std::vector<int> cand;
        q.Interogare(caseta, cand);
        std::vector<Pereche> perechi;
        for (int j : cand) if (j > i) perechi.emplace_back(i, j);

        Pas p;
        p.faza = 1;
        p.activ = i;
        p.inserate = n;
        p.frunze = frunze;
        p.zona = caseta;
        p.candidati = cand;
        Inregistreaza(p, perechi);
        p.text = Fmt("Ball %d: the query box touches %d balls. Only the %d with a higher index are tested.",
                     i, (int)cand.size(), (int)perechi.size());
        pasi.push_back(p);
    }
    AdaugaRezumat(pasi.back().comparatii);
}

void Explicator::GenereazaSweep() {
    int n = scena.Size();
    auto stanga = [&](int i) { return scena.poz[i].x - scena.raza[i]; };
    ordineSweep.resize(n);
    std::iota(ordineSweep.begin(), ordineSweep.end(), 0);
    std::stable_sort(ordineSweep.begin(), ordineSweep.end(), [&](int a, int b) { return stanga(a) < stanga(b); });

    Pas sortare;
    sortare.faza = 0;
    sortare.text = "Sort all balls by their left edge (x - radius). The strip below shows the sorted order.";
    pasi.push_back(sortare);

    for (int k = 0; k < n; k++) {
        int i = ordineSweep[k];
        float dreapta = scena.poz[i].x + scena.raza[i];
        std::vector<Pereche> perechi;
        std::vector<int> cand;
        int oprit = -1;
        for (int m = k + 1; m < n; m++) {
            int j = ordineSweep[m];
            if (stanga(j) > dreapta) { oprit = j; break; }
            perechi.emplace_back(i, j);
            cand.push_back(j);
        }
        Pas p;
        p.faza = 1;
        p.activ = i;
        p.pozSortat = k;
        p.oprit = oprit;
        p.x0 = stanga(i);
        p.x1 = dreapta;
        p.candidati = cand;
        Inregistreaza(p, perechi);
        if (oprit >= 0)
            p.text = Fmt("Ball %d spans x = %.0f..%.0f. %d ball(s) start inside that range and are tested. Ball %d starts at x = %.0f, so the sweep stops.",
                         i, p.x0, p.x1, (int)perechi.size(), oprit, stanga(oprit));
        else
            p.text = Fmt("Ball %d spans x = %.0f..%.0f. %d ball(s) start inside that range and are tested. The end of the list is reached.",
                         i, p.x0, p.x1, (int)perechi.size());
        pasi.push_back(p);
    }
    AdaugaRezumat(pasi.back().comparatii);
}

// =====================================================================
//  Explicator: input / playback
// =====================================================================

bool Explicator::Update(float dt) {
    if (IsKeyPressed(KEY_ESCAPE)) return false;

    if (IsKeyPressed(KEY_ONE))   Seteaza(Algoritm::BruteForce);
    if (IsKeyPressed(KEY_TWO))   Seteaza(Algoritm::Grid);
    if (IsKeyPressed(KEY_THREE)) Seteaza(Algoritm::Quadtree);
    if (IsKeyPressed(KEY_FOUR))  Seteaza(Algoritm::SweepAndPrune);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        for (int i = 0; i < 4; i++)
            if (CheckCollisionPointRec(GetMousePosition(), Tab(i))) Seteaza((Algoritm)i);

    int ultim = (int)pasi.size() - 1;
    if (IsKeyPressed(KEY_SPACE)) {
        if (pas >= ultim) { pas = 0; acum = 0; joaca = true; }
        else joaca = !joaca;
    }
    if (IsKeyPressed(KEY_R))     { pas = 0; acum = 0; joaca = true; }
    if (IsKeyPressed(KEY_RIGHT)) { pas = std::min(pas + 1, ultim); acum = 0; joaca = false; }
    if (IsKeyPressed(KEY_LEFT))  { pas = std::max(pas - 1, 0); acum = 0; joaca = false; }
    if (IsKeyPressed(KEY_UP))    idxViteza = std::min(idxViteza + 1, 5);
    if (IsKeyPressed(KEY_DOWN))  idxViteza = std::max(idxViteza - 1, 0);

    if (joaca) {
        acum += dt * ViteziRedare[idxViteza];
        float interval = IntervalPas(algoritm);
        while (acum >= interval && pas < ultim) { acum -= interval; pas++; }
        if (pas >= ultim) { joaca = false; acum = 0; }
    }
    return true;
}

// =====================================================================
//  Explicator: drawing
// =====================================================================

void Explicator::DeseneazaBila(const Texture2D& cerc, int i, Color c, float alpha) const {
    Vector2 p = P(i);
    float r = scena.raza[i];
    DrawTexturePro(cerc, Rectangle{0, 0, (float)cerc.width, (float)cerc.height},
                   Rectangle{p.x - r, p.y - r, 2 * r, 2 * r}, Vector2{0, 0}, 0.0f, Fade(c, alpha));
    const char* t = TextFormat("%d", i);
    DrawText(t, (int)(p.x - MeasureText(t, 18) / 2.0f), (int)(p.y - 9), 18, Fade(Color{12, 16, 34, 255}, alpha));
}

std::vector<char> Explicator::BileRosii(const Pas& p) const {
    std::vector<char> rosu(scena.Size(), 0);
    for (int k = 0; k < p.nrGasite; k++) rosu[ordineGasite[k].first] = rosu[ordineGasite[k].second] = 1;
    return rosu;
}

void Explicator::DeseneazaBrute(const Texture2D& cerc, const Pas& p, float frac) const {
    int n = scena.Size(), i = p.activ;
    int j = (p.faza == 1 && !p.verificate.empty()) ? p.verificate[0].second : -1;
    Color acc = CuloareAlgoritm(Algoritm::BruteForce);
    std::vector<char> rosu = BileRosii(p);

    for (int b = 0; b < n; b++) {
        float alpha = (p.faza == 1 && b < i) ? 0.40f : 1.0f;        // balls before i are finished
        DeseneazaBila(cerc, b, rosu[b] ? Tema::Rosu : (b == i ? acc : Neutru), alpha);
    }
    if (i < 0 || j < 0) return;

    for (int jj = i + 1; jj < j; jj++) {                            // arrows already finished for this ball
        bool lovit = scena.Ciocnire(i, jj);
        Sageata(P(i), P(jj), scena.raza[i], scena.raza[jj], lovit ? 3.0f : 1.5f, lovit ? Tema::Rosu : Fade(WHITE, 0.30f));
    }
    bool lovit = scena.Ciocnire(i, j);
    Color cc = lovit ? Tema::Rosu : WHITE;
    Sageata(P(i), P(j), scena.raza[i], scena.raza[j], 4.0f, cc, std::min(1.0f, frac * 2.5f));
    Inel(P(i), scena.raza[i], 3, acc);
    Inel(P(j), scena.raza[j], 3, cc);
}

void Explicator::DeseneazaGrid(const Texture2D& cerc, const Pas& p) const {
    float cs = CelulaDemo, ox = OrigineX, oy = OrigineY;
    int n = scena.Size();
    Color acc = CuloareAlgoritm(Algoritm::Grid);
    auto rect = [&](int c) { return Rectangle{ox + (c % colGrid) * cs, oy + (c / colGrid) * cs, cs, cs}; };

    // cells in the last column / row stick out of the scene: clip them by hand
    auto taie = [&](Rectangle r) {
        float x0 = std::max(r.x, ox), y0 = std::max(r.y, oy);
        float x1 = std::min(r.x + r.width, ox + LatimeScena), y1 = std::min(r.y + r.height, oy + InaltimeScena);
        return Rectangle{x0, y0, std::max(0.0f, x1 - x0), std::max(0.0f, y1 - y0)};
    };
    if (p.cel >= 0) DrawRectangleRec(taie(rect(p.cel)), Fade(acc, 0.28f));
    if (p.faza == 1) for (int c : p.vecine) DrawRectangleRec(taie(rect(c)), Fade(Albastru, 0.22f));
    for (int x = 0; x <= colGrid && x * cs <= LatimeScena; x++) DrawLine((int)(ox + x * cs), (int)oy, (int)(ox + x * cs), (int)(oy + InaltimeScena), Fade(WHITE, 0.14f));
    for (int y = 0; y <= rowGrid && y * cs <= InaltimeScena; y++) DrawLine((int)ox, (int)(oy + y * cs), (int)(ox + LatimeScena), (int)(oy + y * cs), Fade(WHITE, 0.14f));

    std::vector<int> cnt(colGrid * rowGrid, 0);                     // how many balls each cell holds so far
    int inserate = p.faza == 0 ? p.inserate : n;
    for (int k = 0; k < inserate; k++) cnt[celulaDe[k]]++;
    for (int c = 0; c < colGrid * rowGrid; c++) {
        if (!cnt[c]) continue;
        Rectangle r = rect(c);
        DrawText(TextFormat("%d", cnt[c]), (int)r.x + 8, (int)r.y + 6, 22, Fade(WHITE, 0.55f));
    }

    std::vector<char> rosu = BileRosii(p);
    for (int b = 0; b < n; b++) {
        float alpha = (p.faza == 0 && b >= p.inserate) ? 0.22f : 1.0f;
        DeseneazaBila(cerc, b, rosu[b] ? Tema::Rosu : Neutru, alpha);
    }

    if (p.faza == 0 && p.activ >= 0) {
        Rectangle r = rect(p.cel);
        Sageata(P(p.activ), Vector2{r.x + cs / 2, r.y + cs / 2}, scena.raza[p.activ], 0.0f, 3.0f, acc);
        Inel(P(p.activ), scena.raza[p.activ], 3, acc);
    }
    if (p.faza == 1) {
        std::vector<char> inJoc(n, 0);
        for (const Pereche& pr : p.verificate) {
            bool aceeasi = celulaDe[pr.first] == celulaDe[pr.second];
            bool lovit = scena.Ciocnire(pr.first, pr.second);
            DrawLineEx(P(pr.first), P(pr.second), lovit ? 4.0f : 2.0f, lovit ? Tema::Rosu : Fade(aceeasi ? Galben : Cyan, 0.85f));
            inJoc[pr.first] = inJoc[pr.second] = 1;
        }
        for (int b = 0; b < n; b++) if (inJoc[b]) Inel(P(b), scena.raza[b], 2, Fade(WHITE, 0.8f));
    }
}

void Explicator::DeseneazaQuadtree(const Texture2D& cerc, const Pas& p) const {
    int n = scena.Size();
    float ox = OrigineX, oy = OrigineY;
    Color acc = CuloareAlgoritm(Algoritm::Quadtree);

    bool detect = (p.faza == 1);
    if (detect)
        for (const Rectangle& f : p.frunze) {
            Rectangle r = {ox + f.x, oy + f.y, f.width, f.height};
            if (CheckCollisionRecs(f, p.zona)) DrawRectangleRec(r, Fade(Albastru, 0.22f));
        }
    for (const Rectangle& f : p.frunze)
        DrawRectangleLines((int)(ox + f.x), (int)(oy + f.y), (int)f.width, (int)f.height, Fade(WHITE, 0.30f));

    if (detect) {
        float zx0 = std::max(ox + p.zona.x, ox), zy0 = std::max(oy + p.zona.y, oy);       // clip to the scene
        float zx1 = std::min(ox + p.zona.x + p.zona.width, ox + LatimeScena), zy1 = std::min(oy + p.zona.y + p.zona.height, oy + InaltimeScena);
        Rectangle z = {zx0, zy0, std::max(0.0f, zx1 - zx0), std::max(0.0f, zy1 - zy0)};
        DrawRectangleRec(z, Fade(Galben, 0.08f));
        DrawRectangleLinesEx(z, 3.0f, Galben);
    }

    std::vector<char> rosu = BileRosii(p), cand(n, 0);
    for (int c : p.candidati) cand[c] = 1;
    for (int b = 0; b < n; b++) {
        float alpha = (p.faza == 0 && b >= p.inserate) ? 0.22f : 1.0f;
        DeseneazaBila(cerc, b, rosu[b] ? Tema::Rosu : (b == p.activ ? acc : Neutru), alpha);
    }
    if (p.activ >= 0) Inel(P(p.activ), scena.raza[p.activ], 3, acc);
    if (detect) {
        for (int b = 0; b < n; b++) if (cand[b] && b != p.activ) Inel(P(b), scena.raza[b], 2, Fade(Cyan, 0.9f));
        for (const Pereche& pr : p.verificate) {
            bool lovit = scena.Ciocnire(pr.first, pr.second);
            Sageata(P(pr.first), P(pr.second), scena.raza[pr.first], scena.raza[pr.second], lovit ? 4.0f : 2.0f, lovit ? Tema::Rosu : WHITE);
        }
    }
}

void Explicator::DeseneazaSweep(const Texture2D& cerc, const Pas& p) const {
    int n = scena.Size();
    float ox = OrigineX, oy = OrigineY;
    Color acc = CuloareAlgoritm(Algoritm::SweepAndPrune);
    auto stanga = [&](int i) { return scena.poz[i].x - scena.raza[i]; };

    if (p.faza == 0)                                                // sorting step: show every left edge
        for (int i = 0; i < n; i++) DrawLine((int)(ox + stanga(i)), (int)oy, (int)(ox + stanga(i)), (int)(oy + InaltimeScena), Fade(WHITE, 0.10f));
    if (p.faza == 1) {
        Rectangle banda = {ox + p.x0, oy, p.x1 - p.x0, InaltimeScena};
        DrawRectangleRec(banda, Fade(Galben, 0.10f));
        DrawLine((int)banda.x, (int)oy, (int)banda.x, (int)(oy + InaltimeScena), Galben);
        DrawLine((int)(banda.x + banda.width), (int)oy, (int)(banda.x + banda.width), (int)(oy + InaltimeScena), Galben);
        if (p.oprit >= 0) {
            float xs = ox + stanga(p.oprit);
            DrawLineEx(Vector2{xs, oy}, Vector2{xs, oy + InaltimeScena}, 2.0f, Tema::Rosu);
        }
    }

    std::vector<char> rosu = BileRosii(p), cand(n, 0);
    for (int c : p.candidati) cand[c] = 1;
    for (int b = 0; b < n; b++)
        DeseneazaBila(cerc, b, rosu[b] ? Tema::Rosu : (b == p.activ ? acc : Neutru), 1.0f);
    if (p.activ >= 0) Inel(P(p.activ), scena.raza[p.activ], 3, acc);
    if (p.faza == 1) {
        for (int b = 0; b < n; b++) if (cand[b]) Inel(P(b), scena.raza[b], 2, Fade(Cyan, 0.9f));
        if (p.oprit >= 0) Inel(P(p.oprit), scena.raza[p.oprit], 3, Tema::Rosu);
        for (const Pereche& pr : p.verificate) {
            bool lovit = scena.Ciocnire(pr.first, pr.second);
            Sageata(P(pr.first), P(pr.second), scena.raza[pr.first], scena.raza[pr.second], lovit ? 4.0f : 2.0f, lovit ? Tema::Rosu : WHITE);
        }
    }

    // strip with the sorted order
    float sy = oy + InaltimeScena + 22.0f, bw = 52.0f;
    DrawText("sorted by left edge:", (int)ox, (int)sy - 22, 16, Tema::TextGri);
    for (int k = 0; k < n; k++) {
        Rectangle r = {ox + k * bw, sy + 2, bw - 6, 34};
        Color fill = Fade(WHITE, 0.08f);
        int nrCand = (int)p.candidati.size();
        if (p.faza == 1) {
            if (k == p.pozSortat) fill = Fade(acc, 0.8f);
            else if (k > p.pozSortat && k <= p.pozSortat + nrCand) fill = Fade(Cyan, 0.45f);
            else if (p.oprit >= 0 && k == p.pozSortat + nrCand + 1) fill = Fade(Tema::Rosu, 0.6f);
        }
        DrawRectangleRounded(r, 0.3f, 6, fill);
        const char* t = TextFormat("%d", ordineSweep[k]);
        DrawText(t, (int)(r.x + r.width / 2 - MeasureText(t, 20) / 2.0f), (int)r.y + 7, 20, WHITE);
    }
}

void Explicator::DeseneazaPanou(const Pas& p) const {
    Color acc = CuloareAlgoritm(algoritm);
    const float x = 1330, y = OrigineY, w = 540, h = InaltimeScena;
    DrawRectangleRounded(Rectangle{x, y, w, h}, 0.03f, 8, Fade(BLACK, 0.40f));
    float tx = x + 24;
    int tw = (int)w - 48;

    DrawText(NumeAlgoritm(algoritm), (int)tx, (int)y + 18, 34, acc);
    DrawText(Complexitate(algoritm), (int)tx, (int)y + 60, 20, Tema::TextGri);

    float cy = y + 98;
    for (const std::string& para : Descriere(algoritm))
        cy = TextImpartit(para, tx, cy, 19, tw, Fade(WHITE, 0.88f), 24.0f) + 6;

    // step counter + progress
    cy = std::max(cy + 8, y + 330);
    DrawLine((int)tx, (int)cy - 8, (int)(tx + tw), (int)cy - 8, Fade(WHITE, 0.15f));
    DrawText(TextFormat("Step %d / %d", pas + 1, (int)pasi.size()), (int)tx, (int)cy, 22, WHITE);
    DrawText(TextFormat("speed %.1fx", ViteziRedare[idxViteza]), (int)(tx + tw - MeasureText(TextFormat("speed %.1fx", ViteziRedare[idxViteza]), 18)), (int)cy + 3, 18, Tema::TextGri);
    Rectangle bar = {tx, cy + 32, (float)tw, 8};
    DrawRectangleRounded(bar, 1.0f, 6, Fade(WHITE, 0.12f));
    float prog = pasi.size() > 1 ? (float)pas / (float)(pasi.size() - 1) : 1.0f;
    if (prog > 0.005f) DrawRectangleRounded(Rectangle{bar.x, bar.y, bar.width * prog, bar.height}, 1.0f, 6, acc);

    // caption of the current step
    cy = TextImpartit(p.text, tx, cy + 56, 21, tw, WHITE, 27.0f);

    // numbers
    float ny = y + h - 250;
    int n = scena.Size();
    DrawText("Comparisons so far", (int)tx, (int)ny, 20, Tema::TextGri);
    DrawText(TextFormat("%d", p.comparatii), (int)(tx + tw - MeasureText(TextFormat("%d", p.comparatii), 22)), (int)ny - 1, 22, WHITE);
    DrawText("Collisions found", (int)tx, (int)ny + 28, 20, Tema::TextGri);
    DrawText(TextFormat("%d", p.nrGasite), (int)(tx + tw - MeasureText(TextFormat("%d", p.nrGasite), 22)), (int)ny + 27, 22, Tema::Rosu);
    DrawText("Brute force needs", (int)tx, (int)ny + 56, 20, Tema::TextGri);
    DrawText(TextFormat("%d", n * (n - 1) / 2), (int)(tx + tw - MeasureText(TextFormat("%d", n * (n - 1) / 2), 22)), (int)ny + 55, 22, WHITE);

    // legend
    struct Leg { Color c; const char* t; };
    std::vector<Leg> leg;
    switch (algoritm) {
        case Algoritm::BruteForce:    leg = {{acc, "ball being compared with all the others"}, {WHITE, "pair under test"}, {Fade(WHITE, 0.4f), "already tested, no collision"}, {Tema::Rosu, "collision found"}}; break;
        case Algoritm::Grid:          leg = {{acc, "current cell"}, {Albastru, "forward neighbour cells"}, {Galben, "test inside the cell"}, {Cyan, "test with a neighbour"}, {Tema::Rosu, "collision found"}}; break;
        case Algoritm::Quadtree:      leg = {{Galben, "query box around the ball"}, {Albastru, "leaves touched by the box"}, {Cyan, "candidate balls"}, {Tema::Rosu, "collision found"}}; break;
        case Algoritm::SweepAndPrune: leg = {{Galben, "x-range of the active ball"}, {Cyan, "balls inside that range (tested)"}, {Tema::Rosu, "first ball outside: sweep stops"}}; break;
    }
    float ly = y + h - 150;
    for (const Leg& l : leg) {
        DrawRectangleRounded(Rectangle{tx, ly + 3, 16, 16}, 0.3f, 4, l.c);
        DrawText(l.t, (int)tx + 28, (int)ly, 18, Tema::TextGri);
        ly += 25;
    }
}

void Explicator::Deseneaza(const Texture2D& cerc) const {
    DeseneazaFundal();
    DrawText("HOW IT WORKS", 50, 16, 36, WHITE);
    for (int i = 0; i < 4; i++) {
        Rectangle r = Tab(i);
        Algoritm a = (Algoritm)i;
        bool sel = (a == algoritm);
        DrawRectangleRounded(r, 0.4f, 8, sel ? Fade(CuloareAlgoritm(a), 0.85f) : Fade(WHITE, 0.07f));
        const char* t = TextFormat("%d  %s", i + 1, NumeAlgoritm(a));
        DrawText(t, (int)(r.x + r.width / 2 - MeasureText(t, 20) / 2.0f), (int)r.y + 10, 20, sel ? Color{12, 16, 34, 255} : Tema::TextGri);
    }

    DrawRectangleRounded(Rectangle{OrigineX, OrigineY, LatimeScena, InaltimeScena}, 0.015f, 8, Fade(BLACK, 0.30f));
    const Pas& p = pasi[pas];
    float frac = joaca ? std::min(1.0f, acum / IntervalPas(algoritm)) : 1.0f;
    switch (algoritm) {
        case Algoritm::BruteForce:    DeseneazaBrute(cerc, p, frac); break;
        case Algoritm::Grid:          DeseneazaGrid(cerc, p); break;
        case Algoritm::Quadtree:      DeseneazaQuadtree(cerc, p); break;
        case Algoritm::SweepAndPrune: DeseneazaSweep(cerc, p); break;
    }
    DeseneazaPanou(p);

    const char* h = "SPACE play / pause     LEFT / RIGHT step back / forward     UP / DOWN speed     R restart     1-4 or click a tab: algorithm     ESC back to menu";
    int hw = MeasureText(h, 18);
    float hx = (Config::Latime - hw) / 2.0f, hy = Config::Inaltime - 46.0f;
    DrawRectangleRounded(Rectangle{hx - 20, hy - 8, (float)hw + 40, 36}, 0.5f, 8, Fade(BLACK, 0.40f));
    DrawText(h, (int)hx, (int)hy, 18, Tema::TextGri);
}

// =====================================================================
//  Benchmark
// =====================================================================

constexpr int Benchmark::Numere[5];

Benchmark::Benchmark(bool t, float ms, int pMin, int pMax)
    : threads(t), msTinta(ms), pasiMin(pMin), pasiMax(pMax),
      sistem(Config::Latime, Config::Inaltime, Config::MarimeCelula) {
    for (int n : Numere)
        for (int a = 0; a < 4; a++) joburi.push_back({(Algoritm)a, n});
}

void Benchmark::PasUnic() {
    const Job& j = joburi[idx];
    if (!pregatit) {
        obiecte = Obiecte{};
        std::mt19937 gen(1000 + j.n);                                           // same layout for every algorithm
        GenereazaObiecte(obiecte, j.n, Config::Latime, Config::Inaltime, gen, 0.4f);   // small balls so 5000 fit
        sistem = SistemColiziuni(Config::Latime, Config::Inaltime, Config::MarimeCelula);
        sistem.SetMultithreading(threads);
        incalzire = 5;
        pasi = 0;
        sumMs = sumComp = sumPer = 0.0;
        pregatit = true;
    }

    obiecte.Update(Config::PasFix, Config::Latime, Config::Inaltime);
    sistem.Update(obiecte, j.a);
    if (incalzire > 0) { incalzire--; return; }                                 // warm-up steps are not measured

    sumMs += sistem.UltimulMs();
    sumComp += sistem.NumarComparatii();
    sumPer += sistem.NumarPerechi();
    pasi++;
    if ((sumMs >= msTinta && pasi >= pasiMin) || pasi >= pasiMax) {
        rezultate.push_back({j.a, j.n, pasi, (float)(sumMs / pasi), (float)(sumComp / pasi), (float)(sumPer / pasi)});
        idx++;
        pregatit = false;
        if (Terminat()) salvat = Salveaza("benchmark_results.csv");
    }
}

bool Benchmark::Update() {
    if (IsKeyPressed(KEY_ESCAPE)) return false;
    if (!Terminat()) {
        double t0 = GetTime();
        do { PasUnic(); } while (!Terminat() && GetTime() - t0 < 0.012);       // keep the window responsive
    }
    return true;
}

bool Benchmark::Salveaza(const char* fisier) const {
    std::ofstream f(fisier);
    if (!f) return false;
    f << "algorithm,objects,threads,steps,avg_ms_per_step,avg_comparisons,avg_collisions\n";
    for (const RezultatBench& r : rezultate)
        f << NumeAlgoritm(r.algoritm) << ',' << r.n << ',' << (threads ? "on" : "off") << ',' << r.pasi << ','
          << r.ms << ',' << r.comparatii << ',' << r.perechi << '\n';
    return (bool)f;
}

void Benchmark::Deseneaza() const {
    DeseneazaFundal();
    float cx = Config::Latime / 2.0f;
    const char* titlu = "BENCHMARK";
    DrawText(titlu, (int)(cx - MeasureText(titlu, 56) / 2.0f), 36, 56, WHITE);

    int total = (int)joburi.size();
    float prog = (float)idx / total;
    Rectangle bar = {cx - 400, 112, 800, 12};
    DrawRectangleRounded(bar, 1.0f, 6, Fade(WHITE, 0.12f));
    if (prog > 0.005f) DrawRectangleRounded(Rectangle{bar.x, bar.y, bar.width * prog, bar.height}, 1.0f, 6, Verde);

    const char* stare;
    if (Terminat()) stare = salvat ? "Finished  -  saved to benchmark_results.csv (ESC to go back)" : "Finished  -  could not write benchmark_results.csv (ESC to go back)";
    else stare = TextFormat("Running %d / %d:  %s with %d objects   (threads %s)", idx + 1, total, NumeAlgoritm(joburi[idx].a), joburi[idx].n, threads ? "on" : "off");
    DrawText(stare, (int)(cx - MeasureText(stare, 22) / 2.0f), 134, 22, Tema::TextGri);

    // grouped horizontal bars, log scale (brute force would otherwise dwarf everything)
    float maxMs = 0.01f;
    for (const RezultatBench& r : rezultate) maxMs = std::max(maxMs, r.ms);
    float lo = -3.0f, hi = std::max(0.0f, ceilf(log10f(maxMs)));                // 0.001 ms .. max
    const float x0 = 360, larg = 1000, grup = 148;

    for (int g = 0; g < 5; g++) {
        float gy = 190 + g * grup;
        DrawText(TextFormat("%d objects", Numere[g]), 90, (int)gy + 34, 26, WHITE);
        float minMs = 1e9f;
        for (const RezultatBench& r : rezultate) if (r.n == Numere[g]) minMs = std::min(minMs, r.ms);

        for (int a = 0; a < 4; a++) {
            float by = gy + a * 32;
            DrawText(NumeAlgoritm((Algoritm)a), (int)x0 - 20 - MeasureText(NumeAlgoritm((Algoritm)a), 18), (int)by + 4, 18, Tema::TextGri);
            DrawRectangleRounded(Rectangle{x0, by, larg, 24}, 0.4f, 6, Fade(WHITE, 0.05f));
            for (const RezultatBench& r : rezultate) {
                if (r.n != Numere[g] || r.algoritm != (Algoritm)a) continue;
                float t = std::clamp((log10f(std::max(r.ms, 0.001f)) - lo) / (hi - lo), 0.02f, 1.0f);
                DrawRectangleRounded(Rectangle{x0, by, larg * t, 24}, 0.4f, 6, CuloareAlgoritm((Algoritm)a));
                const char* v = TextFormat("%.3f ms   (%.0f comparisons)%s", r.ms, r.comparatii, r.ms == minMs ? "   <- fastest" : "");
                DrawText(v, (int)(x0 + larg * t + 10), (int)by + 4, 18, r.ms == minMs ? Verde : WHITE);
            }
        }
    }
    const char* nota = "bars use a log scale: each tick to the right is 10x slower";
    DrawText(nota, (int)(cx - MeasureText(nota, 18) / 2.0f), Config::Inaltime - 44, 18, Tema::TextGri);
}