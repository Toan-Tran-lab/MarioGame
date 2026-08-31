#include <raylib.h>
#include <iostream>
int main() {
    InitWindow(100, 100, "test");
    const char* paths[] = { 
        "assets/textures/Fire Mario/walk/fireMario.png", 
        "assets/textures/Fire Mario/jump/fireMario.png", 
        "assets/textures/Fire Mario/sit/fireMario.png", 
        "assets/textures/Fire Mario/slide/fireMario.png", 
        "assets/textures/Fire Luigi/walk/fireLuigi.png", 
        "assets/textures/Fire Luigi/jump/fireLuigi.png", 
        "assets/textures/Fire Luigi/sit/fireLuigi.png", 
        "assets/textures/Fire Luigi/slide/fireLuigi.png", 
        "assets/textures/Fireball/Fireball.png", 
        "assets/textures/FireFlower/FireFlower.png" 
    };
    for(int i=0; i<10; ++i) {
        Image img = LoadImage(paths[i]);
        std::cout << paths[i] << " - " << img.width << "x" << img.height << std::endl;
        UnloadImage(img);
    }
    CloseWindow();
    return 0;
}
