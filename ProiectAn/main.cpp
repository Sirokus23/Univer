#include "raylib.h"
#include <vector>
#include <cmath>
#include <ctime>
using namespace std;

class Object{
private:
    Vector2 pozitie, viteza;
    float raza;
    Color culoare;
public:
    Object(Vector2 pozitieInit, Vector2 vitezaInit, float razaCerc, Color culoareCerc):
    pozitie(pozitieInit), viteza(vitezaInit), raza(razaCerc), culoare(culoareCerc){

    }

    void Update(float dt, int latimeEcran, int inaltimeEcran){
        pozitie.x += viteza.x * dt;
        pozitie.y += viteza.y * dt;

        if (pozitie.x - raza < 0 || pozitie.x + raza > latimeEcran) {
            viteza.x *= -1;
        }
        if (pozitie.y - raza < 0 || pozitie.y + raza > inaltimeEcran) {
            viteza.y *= -1;
        }
    }

    void DrawCircle(){
        DrawCircleV(pozitie, raza, culoare);
    }

    bool CheckCollision(Object &other) {
        float dx = pozitie.x - other.pozitie.x;
        float dy = pozitie.y - other.pozitie.y;
        float distSq = dx*dx + dy*dy;
        float razaSum = raza + other.raza;
        return distSq <= razaSum * razaSum;
    }

    void SetColor(Color c){
    culoare = c;
}

void ResolveCollision(Object &other){
    float dx = other.pozitie.x - pozitie.x;
    float dy = other.pozitie.y - pozitie.y;
    float dist = sqrtf(dx*dx + dy*dy);

    if (dist == 0.0f) { dist = 0.01f; dx = 0.01f; }

    float nx = dx / dist;
    float ny = dy / dist;

    float dvx = other.viteza.x - viteza.x;
    float dvy = other.viteza.y - viteza.y;
    float speedAlongNormal = dvx * nx + dvy * ny;

    if (speedAlongNormal > 0) return;

    float impulse = -2.0f * speedAlongNormal / 2.0f;

    viteza.x -= impulse * nx;
    viteza.y -= impulse * ny;
    other.viteza.x += impulse * nx;
    other.viteza.y += impulse * ny;

    float overlap = (raza + other.raza) - dist;
    if (overlap > 0){
        pozitie.x -= nx * overlap / 2.0f;
        pozitie.y -= ny * overlap / 2.0f;
        other.pozitie.x += nx * overlap / 2.0f;
        other.pozitie.y += ny * overlap / 2.0f;
    }
}

};

void CheckCollisionsBruteForce(std::vector<Object> &obiecte){
    for (size_t i = 0; i < obiecte.size(); i++){
        for (size_t j = i + 1; j < obiecte.size(); j++){
            if (obiecte[i].CheckCollision(obiecte[j])){
                obiecte[i].SetColor(YELLOW);
                obiecte[j].SetColor(YELLOW);
                obiecte[i].ResolveCollision(obiecte[j]);
            }
        }
    }
}


int main() {
    const int width = 720;
    const int height = 720;

    InitWindow(width, height, "Collision");

    SetTargetFPS(60);

    vector<Object> obiecte;
    srand(time(0));

    for (int i = 0; i < 10; i++){
        float x = rand() % (width - 60) + 30;
        float y = rand() % (height - 60) + 30;
        float vx = (rand() % 200) - 100;
        float vy = (rand() % 200) - 100;
        float raza = 15 + rand() % 15;
        Color culoare = { (unsigned char)(rand() % 256), (unsigned char)(rand() % 256), (unsigned char)(rand() % 256), 255 };

        obiecte.push_back(Object({x, y}, {vx, vy}, raza, culoare));
    }

    while (!WindowShouldClose()){
        float dt = GetFrameTime();
        
        for (auto &obj : obiecte){
            obj.Update(dt, width, height);
        }

        CheckCollisionsBruteForce(obiecte);

        BeginDrawing();

            ClearBackground(BLACK);
        for (auto &obj : obiecte){
            obj.DrawCircle();
        }
            EndDrawing();
    }

    CloseWindow();

    return 0;
}