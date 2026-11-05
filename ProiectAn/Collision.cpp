#include "Collision.h"
#include <algorithm>
#include <chrono>
#include <numeric>
#include <thread>

SistemColiziuni::SistemColiziuni(int latime, int inaltime, float marimeCelula)
    : grid(marimeCelula, latime, inaltime),
      quadtree(Rectangle{0, 0, (float)latime, (float)inaltime},
               Config::QuadCapacitate, Config::QuadAdancimeMax) {
    unsigned hw = std::thread::hardware_concurrency();
    nrThreaduri = (int)std::max(1u, std::min(hw, (unsigned)Config::ThreaduriMax));
    buffere.resize(nrThreaduri);
}

// Single narrow-phase entry point: every algorithm counts comparisons the same way.
void SistemColiziuni::Testeaza(const Obiecte& o, int a, int b, BufferThread& buf) {
    buf.comparatii++;
    if (o.Ciocnire(a, b)) buf.perechi.emplace_back(a, b);
}

// ---------------- build phase (single-threaded) ----------------

void SistemColiziuni::PregatesteGrid(const Obiecte& o) {
    grid.Build(o.poz);
}

void SistemColiziuni::PregatesteQuadtree(const Obiecte& o) {
    quadtree.Clear();
    razaMax = 0.0f;
    for (int i = 0; i < o.Size(); i++) {
        quadtree.Insert(i, o.poz[i]);
        razaMax = std::max(razaMax, o.raza[i]);
    }
}

void SistemColiziuni::PregatesteSweep(const Obiecte& o) {
    int n = o.Size();
    // keep the previous order (objects barely move between steps), just fix size changes
    if ((int)ordine.size() > n)
        ordine.erase(std::remove_if(ordine.begin(), ordine.end(), [n](int i) { return i >= n; }), ordine.end());
    for (int i = (int)ordine.size(); i < n; i++) ordine.push_back(i);

    auto stanga = [&](int i) { return o.poz[i].x - o.raza[i]; };
    // insertion sort: ~O(n) on an almost-sorted array
    for (int a = 1; a < n; a++) {
        int v = ordine[a];
        float kv = stanga(v);
        int b = a - 1;
        while (b >= 0 && stanga(ordine[b]) > kv) { ordine[b + 1] = ordine[b]; b--; }
        ordine[b + 1] = v;
    }
}

// ---------------- detect phase (read-only, thread-safe) ----------------

void SistemColiziuni::UnitateBruteForce(const Obiecte& o, int i, BufferThread& buf) const {
    for (int j = i + 1; j < o.Size(); j++) Testeaza(o, i, j, buf);
}

void SistemColiziuni::UnitateGrid(const Obiecte& o, int c, BufferThread& buf) const {
    int b0 = grid.Inceput(c), b1 = grid.Sfarsit(c);
    if (b0 == b1) return;

    // inside the cell
    for (int k = b0; k < b1; k++)
        for (int m = k + 1; m < b1; m++)
            Testeaza(o, grid.Element(k), grid.Element(m), buf);

    // only 4 "forward" neighbours (right, bottom-left, bottom, bottom-right):
    // every pair of adjacent cells is visited exactly once instead of twice.
    static const int dx[4] = {1, -1, 0, 1};
    static const int dy[4] = {0, 1, 1, 1};
    int cx = c % grid.Coloane(), cy = c / grid.Coloane();
    for (int d = 0; d < 4; d++) {
        int nx = cx + dx[d], ny = cy + dy[d];
        if (nx < 0 || nx >= grid.Coloane() || ny >= grid.Rinduri()) continue;
        int nc = ny * grid.Coloane() + nx;
        for (int k = b0; k < b1; k++)
            for (int m = grid.Inceput(nc); m < grid.Sfarsit(nc); m++)
                Testeaza(o, grid.Element(k), grid.Element(m), buf);
    }
}

void SistemColiziuni::UnitateQuadtree(const Obiecte& o, int i, BufferThread& buf) const {
    // Points are stored by centre, so query (own radius + largest radius):
    // any circle that can touch this one has its centre inside that box.
    float r = o.raza[i] + razaMax;
    buf.candidati.clear();
    quadtree.Interogare(Rectangle{o.poz[i].x - r, o.poz[i].y - r, 2 * r, 2 * r}, buf.candidati);
    for (int j : buf.candidati)
        if (j > i) Testeaza(o, i, j, buf);
}

void SistemColiziuni::UnitateSweep(const Obiecte& o, int k, BufferThread& buf) const {
    int i = ordine[k];
    float dreapta = o.poz[i].x + o.raza[i];
    for (int m = k + 1; m < (int)ordine.size(); m++) {
        int j = ordine[m];
        if (o.poz[j].x - o.raza[j] > dreapta) break;      // sorted by left edge: nothing further can overlap
        Testeaza(o, i, j, buf);
    }
}

// ---------------- one full step ----------------

void SistemColiziuni::Update(Obiecte& o, Algoritm algoritm) {
    auto t0 = std::chrono::steady_clock::now();
    int n = o.Size();

    int unitati = n;
    switch (algoritm) {
        case Algoritm::Grid:           PregatesteGrid(o); unitati = grid.NrCelule(); break;
        case Algoritm::Quadtree:       PregatesteQuadtree(o); break;
        case Algoritm::SweepAndPrune:  PregatesteSweep(o); break;
        case Algoritm::BruteForce:     break;
    }

    int nt = multithreading ? nrThreaduri : 1;
    for (int t = 0; t < nt; t++) {
        buffere[t].perechi.clear();
        buffere[t].comparatii = 0;
    }

    const Obiecte& cO = o;
    auto lucru = [&](int t) {
        BufferThread& buf = buffere[t];
        for (int u = t; u < unitati; u += nt) {            // strided: balances the triangular brute-force loop
            switch (algoritm) {
                case Algoritm::BruteForce:     UnitateBruteForce(cO, u, buf); break;
                case Algoritm::Grid:           UnitateGrid(cO, u, buf); break;
                case Algoritm::Quadtree:       UnitateQuadtree(cO, u, buf); break;
                case Algoritm::SweepAndPrune:  UnitateSweep(cO, u, buf); break;
            }
        }
    };

    if (nt == 1) {
        lucru(0);
    } else {
        std::vector<std::thread> threaduri;
        for (int t = 1; t < nt; t++) threaduri.emplace_back(lucru, t);
        lucru(0);
        for (auto& th : threaduri) th.join();
    }

    // resolve phase (sequential)
    comparatii = 0;
    perechiGasite = 0;
    for (int t = 0; t < nt; t++) {
        comparatii += buffere[t].comparatii;
        perechiGasite += (int)buffere[t].perechi.size();
        for (const Pereche& p : buffere[t].perechi) {
            o.flash[p.first] = o.flash[p.second] = 1.0f;
            o.Rezolva(p.first, p.second);
        }
    }

    float ms = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - t0).count();
    timpMs = (timpMs == 0.0f) ? ms : timpMs * 0.9f + ms * 0.1f;
}

void SistemColiziuni::DeseneazaDebug(Algoritm algoritm) const {
    if (algoritm == Algoritm::Grid) grid.Deseneaza();
    else if (algoritm == Algoritm::Quadtree) quadtree.Deseneaza();
}