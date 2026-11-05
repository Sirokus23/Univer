#include "Spatial.h"
#include <algorithm>

SpatialGrid::SpatialGrid(float dimensiune, int lat, int inalt)
    : marimeCelula(dimensiune), latime(lat), inaltime(inalt) {
    coloane = (int)(latime / marimeCelula) + 1;
    rinduri = (int)(inaltime / marimeCelula) + 1;
    start.assign(coloane * rinduri + 1, 0);
}

int SpatialGrid::CelulaX(float x) const {
    return std::max(0, std::min((int)(x / marimeCelula), coloane - 1));
}

int SpatialGrid::CelulaY(float y) const {
    return std::max(0, std::min((int)(y / marimeCelula), rinduri - 1));
}

void SpatialGrid::Build(const std::vector<Vector2>& poz) {
    int n = (int)poz.size();
    int nc = NrCelule();

    std::fill(start.begin(), start.end(), 0);
    celulaDe.resize(n);
    for (int i = 0; i < n; i++) {
        int c = CelulaY(poz[i].y) * coloane + CelulaX(poz[i].x);
        celulaDe[i] = c;
        start[c + 1]++;
    }
    for (int c = 0; c < nc; c++) start[c + 1] += start[c];

    cursor.assign(start.begin(), start.begin() + nc);
    elemente.resize(n);
    for (int i = 0; i < n; i++) elemente[cursor[celulaDe[i]]++] = i;
}

void SpatialGrid::Deseneaza() const {
    for (int x = 0; x <= latime; x += (int)marimeCelula) DrawLine(x, 0, x, inaltime, Fade(WHITE, 0.07f));
    for (int y = 0; y <= inaltime; y += (int)marimeCelula) DrawLine(0, y, latime, y, Fade(WHITE, 0.07f));
}

Quadtree::Quadtree(Rectangle limite, int cap, int adancime)
    : capacitate(cap), adancimeMax(adancime) {
    noduri.reserve(512);
    noduri.resize(1);
    noduri[0].limite = limite;
}

void Quadtree::Clear() {
    nrNoduri = 1;
    noduri[0].primCopil = -1;
    noduri[0].intrari.clear();
}

void Quadtree::Insert(int id, Vector2 pozitie) {
    InserareLa(0, 0, Intrare{id, pozitie});
}

int Quadtree::CopilPentru(int nod, Vector2 p) const {
    const Rectangle& b = noduri[nod].limite;
    bool dreapta = p.x >= b.x + b.width * 0.5f;
    bool jos = p.y >= b.y + b.height * 0.5f;
    return noduri[nod].primCopil + (jos ? 2 : 0) + (dreapta ? 1 : 0);
}

void Quadtree::Subdivide(int nod) {
    const Rectangle b = noduri[nod].limite;
    int prim = nrNoduri;
    nrNoduri += 4;
    if ((int)noduri.size() < nrNoduri) noduri.resize(nrNoduri);   // reuses nodes after first growth

    float hw = b.width * 0.5f, hh = b.height * 0.5f;
    Rectangle r[4] = {
        {b.x,      b.y,      hw, hh}, {b.x + hw, b.y,      hw, hh},
        {b.x,      b.y + hh, hw, hh}, {b.x + hw, b.y + hh, hw, hh}
    };
    for (int i = 0; i < 4; i++) {
        noduri[prim + i].limite = r[i];
        noduri[prim + i].primCopil = -1;
        noduri[prim + i].intrari.clear();
    }
    noduri[nod].primCopil = prim;

    for (const Intrare& e : noduri[nod].intrari)
        noduri[CopilPentru(nod, e.p)].intrari.push_back(e);
    noduri[nod].intrari.clear();
}

void Quadtree::InserareLa(int nod, int adancime, const Intrare& e) {
    if (noduri[nod].primCopil != -1) {
        InserareLa(CopilPentru(nod, e.p), adancime + 1, e);
        return;
    }
    if ((int)noduri[nod].intrari.size() < capacitate || adancime >= adancimeMax) {
        noduri[nod].intrari.push_back(e);   // max depth guards against coincident points
        return;
    }
    Subdivide(nod);
    InserareLa(CopilPentru(nod, e.p), adancime + 1, e);
}

void Quadtree::InterogareLa(int nod, const Rectangle& zona, std::vector<int>& rezultat) const {
    if (!CheckCollisionRecs(noduri[nod].limite, zona)) return;
    if (noduri[nod].primCopil == -1) {
        for (const Intrare& e : noduri[nod].intrari) rezultat.push_back(e.id);
        return;
    }
    for (int i = 0; i < 4; i++) InterogareLa(noduri[nod].primCopil + i, zona, rezultat);
}

void Quadtree::Interogare(const Rectangle& zona, std::vector<int>& rezultat) const {
    InterogareLa(0, zona, rezultat);
}

void Quadtree::DeseneazaLa(int nod) const {
    const Nod& n = noduri[nod];
    if (n.primCopil == -1) {
        DrawRectangleLines((int)n.limite.x, (int)n.limite.y, (int)n.limite.width, (int)n.limite.height, Fade(WHITE, 0.07f));
        return;
    }
    for (int i = 0; i < 4; i++) DeseneazaLa(n.primCopil + i);
}

void Quadtree::Deseneaza() const { DeseneazaLa(0); }