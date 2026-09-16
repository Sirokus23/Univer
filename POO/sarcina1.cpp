#include <iostream>
using namespace std;

typedef struct Product {
    string nume;
    double pret;
    int cantitate;
} Product;

void meniu();
int cautareProdus(Product* producte, int n, string nume);
void afisareProduse(Product* producte, int n);
void afisareProdus(Product* producte, int n, int index);
Product* creareArray(Product* producte, int n);
void cumparareProdus(Product* producte, int n, Product* cos, int &cosSize);
void afisareCos(Product* producte, int n, Product* cos, int &cosSize);
void eliminareDinCos(Product* producte, int n, Product* cos, int &cosSize);
void finalizarePlata(Product* cos, int &cosSize);

int main(){
    int n = 10;
    Product* producte = new Product[n];
    if (producte == NULL) {
        cout << "Eroare la alocarea memoriei" << endl;
        return 1;
    }

    producte = creareArray(producte, n);

    int MAX_COS = 10;
    Product* cos = new Product[MAX_COS];
    int cosSize = 0;

    int optiune;
    do {
        meniu();
        cout << "Alegeti optiunea: " << endl;
        cin >> optiune;

        switch (optiune) {
            case 1:
                afisareProduse(producte, n);
                break;
            case 2:
                cumparareProdus(producte, n, cos, cosSize);
                break;
            case 3:
                cout << "La revedere" << endl;
                break;
            default:
                cout << "Optiune invalida" << endl;
        }
    } while (optiune != 3);

    delete[] producte;
    delete[] cos;
    return 0;
}

void meniu(){
    cout << "1. Afisare Produse" << endl;
    cout << "2. Cumparare Produs" << endl;
    cout << "3. Iesire" << endl;
}

int cautareProdus(Product* product, int n, string nume) {
    for (int i = 0; i < n; i++) {
        if (product[i].nume == nume)
            return i;
    }
    return -1;
}

void afisareProdus(Product* product, int n, int index) {
    for (int i = 0; i < n; i++){
        if (i == index) {
            cout << "Produs " << i + 1 << ": " << product[i].nume
                 << "  Pret: " << product[i].pret
                 << "  Cantitate: " << product[i].cantitate << endl;
        }
    }
}

void afisareProduse(Product* product, int n){
    for (int i = 0; i < n; i++){
        cout << "Produs " << i + 1 << ": " << product[i].nume
             << "  Pret: " << product[i].pret
             << "  Cantitate: " << product[i].cantitate << endl;
    }
}

Product* creareArray(Product* producte, int n) {
    producte[0] = {"Robinet", 250.50, 15};
    producte[1] = {"Lavuar", 180.00, 20};
    producte[2] = {"Radiator", 770.75, 8};
    producte[3] = {"Teava", 12.30, 100};
    producte[4] = {"Boiler", 2485.90, 7};
    producte[5] = {"Filtru", 350.00, 12};
    producte[6] = {"Pompa", 890.00, 5};
    producte[7] = {"Furtun", 35.20, 40};
    producte[8] = {"Sifon", 45.60, 30};
    producte[9] = {"WC", 620.00, 10};

    return producte;
}

void cumparareProdus(Product* producte, int n, Product* cos, int &cosSize) {
    int optiuneCautare;
    int index = -1;

    cout << "Cautati produsul dupa:" << endl;
    cout << "1. Nume" << endl;
    cout << "2. Numar (din lista afisata)" << endl;
    cout << "3. Elimina un produs din cos" << endl;
    cout << "4. Afiseaza cosul" << endl;
    cout << "5. Finalizare Plata" << endl;
    cout << "Alegeti optiunea: ";
    cin >> optiuneCautare;

    if (optiuneCautare == 3) {
        eliminareDinCos(producte, n, cos, cosSize);
        return;
    }

    if (optiuneCautare == 4) {
        afisareCos(producte, n, cos, cosSize);
        return;
    }

    if (optiuneCautare == 5) {
        finalizarePlata(cos, cosSize);
        return;
    }

    if (optiuneCautare == 1) {
        string nume;
        cout << "Introduceti numele produsului: ";
        cin >> nume;

        index = cautareProdus(producte, n, nume);
        if (index == -1) {
            cout << "Produsul nu a fost gasit." << endl;
            return;
        }
    }
    else if (optiuneCautare == 2) {
        int numar;
        cout << "Introduceti numarul produsului (1-" << n << "): ";
        cin >> numar;

        if (numar < 1 || numar > n) {
            cout << "Numar invalid." << endl;
            return;
        }
        index = numar - 1;
    }
    else {
        cout << "Optiune invalida." << endl;
        return;
    }

    int cantitateCeruta;
    cout << "Introduceti cantitatea dorita: ";
    cin >> cantitateCeruta;

    if (cantitateCeruta <= 0) {
        cout << "Cantitate invalida." << endl;
        return;
    }

    if (cantitateCeruta > producte[index].cantitate) {
        cout << "Stoc insuficient. Disponibil: " << producte[index].cantitate << endl;
        return;
    }

    producte[index].cantitate -= cantitateCeruta;

    double costTranzactie = producte[index].pret * cantitateCeruta;
    cout << "Cost total pentru aceasta cumparare: " << costTranzactie << " MDL" << endl;

    string nume = producte[index].nume;
    bool gasitInCos = false;
    for (int i = 0; i < cosSize; i++) {
        if (cos[i].nume == nume) {
            cos[i].cantitate += cantitateCeruta;
            gasitInCos = true;
            break;
        }
    }

    if (!gasitInCos) {
        cos[cosSize].nume = producte[index].nume;
        cos[cosSize].pret = producte[index].pret;
        cos[cosSize].cantitate = cantitateCeruta;
        cosSize++;
    }

    afisareCos(producte, n, cos, cosSize);
}

