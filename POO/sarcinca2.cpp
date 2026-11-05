#include <iostream>
#include <string>
using namespace std;

const int MAX_COS = 10;

class Product {
public:
    string* nume;
    double pret;
    int cantitate;
    int reducere;   
    bool bogo;      

    Product(const string& denum, double bani, int cant)
        : nume(new string(denum)), pret(bani), cantitate(cant), reducere(0), bogo(false) {}

    Product(const string& denum, double bani, int cant, int red)
        : nume(new string(denum)), pret(bani), cantitate(cant), reducere(red), bogo(false) {}

    Product(const Product& alt)
        : nume(new string(*alt.nume)), pret(alt.pret), cantitate(alt.cantitate),
          reducere(alt.reducere), bogo(alt.bogo) {}

    ~Product(){
        delete nume;
    }

    double pretUnitar() const {
        return pret * (100 - reducere) / 100.0;
    }
};

class Evenimente {
public:
    static const int nr = 5;
    string pachet1[nr] = {"Lavuar", "Teava", "Radiator", "Boiler", "WC"};
    string pachet2[nr] = {"Robinet", "Furtun", "Pompa", "Filtru", "Sifon"};
    double pretPachet[nr] = {380, 40, 1300, 2250, 600};
    string coduri[nr] = {"BUN5", "PRIMAVARA10", "STUDENT15", "VARA20", "SUPER30"};
    int procente[nr] = {5, 10, 15, 20, 30};
    int codActiv = -1;       
    double prag;             
    double costTransport;

    Evenimente(double pragTransport, double costT)
        : prag(pragTransport), costTransport(costT) {}

    Evenimente() : Evenimente(1000, 50) {}

    void afiseazaPachete(){
        cout << " Pachete promotionale " << endl;
        for (int i = 0; i < nr; i++)
            cout << i + 1 << ". " << pachet1[i] << " + " << pachet2[i]
                 << "  Pret pachet: " << pretPachet[i] << " MDL" << endl;
    }

    void aplicaCod(){
        string cod;
        cout << "Introduceti codul promotional: ";
        cin >> cod;
        for (int i = 0; i < nr; i++){
            if (coduri[i] == cod){
                codActiv = i;
                cout << "Cod valid! Reducere de " << procente[i] << "% aplicata." << endl;
                return;
            }
        }
        cout << "Cod invalid." << endl;
    }

    double calculeaza(Product** cos, int cosSize, double &transport, bool* bogoFolosit = nullptr){
        int ramas[MAX_COS];   
        for (int i = 0; i < cosSize; i++){
            ramas[i] = cos[i]->cantitate;
            if (bogoFolosit) bogoFolosit[i] = false;
        }
        double total = 0;

        for (int p = 0; p < nr; p++){
            int a = -1, b = -1;
            for (int i = 0; i < cosSize; i++){
                if (*cos[i]->nume == pachet1[p]) a = i;
                if (*cos[i]->nume == pachet2[p]) b = i;
            }
            if (a == -1 || b == -1) continue;
            if (pretPachet[p] >= cos[a]->pretUnitar() + cos[b]->pretUnitar()) continue;
            int seturi = ramas[a] < ramas[b] ? ramas[a] : ramas[b];
            if (seturi == 0) continue;
            ramas[a] -= seturi;
            ramas[b] -= seturi;
            total += seturi * pretPachet[p];
            cout << "[PACHET] " << pachet1[p] << " + " << pachet2[p] << "  x" << seturi
                 << "  Subtotal: " << seturi * pretPachet[p] << endl;
        }

        for (int i = 0; i < cosSize; i++){
            if (ramas[i] == 0) continue;
            double unitar = cos[i]->pretUnitar();
            int gratis = cos[i]->bogo ? ramas[i] / 2 : 0;
            double subtotal = unitar * (ramas[i] - gratis);
            total += subtotal;
            if (gratis > 0 && bogoFolosit) bogoFolosit[i] = true;
            cout << *cos[i]->nume << "  x" << ramas[i] << "  Pret unitar: " << unitar
                 << "  Subtotal: " << subtotal;
            if (gratis > 0) cout << "  (" << gratis << " gratis)";
            cout << endl;
        }

        if (codActiv != -1){
            double red = total * procente[codActiv] / 100.0;
            total -= red;
            cout << "Cod " << coduri[codActiv] << " (-" << procente[codActiv] << "%): -" << red << " MDL" << endl;
        }

        transport = (total >= prag) ? 0 : costTransport;
        return total;
    }
};

