#include <iostream>
#include <string>
using namespace std;

class Animal {
protected:
    string nume;
public:
    Animal(string n) : nume(n) {}
    virtual void sunet() = 0;
    virtual ~Animal() {}
};

class Caine : public Animal {
public:
    Caine(string n) : Animal(n) {}
    void sunet() { cout << nume << " face: Ham!" << endl; }
};

class Pisica : public Animal {
public:
    Pisica(string n) : Animal(n) {}
    void sunet() { cout << nume << " face: Miau!" << endl; }
};

class Vaca : public Animal {
public:
    Vaca(string n) : Animal(n) {}
    void sunet() { cout << nume << " face: Muu!" << endl; }
};

int main() {
    Animal* a[] = { new Caine("Rex"), new Pisica("Tom"), new Caine("Azor"), new Vaca("Mima") };
    for (int i = 0; i < 4; i++) {
        a[i]->sunet();
        delete a[i];
    }
}