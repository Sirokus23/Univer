#include <iostream>
#include <string.h>
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
void cumparareProdus(Product* producte, int n);
struct Product* creareArray(Product* producte, int n);

int main(){
    int n = 10;
    Product* producte = new Product[n];
    if (producte == NULL) {
        cout << "Eroare la alocarea memoriei" << endl;
        return 1;
    }

    producte = creareArray(producte, n);

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
                cumparareProdus(producte, n);
                break;
            case 3:
                cout << "La revedere" << endl;
                break;
            default:
                cout << "Optiune invalida" << endl;
        }
    }while (optiune != 3);

    delete[] producte;
    return 0;
}

void meniu(){
    cout << "1.Afisare Produs" << endl;
    cout << "2.Cumparare Produs" << endl;
    cout << "3. Iesire" << endl;
}

int cautareProduse(Product* product, int n, string nume) {
    for (int i = 0; i < n; i++) {
        if (product[i].nume == nume)
            return i;
    }
    return -1;
}

void afisareProdus(Product* product, int n, int index) {
    for (int i = 0;i < n; i++){
        if (i == index) {
            cout << "Produs " << i + 1 << ": " << product[i].nume << "  Pret: " << product[i].pret << "  Cantitate: " << product[i].cantitate << endl;
        }
    }
}

void cumparareProdus(Product* product, int n) {
    int alegere;
    string denumire;
    int index;
    int cantitateDorita;

    afisareProduse(product, n);

    cout << "Alegeti forma de cautare" << endl;
    cout << "1. Dupa nume" << endl;
    cout << "2. Dupa numar" << endl;
    cout << "Alegeti optiunea: ";
    cin >> alegere;

    if (alegere == 1) {
        cout << "Introduceri denumirea produsului: " << endl;
        cin >> denumire;
        index = cautareProduse(product, n, denumire);

        if (index == -1) {
            cout << "Produsul nu a fost gasit" << endl;
            return;
        }

        afisareProdus(product, n, index);

    } else if (alegere == 2) {
        cout << "Introduceti numarul produsului: " << endl;
        cin >> index;
        index--;

        if (index < 0 || index >= n){
            cout << "Numar invalid" << endl;
            return;
        }

        afisareProdus(product, n, index);
        
    }else {
        cout << "Optiune invalida" << endl;
        return;
    }

    cout << "Introduceti cantitatea dorita" << endl;
    cin >> cantitateDorita;

    if (cantitateDorita > product[index].cantitate) {
        cout << "Nu sunt suficiente produse in stoc" << endl;
        return;
    } else {
        double total = cantitateDorita * product[index].pret;
        cout << "Produs achizitionat. Total de plata: " << total << " lei" << endl;
        product[index].cantitate -= cantitateDorita;
    }
}

void afisareProduse(Product* product, int n){
    for (int i = 0; i < n; i++){
        cout << "Produs " << i + 1 << ": " << product[i].nume << "  Pret: " << product[i].pret << "  Cantitate: " << product[i].cantitate << endl;
    }
}

Product* creareArray(Product* producte, int n) {
    producte[0] = {"Robinet", 250.50, 15};
    producte[1] = {"Baterie", 180.00, 20};
    producte[2] = {"Dus", 420.75, 8};
    producte[3] = {"Teava", 12.30, 100};
    producte[4] = {"Cupru", 25.90, 60};
    producte[5] = {"Filtru", 350.00, 12};
    producte[6] = {"Pompa", 890.00, 5};
    producte[7] = {"Furtun", 35.20, 40};
    producte[8] = {"Sifon", 45.60, 30};
    producte[9] = {"WC", 620.00, 10};

    return producte;
}