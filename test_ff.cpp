#include <raylib.h>
#include <iostream>
int main() {
    InitWindow(100, 100, "test");
    Image img = LoadImage("assets/textures/FireFlower/FireFlower.png");
    std::cout << img.width << "x" << img.height << std::endl;
    UnloadImage(img);
    CloseWindow();
    return 0;
}
