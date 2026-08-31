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
        int transparent = 0, opaque = 0, semi = 0;
        for(int y=0; y<img.height; ++y) {
            for(int x=0; x<img.width; ++x) {
                Color c = GetImageColor(img, x, y);
                if(c.a == 0) transparent++;
                else if(c.a == 255) opaque++;
                else semi++;
            }
        }
        std::cout << paths[i] << " | " << img.width << "x" << img.height
                  << " | format=" << img.format
                  << " | transparent=" << transparent
                  << " opaque=" << opaque
                  << " semi=" << semi << std::endl;
        UnloadImage(img);
    }
    CloseWindow();
    return 0;
}
