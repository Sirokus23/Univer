#include "Object.h"
#include <algorithm>
#include <cmath>

namespace {
    const Color palet[] = {
        {255, 107, 129, 255}, {255, 168,  76, 255}, {255, 214, 102, 255}, {106, 230, 160, 255},
        { 84, 195, 255, 255}, {138, 150, 255, 255}, {201, 139, 255, 255}, {255, 140, 200, 255},
    };
    const int nrPalet = (int)(sizeof(palet) / sizeof(palet[0]));
}

void Obiecte::Adauga(Vector2 p, Vector2 v, float r, Color c) {
    poz.push_back(p);
    viteza.push_back(v);
    raza.push_back(r);
    masa.push_back(r * r);
    culoare.push_back(c);
    flash.push_back(0.0f);
}

void Obiecte::EliminaUltimele(int k) {
    int n = Size() - std::min(k, Size());
    poz.resize(n); viteza.resize(n); raza.resize(n);
    masa.resize(n); culoare.resize(n); flash.resize(n);
}

void Obiecte::Update(float dt, int latimeEcran, int inaltimeEcran) {
    for (int i = 0; i < Size(); i++) {
        Vector2& p = poz[i];
        Vector2& v = viteza[i];
        float r = raza[i];

        v.y += gravitatie * dt;
        p.x += v.x * dt;
        p.y += v.y * dt;

        // Clamp + force direction, so an object never gets stuck flipping velocity outside the screen.
        const float e = restitutie;
        if (p.x - r < 0)                  { p.x = r;                 v.x =  fabsf(v.x) * e; }
        else if (p.x + r > latimeEcran)   { p.x = latimeEcran - r;   v.x = -fabsf(v.x) * e; }
        if (p.y - r < 0)                  { p.y = r;                 v.y =  fabsf(v.y) * e; }
        else if (p.y + r > inaltimeEcran) { p.y = inaltimeEcran - r; v.y = -fabsf(v.y) * e; }

        flash[i] = std::max(0.0f, flash[i] - dt * 4.0f);
    }
}

bool Obiecte::Ciocnire(int a, int b) const {
    float dx = poz[a].x - poz[b].x;
    float dy = poz[a].y - poz[b].y;
    float s = raza[a] + raza[b];
    return dx * dx + dy * dy <= s * s;
}

void Obiecte::Rezolva(int a, int b) {
    float dx = poz[b].x - poz[a].x;
    float dy = poz[b].y - poz[a].y;
    float dist = sqrtf(dx * dx + dy * dy);

    float nx = 1.0f, ny = 0.0f;                 // fallback if centres coincide
    if (dist > 1e-6f) { nx = dx / dist; ny = dy / dist; }

    const float invM1 = 1.0f / masa[a];
    const float invM2 = 1.0f / masa[b];

    // Elastic impulse (e = 1) along the normal, weighted by inverse masses.
    float vn = (viteza[b].x - viteza[a].x) * nx + (viteza[b].y - viteza[a].y) * ny;
    if (vn < 0) {
        float j = -(1.0f + restitutie) * vn / (invM1 + invM2);
        viteza[a].x -= j * invM1 * nx;
        viteza[a].y -= j * invM1 * ny;
        viteza[b].x += j * invM2 * nx;
        viteza[b].y += j * invM2 * ny;
    }

    // Positional correction: the lighter object moves more.
    float overlap = (raza[a] + raza[b]) - dist;
    if (overlap > 0) {
        float total = masa[a] + masa[b];
        float m1 = overlap * (masa[b] / total);
        float m2 = overlap * (masa[a] / total);
        poz[a].x -= nx * m1;  poz[a].y -= ny * m1;
        poz[b].x += nx * m2;  poz[b].y += ny * m2;
    }
}

void Obiecte::ApplicaForta(Vector2 c, float R, float acc, float dt) {
    const float R2 = R * R;
    for (int i = 0; i < Size(); i++) {
        float dx = poz[i].x - c.x, dy = poz[i].y - c.y;
        float d2 = dx * dx + dy * dy;
        if (d2 >= R2) continue;

        float d = sqrtf(d2);
        float nx = 0.0f, ny = -1.0f;            // cursor exactly on a ball: push up
        if (d > 1e-3f) { nx = dx / d; ny = dy / d; }

        float a = acc * (1.0f - d / R);         // strongest at the cursor
        viteza[i].x += nx * a * dt;
        viteza[i].y += ny * a * dt;
    }
}

