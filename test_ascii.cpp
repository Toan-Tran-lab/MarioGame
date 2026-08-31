#include <raylib.h>
#include <iostream>
int main() {
    InitWindow(100, 100, "test");
    Image img = LoadImage("assets/textures/Fire Mario/walk/fireMario.png");
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Color c = GetImageColor(img, x, y);
            if (c.a == 0) std::cout << " " ;
            else if (c.r > 200 && c.g > 200 && c.b > 200) std::cout << "W";
            else if (c.r < 50 && c.g < 50 && c.b < 50) std::cout << "B";
            else std::cout << "X";
        }
        std::cout << std::endl;
    }
    UnloadImage(img);
    CloseWindow();
    return 0;
}
