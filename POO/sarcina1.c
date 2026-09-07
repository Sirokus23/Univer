#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Product {
    char nume[50];
    double pret;
    int cantitate;
};

void meniu();
int cautareProdus(struct Product* producte, int n, char numeprod[50]);
void afisareProduse(struct Product* producte, int n);
void afisareProdus(struct Product* producte, int n, int index);
void cumparareProdus(struct Product* producte, int n);
struct Product* creareArray(struct Product* producte, int n);

int main() {
    int n;
    printf("Cate produse doriti sa introduceti? ");
    scanf("%d", &n);
 
    struct Product* producte = malloc(n * sizeof(struct Product));
    if (producte == NULL) {
        printf("Eroare de alocare memorie!\n");
        return 1;
    }
 
    producte = creareArray(producte, n);
 
    int optiune;
    do {
        meniu();
        printf("Alegeti optiunea: ");
        scanf("%d", &optiune);
 
        switch (optiune) {
            case 1:
                afisareProduse(producte, n);
                break;
            case 2:
                cumparareProdus(producte, n);
                break;
            case 3:
                printf("La revedere!\n");
                break;
            default:
                printf("Optiune invalida!\n");
        }
    } while (optiune != 3);
 
    free(producte);
    return 0;
}

void meniu() {
    printf("1. Afisare produse\n");
    printf("2. Cumparare produs\n");
    printf("3. Iesire\n");
}

int cautareProdus(struct Product* product, int n, char numeprod[50]) {
    for (int i = 0; i < n; i++) {
        if (strcmp(product[i].nume, numeprod) == 0) {
            return i;
        }           
    }
    return -1;
}

void afisareProdus(struct Product* producte, int n, int index) {
    for (int i = 0;i < n;i++){
        if (i == index) {
            printf("Produs %d:\n", i + 1);
            printf("Denumire: %s\n", producte[index].nume);
            printf("Pret: %.2lf\n", producte[index].pret);
            printf("Cantitate: %d\n", producte[index].cantitate);
        }
    }
}

void cumparareProdus(struct Product* producte, int n) {
    int alegere;
    char denumire[50];
    int index;
    int cantitateDorita;
 
    afisareProduse(producte, n);
 
    printf("Alegeti forma de cautare\n");
    printf("1. Dupa nume\n");
    printf("2. Dupa numar\n");
    printf("Alegeti optiunea: ");
    scanf("%d", &alegere);
 
    if (alegere == 1) {
        printf("Introduceti denumirea produsului: ");
        scanf("%s", denumire);
        index = cautareProdus(producte, n, denumire);
 
        if (index == -1) {
            printf("Produsul nu a fost gasit!\n");
            return;
        }
    } else if (alegere == 2) {
        printf("Introduceti numarul produsului: ");
        scanf("%d", &index);
        index--; // utilizatorul introduce numarul incepand de la 1
 
        if (index < 0 || index >= n) {
            printf("Numar invalid!\n");
            return;
        }
    } else {
        printf("Optiune invalida!\n");
        return;
    }
 
    printf("Introduceti cantitatea dorita: ");
    scanf("%d", &cantitateDorita);
 
    if (cantitateDorita > producte[index].cantitate) {
        printf("Nu exista suficiente produse in stoc\n");
    } else {
        producte[index].cantitate -= cantitateDorita;
        printf("Cumparare realizata cu succes!\n");
    }
}

void afisareProduse(struct Product* producte, int n) {
    for (int i = 0; i < n; i++) {
        printf("Produs %d:\n", i + 1);
        printf("Denumire: %s\n", producte[i].nume);
        printf("Pret: %.2lf\n", producte[i].pret);
        printf("Cantitate: %d\n", producte[i].cantitate);
    }
}

struct Product* creareArray(struct Product* producte, int n) {
    for (int i = 0; i < n; i++) {
        printf("Introduceti denumirea produsului %d: ", i + 1);
        scanf("%s", producte[i].nume);
        printf("Introduceti pretul produsului %d: ", i + 1);
        scanf("%lf", &producte[i].pret);
        printf("Introduceti cantitatea produsului %d: ", i + 1);
        scanf("%d", &producte[i].cantitate);
    }
    return producte;
}