void Obiecte::Amortizeaza(float dt) {
    const float calm2 = Config::VitezaCalm * Config::VitezaCalm;
    const float f = std::min(1.0f, Config::Amortizare * dt);
    for (int i = 0; i < Size(); i++) {
        Vector2& v = viteza[i];
        float sp2 = v.x * v.x + v.y * v.y;
        if (sp2 <= calm2) continue;

        float sp = sqrtf(sp2);
        float tinta = std::min(sp - (sp - Config::VitezaCalm) * f, Config::VitezaCap);
        float s = tinta / sp;
        v.x *= s;
        v.y *= s;
    }
}

namespace {
    // cool blue -> teal -> yellow -> hot red
    Color ColoareViteza(float viteza) {
        static const Color st[4] = {{80, 140, 255, 255}, {110, 230, 200, 255}, {255, 220, 100, 255}, {255, 90, 110, 255}};
        float t = std::min(1.0f, viteza / Config::VitezaRosu) * 3.0f;
        int k = (int)t;
        if (k >= 3) return st[3];
        float f = t - k;
        return Color{(unsigned char)(st[k].r + (st[k + 1].r - st[k].r) * f),
                     (unsigned char)(st[k].g + (st[k + 1].g - st[k].g) * f),
                     (unsigned char)(st[k].b + (st[k + 1].b - st[k].b) * f), 255};
    }
}

void Obiecte::Deseneaza(const Texture2D& cerc, float alpha, bool dupaViteza) const {
    Rectangle sursa = {0, 0, (float)cerc.width, (float)cerc.height};
    unsigned char a = (unsigned char)(255.0f * alpha);
    for (int i = 0; i < Size(); i++) {
        float r = raza[i];
        float f = flash[i];
        Color c = dupaViteza ? ColoareViteza(sqrtf(viteza[i].x * viteza[i].x + viteza[i].y * viteza[i].y)) : culoare[i];
        Color tinta = { (unsigned char)(c.r + (255 - c.r) * f),
                        (unsigned char)(c.g + (255 - c.g) * f),
                        (unsigned char)(c.b + (255 - c.b) * f), a };
        Rectangle dest = {poz[i].x - r, poz[i].y - r, 2 * r, 2 * r};
        DrawTexturePro(cerc, sursa, dest, Vector2{0, 0}, 0.0f, tinta);
    }
}

void GenereazaObiecte(Obiecte& obiecte, int nr, int latimeEcran, int inaltimeEcran, std::mt19937& gen, float scara) {
    const float margine = Config::RazaMax * scara;
    std::uniform_real_distribution<float> distX(margine, latimeEcran - margine);
    std::uniform_real_distribution<float> distY(margine, inaltimeEcran - margine);
    std::uniform_real_distribution<float> distV(-Config::VitezaMax, Config::VitezaMax);
    std::uniform_real_distribution<float> distRaza(Config::RazaMin * scara, Config::RazaMax * scara);
    std::uniform_int_distribution<int> distPalet(0, nrPalet - 1);

    for (int i = 0; i < nr; i++)
        obiecte.Adauga(Vector2{distX(gen), distY(gen)}, Vector2{distV(gen), distV(gen)},
                       distRaza(gen), palet[distPalet(gen)]);
}

void GenereazaObiectLa(Obiecte& obiecte, Vector2 pozitie, std::mt19937& gen) {
    std::uniform_real_distribution<float> distV(-Config::VitezaMax, Config::VitezaMax);
    std::uniform_real_distribution<float> distRaza(Config::RazaMin, Config::RazaMax);
    std::uniform_int_distribution<int> distPalet(0, nrPalet - 1);
    obiecte.Adauga(pozitie, Vector2{distV(gen), distV(gen)}, distRaza(gen), palet[distPalet(gen)]);
}

void GenereazaGigant(Obiecte& obiecte, Vector2 pozitie, std::mt19937& gen) {
    std::uniform_real_distribution<float> distV(-Config::VitezaMax, Config::VitezaMax);
    std::uniform_real_distribution<float> distRaza(Config::RazaGigantMin, Config::RazaGigantMax);
    std::uniform_int_distribution<int> distPalet(0, nrPalet - 1);
    obiecte.Adauga(pozitie, Vector2{distV(gen) * 0.5f, distV(gen) * 0.5f}, distRaza(gen), palet[distPalet(gen)]);
}

const char* NumeLayout(Layout l) {
    switch (l) {
        case Layout::Aleator:   return "Random";
        case Layout::Uniform:   return "Even";
        case Layout::Ciorchini: return "Clumps";
        case Layout::Teanc:     return "Pile";
        case Layout::Banda:     return "Band";
        case Layout::Gigant:    return "Giant";
    }
    return "?";
}