int cautareProdus(Product** product, int n, const string& nume) {
    for (int i = 0; i < n; i++)
        if (*product[i]->nume == nume) return i;
    return -1;
}

void afisareProduse(Product** product, int n){
    for (int i = 0; i < n; i++){
        cout << "Produs " << i + 1 << ": " << *product[i]->nume
             << "  Pret: " << product[i]->pret
             << "  Cantitate: " << product[i]->cantitate;
        if (product[i]->reducere > 0) cout << "  REDUCERE -" << product[i]->reducere << "%";
        if (product[i]->bogo) cout << "  1+1 GRATIS";
        cout << endl;
    }
}

void creareArray(Product** p) {
    p[0] = new Product("Robinet",  250.50,  15,  15);
    p[2] = new Product("Radiator", 770.75,  8,   10);
    p[4] = new Product("Boiler",   2485.90, 7,   20);
    p[6] = new Product("Pompa",    890.00,  5,   25);
    p[9] = new Product("WC",       620.00,  10,  5);

    p[1] = new Product("Lavuar", 180.00, 20);
    p[3] = new Product("Teava",  12.30,  100);
    p[5] = new Product("Filtru", 350.00, 12);
    p[7] = new Product("Furtun", 35.20,  40);
    p[8] = new Product("Sifon",  45.60,  30);
    int bogoIdx[5] = {1, 3, 5, 7, 8};
    for (int i = 0; i < 5; i++) p[bogoIdx[i]]->bogo = true;
}

void afisareCos(Product** cos, int cosSize, Evenimente& ev) {
    if (cosSize == 0) {
        cout << "Cosul este gol." << endl;
        return;
    }
    cout << " Cosul dumneavoastra " << endl;
    double transport;
    double total = ev.calculeaza(cos, cosSize, transport);
    cout << "Total: " << total << " MDL" << endl;
    if (transport == 0) cout << "Transport: GRATUIT" << endl;
    else cout << "Transport: " << transport << " MDL (gratuit peste " << ev.prag << " MDL)" << endl;
}

void eliminareDinCos(Product** producte, int n, Product** cos, int &cosSize, Evenimente& ev) {
    if (cosSize == 0) {
        cout << "Cosul este gol." << endl;
        return;
    }
    afisareCos(cos, cosSize, ev);

    string nume;
    cout << "Introduceti numele produsului de eliminat din cos: ";
    cin >> nume;

    int indexCos = cautareProdus(cos, cosSize, nume);
    if (indexCos == -1) {
        cout << "Produsul nu a fost gasit in cos." << endl;
        return;
    }

    int cantitateDeEliminat;
    cout << "Cantitate in cos: " << cos[indexCos]->cantitate << endl;
    cout << "Introduceti cantitatea de eliminat: ";
    cin >> cantitateDeEliminat;

    if (cantitateDeEliminat <= 0 || cantitateDeEliminat > cos[indexCos]->cantitate) {
        cout << "Cantitate invalida." << endl;
        return;
    }

    producte[cautareProdus(producte, n, nume)]->cantitate += cantitateDeEliminat;   // stocul revine

    if (cantitateDeEliminat == cos[indexCos]->cantitate) {
        delete cos[indexCos];
        for (int i = indexCos; i < cosSize - 1; i++)
            cos[i] = cos[i + 1];
        cosSize--;
    } else {
        cos[indexCos]->cantitate -= cantitateDeEliminat;
    }
    cout << "Eliminat din cos: " << cantitateDeEliminat << " x " << nume << endl;
}

