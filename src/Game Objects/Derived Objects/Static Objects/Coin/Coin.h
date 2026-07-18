#include "../../../Core Header Files/Characters.h"

class Coin : public GameObject {
public:
    // Just spins in place, no velocity/facing needed
    void Update(float /*dt*/) override {
        // Spinning in place
    }

    void Draw() override;
};