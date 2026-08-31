#include <raylib.h>
#include <iostream>
int main() {
    InitWindow(100, 100, "test");
    const char* paths[] = { 
        "assets/textures/Fire Mario/walk/fireMario.png", 
        "assets/textures/Fireball/Fireball.png" 
    };
    for(int i=0; i<2; ++i) {
        Image img = LoadImage(paths[i]);
        Color c = GetImageColor(img, 0, 0);
        std::cout << paths[i] << " - Format: " << img.format << " | Pixel(0,0): (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a << ")" << std::endl;
        UnloadImage(img);
    }
    CloseWindow();
    return 0;
}
