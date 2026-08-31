#include <raylib.h>
#include <iostream>
int main() {
    InitWindow(100, 100, "test");
    Image img = LoadImage("assets/textures/Fire Mario/walk/fireMario.png");
    Color c = GetImageColor(img, 0, 7);
    std::cout << "Pixel(0,7): (" << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a << ")" << std::endl;
    UnloadImage(img);
    CloseWindow();
    return 0;
}
