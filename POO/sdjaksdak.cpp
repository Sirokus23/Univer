class Iaibu
{
public:
    void hamma(){
        cout << "hamma" << endl;
    };
};

class pizdet:
{
public:
    void hamma(){cout << "pizdet hamma" << endl;};
};

class gandon: public Iaibu, public pizdet
{
    pizdet::hamma();
}