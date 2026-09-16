#include <iostream>
using namespace std;

class Student {
private:
    string nume;
    double medie;
    int nrCredite;

public:
    Student(string n, double m, int c) {
        nume = n;
        medie = m;
        nrCredite = c;
    }

    string getNume() { 
        return nume; 
    }

    double getMedie() { 
        return medie; 
    }

    int getCredite() {
        return nrCredite;
    }

    void afiseaza() {
        cout << nume << " - media: " << medie << "\n";
    }

    bool esteRestant(){
    return medie < 5.0;
    };
};

void meniu(){
    cout << "1.Afisare toti studenti" << endl;
    cout << "2.Afisare informatie un Student" << endl;
    cout << "3.Afisare status a unui student" << endl;
    cout << "4. Iesire" << endl;
}

void afisareStudenti(Student* studenti){
    for (int i = 0;i < 7; i++){
        cout << i + 1 << endl;
        studenti[i].afiseaza();
    }
}

int main() {
    Student studenti[7]
    {
        {"Ion", 6.40, 15},
        {"Vasile", 7.80, 15},
        {"Maria", 8.56, 15},
        {"Petru", 4.50, 12},
        {"Oleg", 5.0, 15},
        {"Vlad", 3.70, 12},
        {"Ivan", 7.59, 15},
    };

    int alegere;
    int index;

    do{
        meniu();

        cout << "Alegeti o varianta: " << endl;
        cin >> alegere;

        switch (alegere)
        {
        case 1:
            afisareStudenti(studenti);
            break;
        case 2:
            cout << "Alegeti indexul unui student pentru afisare a informatiei: " << endl;
            cin >> index;
            index--;
            studenti[index].afiseaza();
            break;
        case 3:
            cout << "Alegeti indexul unui student pentru afisare a situatiei studentului: " << endl;
            cin >> index;
            index--;
            if (studenti[index].esteRestant()){
                cout << "Studentul dat este restantier" << endl;
            }else {
                cout << "Studentul date nu este restantier" << endl;
            }
        default:
            break;
        }
    } while (alegere != 4);
}