#include <iostream>
#include <string>
using namespace std;

class Instrument {
protected:
    string marca;
public:
    Instrument(string m) : marca(m) {}
    virtual void canta() = 0;
    virtual ~Instrument() {}
};

class Chitara : public Instrument {
public:
    Chitara(string m) : Instrument(m) {}
    void canta() { cout << marca << ": dzin-dzin" << endl; }
};

class Pian : public Instrument {
public:
    Pian(string m) : Instrument(m) {}
    void canta() { cout << marca << ": plim-plim" << endl; }
};

class Toba : public Instrument {
public:
    Toba(string m) : Instrument(m) {}
    void canta() { cout << marca << ": bum-bum" << endl; }
};

int main() {
    Instrument* a[] = { new Chitara("Yamaha"), new Pian("Steinway"), new Chitara("Fender"), new Toba("Pearl") };
    for (int i = 0; i < 4; i++) {
        a[i]->canta();
        delete a[i];
    }
}