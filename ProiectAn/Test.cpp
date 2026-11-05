// Headless correctness tests. Build + run with run_tests.bat (Windows) or run_tests.sh.
#include "Collision.h"
#include "Modes.h"
#include <cmath>
#include <cstdio>
#include <fstream>

static int bad = 0;
static void Verifica(bool ok, const char* nume) {
    std::printf("  [%s] %s\n", ok ? "OK" : "FAIL", nume);
    if (!ok) bad++;
}
static int PerechiReale(const Obiecte& o) {
    int ref = 0;
    for (int i = 0; i < o.Size(); i++) for (int j = i + 1; j < o.Size(); j++) ref += o.Ciocnire(i, j);
    return ref;
}
static const Algoritm toate[4] = {Algoritm::BruteForce, Algoritm::Grid, Algoritm::Quadtree, Algoritm::SweepAndPrune};

int main() {
    std::mt19937 gen(12345);
    const int L = Config::Latime, I = Config::Inaltime;

    std::printf("1. every algorithm finds exactly the pairs brute force finds (threads on/off, with giants)\n");
    for (int n : {1, 2, 100, 3000}) {
        for (bool gigant : {false, true}) {
            Obiecte base; GenereazaObiecte(base, n, L, I, gen);
            for (int i = 0; i + 1 < n && i < 40; i += 2) base.poz[i + 1] = base.poz[i];   // coincident centres
            if (gigant) for (int k = 0; k < 3; k++) GenereazaGigant(base, {400.0f + 300 * k, 500}, gen);
            int ref = PerechiReale(base);
            bool ok = true;
            for (Algoritm a : toate) for (bool mt : {false, true}) {
                Obiecte o = base;
                SistemColiziuni s(L, I, Config::MarimeCelula);
                s.SetMultithreading(mt);
                s.Update(o, a);
                if (s.NumarPerechi() != ref) { ok = false; std::printf("    mismatch n=%d algo=%d mt=%d: %d vs %d\n", n, (int)a, (int)mt, s.NumarPerechi(), ref); }
            }
            char nume[64]; std::snprintf(nume, sizeof nume, "n=%d%s (%d pairs)", n, gigant ? " + giants" : "", ref);
            Verifica(ok, nume);
        }
    }

    std::printf("2. sweep and prune keeps working while objects are added / removed\n");
    {
        Obiecte o; GenereazaObiecte(o, 500, L, I, gen);
        SistemColiziuni s(L, I, Config::MarimeCelula);
        bool ok = true;
        for (int step = 0; step < 300 && ok; step++) {
            if (step == 100) GenereazaObiecte(o, 700, L, I, gen);
            if (step == 200) o.EliminaUltimele(900);
            o.Update(Config::PasFix, L, I);
            int ref = PerechiReale(o);
            s.Update(o, Algoritm::SweepAndPrune);
            ok = (s.NumarPerechi() == ref);
        }
        Verifica(ok, "300 steps with growth and shrink");
    }

    std::printf("3. physics\n");
    {
        Obiecte e; GenereazaObiecte(e, 800, 400, 300, gen);
        auto ke = [&] { double k = 0; for (int i = 0; i < e.Size(); i++) k += 0.5 * e.masa[i] * (e.viteza[i].x * e.viteza[i].x + e.viteza[i].y * e.viteza[i].y); return k; };
        double k0 = ke(); SistemColiziuni se(400, 300, Config::MarimeCelula);
        for (int i = 0; i < 600; i++) { e.Update(Config::PasFix, 400, 300); se.Update(e, Algoritm::Grid); }
        Verifica(std::fabs(ke() / k0 - 1.0) < 0.02, "elastic collisions conserve kinetic energy");

        Obiecte f; f.Adauga({500, 500}, {0, 0}, 10, WHITE); f.Adauga({560, 500}, {0, 0}, 10, WHITE); f.Adauga({1500, 900}, {0, 0}, 10, WHITE);
        f.ApplicaForta({520, 500}, 180.0f, Config::AcceleratieMouse, 0.1f);
        Verifica(f.viteza[0].x < 0 && f.viteza[1].x > 0 && f.viteza[2].x == 0, "mouse repulsion pushes away, far balls untouched");
        Obiecte f2; f2.Adauga({500, 500}, {0, 0}, 10, WHITE); f2.ApplicaForta({600, 500}, 180.0f, -Config::AcceleratieMouse, 0.1f);
        Verifica(f2.viteza[0].x > 0, "negative acceleration attracts");

        f.viteza[2] = {5000, 0};
        for (int i = 0; i < 1200; i++) f.Amortizeaza(Config::PasFix);
        Verifica(f.viteza[2].x < Config::VitezaCalm + 1.0f && f.viteza[2].x >= Config::VitezaCalm - 0.01f, "fast balls settle to the calm speed");

        Obiecte g; g.Adauga({100, 100}, {0, 0}, 10, WHITE); g.gravitatie = 1000.0f;
        g.Update(0.1f, L, I);
        Verifica(std::fabs(g.viteza[0].y - 100.0f) < 0.01f, "gravity accelerates");

        Obiecte r; r.restitutie = 0.5f; r.Adauga({100, 100}, {100, 0}, 10, WHITE); r.Adauga({125, 100}, {-100, 0}, 10, WHITE);
        r.Rezolva(0, 1);
        Verifica(std::fabs((r.viteza[1].x - r.viteza[0].x) - 100.0f) < 0.1f, "restitution 0.5: relative speed 200 -> 100 after impact");

        Obiecte w; w.restitutie = 0.5f; w.Adauga({5, 200}, {-100, 0}, 10, WHITE); w.Update(0.0f, L, I);
        Verifica(std::fabs(w.viteza[0].x - 50.0f) < 0.01f, "walls respect restitution");
    }

    std::printf("4. giant ball makes the grid grow its cells, then shrink back\n");
    {
        Obiecte o; GenereazaObiecte(o, 200, L, I, gen);
        SistemColiziuni s(L, I, Config::MarimeCelula);
        s.Update(o, Algoritm::Grid);
        float normal = s.CelulaGrid();
        GenereazaGigant(o, {900, 500}, gen);
        s.Update(o, Algoritm::Grid);
        float cuGigant = s.CelulaGrid();
        o.EliminaUltimele(1);
        s.Update(o, Algoritm::Grid);
        Verifica(normal == Config::MarimeCelula && cuGigant >= 2 * Config::RazaGigantMin && s.CelulaGrid() == Config::MarimeCelula, "50 -> bigger -> 50");
    }

    std::printf("5. 'how it works' step lists agree with the real algorithms\n");
    {
        Explicator e;
        for (Algoritm a : toate) {
            e.Seteaza(a);
            const Explicator::Pas& ultim = e.UltimulPas();
            Obiecte copie = e.Scena();
            SistemColiziuni s(Explicatie::LatimeScena, Explicatie::InaltimeScena, Explicatie::CelulaDemo);
            s.SetMultithreading(false);
            s.Update(copie, a);
            bool ok = e.CiocniriGasite() == e.CiocniriAdevarate() && e.CiocniriAdevarate() >= 3
                   && ultim.comparatii == s.NumarComparatii() && ultim.faza == 2 && e.NrPasi() > 3;
            std::printf("    %-16s steps=%-3d comparisons=%-4d (real %-4d) collisions=%d/%d\n", NumeAlgoritm(a), e.NrPasi(), ultim.comparatii,
                        s.NumarComparatii(), e.CiocniriGasite(), e.CiocniriAdevarate());
            Verifica(ok, NumeAlgoritm(a));
        }
    }

    std::printf("6. benchmark runs all 20 configurations and writes the CSV\n");
    {
        Benchmark b(true, 1.0f, 3, 4);
        int ghi = 0;
        while (!b.Terminat() && ghi++ < 100000) b.Update();
        const auto& r = b.Rezultate();
        bool brute = true;
        for (const RezultatBench& x : r) if (x.algoritm == Algoritm::BruteForce && std::fabs(x.comparatii - x.n * (x.n - 1) / 2.0f) > 0.5f) brute = false;
        int linii = 0; { std::ifstream f("benchmark_results.csv"); std::string l; while (std::getline(f, l)) linii++; }
        Verifica(b.Terminat() && r.size() == 20, "20 results");
        Verifica(brute, "brute force always does n(n-1)/2 comparisons");
        Verifica(linii == 21, "CSV has header + 20 rows");
    }

    std::printf(bad ? "\n%d FAILURE(S)\n" : "\nALL TESTS PASSED\n", bad);
    return bad ? 1 : 0;
}