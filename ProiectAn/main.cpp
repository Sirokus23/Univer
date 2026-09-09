#include "raylib.h"

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
        pozitie.x = viteza.x * dt;
        pozitie.y = viteza.y * dt;

        if (pozitie.x - raza < 0 || pozitie.x + raza > latimeEcran) {
            viteza.x *= -1;
        }
        if (pozitie.y - raza < 0 || pozitie.y + raza > inaltimeEcran) {
            viteza.y *= -1;
        }
    }
};

int main() {
    const int width = 720;
    const int height = 720;

    InitWindow(width, height, "Collision");

    SetTargetFPS(60);

    while (!WindowShouldClose()){
        BeginDrawing();

            ClearBackground(BLACK);
            DrawText("Ebati loh", 190,200, 20, WHITE);
            EndDrawing();
    }

    CloseWindow();

    return 0;
}