const char* DescriereLayout(Layout l) {
    switch (l) {
        case Layout::Aleator:   return "typical case";
        case Layout::Uniform:   return "best case for the grid";
        case Layout::Ciorchini: return "grid struggles, quadtree adapts";
        case Layout::Teanc:     return "worst case for everything";
        case Layout::Banda:     return "worst case for sweep & prune";
        case Layout::Gigant:    return "grid cells have to grow";
    }
    return "";
}

void GenereazaLayout(Obiecte& o, Layout l, int W, int H, std::mt19937& gen) {
    o.EliminaUltimele(o.Size());                 // keeps gravity / restitution settings
    const float m = Config::RazaMax;
    std::uniform_real_distribution<float> dr(Config::RazaMin, Config::RazaMax), u01(0.0f, 1.0f);
    std::uniform_int_distribution<int> dp(0, nrPalet - 1);
    auto bila = [&](float x, float y, float r) {
        x = std::clamp(x, r, W - r);
        y = std::clamp(y, r, H - r);
        o.Adauga(Vector2{x, y}, Vector2{0, 0}, r, palet[dp(gen)]);
    };

    // jittered lattice, always wide enough that no two balls touch
    auto grila = [&](int n, bool lasaLocGigantului) {
        int cols = std::max(1, (int)lroundf(sqrtf(n * (W - 2 * m) / (H - 2 * m))));
        int rows = (n + cols - 1) / cols;
        float cw = (W - 2 * m) / cols, ch = (H - 2 * m) / rows;
        float jit = std::max(0.0f, std::min(cw, ch) / 2.0f - m - 1.0f);
        std::uniform_real_distribution<float> dj(-jit, jit);
        int pus = 0;
        for (int r = 0; r < rows && pus < n; r++)
            for (int c = 0; c < cols && pus < n; c++) {
                float x = m + (c + 0.5f) * cw, y = m + (r + 0.5f) * ch;
                if (lasaLocGigantului && hypotf(x - W / 2.0f, y - H / 2.0f) < Config::RazaGigantMax + m + 4.0f) continue;
                bila(x + (jit > 0 ? dj(gen) : 0.0f), y + (jit > 0 ? dj(gen) : 0.0f), dr(gen));
                pus++;
            }
    };

    switch (l) {
        case Layout::Aleator:
            GenereazaObiecte(o, 1000, W, H, gen);           // the only preset that keeps moving
            break;
        case Layout::Uniform:
            grila(1000, false);
            break;
        case Layout::Ciorchini: {
            std::uniform_real_distribution<float> cx(0.15f * W, 0.85f * W), cy(0.15f * H, 0.85f * H);
            Vector2 centre[5];
            for (Vector2& c : centre) c = {cx(gen), cy(gen)};
            std::normal_distribution<float> g(0.0f, 60.0f);
            for (int i = 0; i < 1000; i++) {
                const Vector2& c = centre[i % 5];
                bila(c.x + g(gen), c.y + g(gen), dr(gen));
            }
            break;
        }
        case Layout::Teanc:                                  // 500 balls piled into a 40 px disc
            for (int i = 0; i < 500; i++) {
                float ang = u01(gen) * 6.2831853f, d = sqrtf(u01(gen)) * 40.0f;
                bila(W / 2.0f + cosf(ang) * d, H / 2.0f + sinf(ang) * d, dr(gen));
            }
            break;
        case Layout::Banda:                                  // all balls share (almost) the same x range
            for (int i = 0; i < 120; i++)
                bila(W / 2.0f + (u01(gen) - 0.5f) * 24.0f, m + u01(gen) * (H - 2 * m), dr(gen));
            break;
        case Layout::Gigant:
            grila(400, true);
            o.Adauga(Vector2{W / 2.0f, H / 2.0f}, Vector2{0, 0}, Config::RazaGigantMax, palet[dp(gen)]);
            break;
    }
}

Texture2D CreeazaTexturaCerc() {
    // White, fully transparent start so mipmap filtering doesn't create dark fringes.
    Image img = GenImageColor(64, 64, Color{255, 255, 255, 0});

    // Concentric discs, getting brighter and drifting to the top-left: a cheap "sphere" look.
    // The texture is multiplied by the ball colour when drawn.
    for (int k = 0; k < 28; k++) {
        float t = k / 27.0f;
        unsigned char v = (unsigned char)(150 + 105 * t);
        int off = (int)(k * 0.18f);
        ImageDrawCircle(&img, 32 - off, 32 - off, 31 - k, Color{v, v, v, 255});
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}