void afisareCos(Product* producte, int n, Product* cos, int &cosSize) {
    if (cosSize == 0) {
        cout << "Cosul este gol." << endl;
        return;
    }

    double total = 0;
    cout << "--- Cosul dumneavoastra ---" << endl;
    for (int i = 0; i < cosSize; i++) {
        double subtotal = cos[i].pret * cos[i].cantitate;
        cout << cos[i].nume << "  x" << cos[i].cantitate
             << "  Pret unitar: " << cos[i].pret
             << "  Subtotal: " << subtotal << endl;
        total += subtotal;
    }
    cout << "Total: " << total << " MDL" << endl;
}

void eliminareDinCos(Product* producte, int n, Product* cos, int &cosSize) {
    if (cosSize == 0) {
        cout << "Cosul este gol." << endl;
        return;
    }

    afisareCos(producte, n, cos, cosSize);

    string nume;
    cout << "Introduceti numele produsului de eliminat din cos: ";
    cin >> nume;

    int indexCos = -1;
    for (int i = 0; i < cosSize; i++) {
        if (cos[i].nume == nume) {
            indexCos = i;
            break;
        }
    }

    if (indexCos == -1) {
        cout << "Produsul nu a fost gasit in cos." << endl;
        return;
    }

    int cantitateMax = cos[indexCos].cantitate;
    int cantitateDeEliminat;
    cout << "Cantitate in cos: " << cantitateMax << endl;
    cout << "Introduceti cantitatea de eliminat: ";
    cin >> cantitateDeEliminat;

    if (cantitateDeEliminat <= 0 || cantitateDeEliminat > cantitateMax) {
        cout << "Cantitate invalida." << endl;
        return;
    }

    int indexProdus = cautareProdus(producte, n, nume);
    if (indexProdus != -1) {
        producte[indexProdus].cantitate += cantitateDeEliminat;
    }

    if (cantitateDeEliminat == cantitateMax) {
        for (int i = indexCos; i < cosSize - 1; i++) {
            cos[i] = cos[i + 1];
        }
        cosSize--;
    } else {
        cos[indexCos].cantitate -= cantitateDeEliminat;
    }

    cout << "Eliminat din cos: " << cantitateDeEliminat << " x " << nume << endl;
}

void finalizarePlata(Product* cos, int &cosSize) {
    if (cosSize == 0) {
        cout << "Cosul este gol. Nu exista nimic de platit." << endl;
        return;
    }

    double total = 0;
    cout << " Chitanta " << endl;
    for (int i = 0; i < cosSize; i++) {
        double subtotal = cos[i].pret * cos[i].cantitate;
        cout << cos[i].nume << "  x" << cos[i].cantitate
             << "  Pret unitar: " << cos[i].pret
             << "  Subtotal: " << subtotal << endl;
        total += subtotal;
    }
    cout << "Total de plata: " << total << " MDL" << endl;

    double sumaPlatita;
    cout << "Introduceti suma platita: ";
    cin >> sumaPlatita;

    if (sumaPlatita < total) {
        cout << "Suma insuficienta. Mai sunt necesari " << (total - sumaPlatita) << " MDL." << endl;
        return;
    }

    double rest = sumaPlatita - total;
    cout << "Plata efectuata cu succes." << endl;
    cout << "Rest: " << rest << " MDL" << endl;

    cosSize = 0;
}