void finalizarePlata(Product** producte, int n, Product** cos, int &cosSize, Evenimente& ev) {
    if (cosSize == 0) {
        cout << "Cosul este gol. Nu exista nimic de platit." << endl;
        return;
    }

    cout << " Chitanta " << endl;
    double transport;
    bool bogoFolosit[MAX_COS];
    double total = ev.calculeaza(cos, cosSize, transport, bogoFolosit) + transport;
    cout << "Transport: " << transport << " MDL" << endl;
    cout << "Total de plata: " << total << " MDL" << endl;

    double sumaPlatita;
    cout << "Introduceti suma platita: ";
    cin >> sumaPlatita;

    if (sumaPlatita < total) {
        cout << "Suma insuficienta. Mai sunt necesari " << (total - sumaPlatita) << " MDL." << endl;
        return;
    }

    cout << "Plata efectuata cu succes." << endl;
    cout << "Rest: " << sumaPlatita - total << " MDL" << endl;

    for (int i = 0; i < cosSize; i++)
        if (bogoFolosit[i])
            producte[cautareProdus(producte, n, *cos[i]->nume)]->bogo = false;

    for (int i = 0; i < cosSize; i++) delete cos[i];
    ev.codActiv = -1;  
    cosSize = 0;
}

void cumparareProdus(Product** producte, int n, Product** cos, int &cosSize, Evenimente& ev) {
    int optiune, index = -1;

    cout << "1. Cumparare dupa nume\n2. Cumparare dupa numar\n3. Elimina un produs din cos\n"
         << "4. Afiseaza cosul\n5. Finalizare Plata\n6. Cod promotional\n7. Pachete promotionale\n"
         << "Alegeti optiunea: ";
    cin >> optiune;

    if (optiune == 3) { eliminareDinCos(producte, n, cos, cosSize, ev); return; }
    if (optiune == 4) { afisareCos(cos, cosSize, ev); return; }
    if (optiune == 5) { finalizarePlata(producte, n, cos, cosSize, ev); return; }
    if (optiune == 6) { ev.aplicaCod(); return; }
    if (optiune == 7) { ev.afiseazaPachete(); return; }

    afisareProduse(producte, n);
    if (optiune == 1) {
        string nume;
        cout << "Introduceti numele produsului: ";
        cin >> nume;
        index = cautareProdus(producte, n, nume);
    } else if (optiune == 2) {
        int numar;
        cout << "Introduceti numarul produsului (1-" << n << "): ";
        cin >> numar;
        if (numar >= 1 && numar <= n) index = numar - 1;
    } else {
        cout << "Optiune invalida." << endl;
        return;
    }

    if (index == -1) {
        cout << "Produsul nu a fost gasit." << endl;
        return;
    }

    int cantitateCeruta;
    cout << "Introduceti cantitatea dorita: ";
    cin >> cantitateCeruta;

    if (cantitateCeruta <= 0) {
        cout << "Cantitate invalida." << endl;
        return;
    }
    if (cantitateCeruta > producte[index]->cantitate) {
        cout << "Stoc insuficient. Disponibil: " << producte[index]->cantitate << endl;
        return;
    }

    producte[index]->cantitate -= cantitateCeruta;

    int indexCos = cautareProdus(cos, cosSize, *producte[index]->nume);
    if (indexCos != -1) {
        cos[indexCos]->cantitate += cantitateCeruta;
    } else {
        cos[cosSize] = new Product(*producte[index]);   
        cos[cosSize]->cantitate = cantitateCeruta;
        cosSize++;
    }
    afisareCos(cos, cosSize, ev);
}

int main(){
    const int n = 10;
    Product** producte = new Product*[n];
    creareArray(producte);

    Product** cos = new Product*[MAX_COS];
    int cosSize = 0;

    Evenimente ev;   

    int optiune;
    do {
        cout << "1. Afisare Produse\n2. Cumparare Produs\n3. Iesire\nAlegeti optiunea: " << endl;
        if (!(cin >> optiune)) break;   

        switch (optiune) {
            case 1:
                afisareProduse(producte, n);
                break;
            case 2:
                cumparareProdus(producte, n, cos, cosSize, ev);
                break;
            case 3:
                cout << "La revedere" << endl;
                break;
            default:
                cout << "Optiune invalida" << endl;
        }
    } while (optiune != 3);

    for (int i = 0; i < n; i++) delete producte[i];
    delete[] producte;
    for (int i = 0; i < cosSize; i++) delete cos[i];
    delete[] cos;
    return 0;
}