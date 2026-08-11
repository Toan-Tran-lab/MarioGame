#pragma once

#include <memory>
#include <stack>
#include <vector>
#include "physics/PhysicsBody.h"
#include "physics/InputManager.h"

class IGameState {
public:
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Draw() = 0;
    virtual void Cleanup() = 0;
    virtual ~IGameState() = default;
};

class GameStateManager {
private:
    // ở đây sử dụng std::unique_ptr để quản lý bộ nhớ của các trạng thái trò chơi, đảm bảo rằng khi một trạng thái bị loại bỏ, bộ nhớ của nó sẽ được giải phóng tự động.
    // aka smart pointer
    std::stack<std::unique_ptr<IGameState>> states; 

public:
    void PushState(std::unique_ptr<IGameState> state);
    void PopState();
    void Update(float deltaTime);
    void Draw();
};

class GameplayState : public IGameState {
private:
    physics::PhysicsBody player_;
    physics::PhysicsBody enemy_;
    std::vector<Rectangle> blocks_;
    physics::InputState inputState_;

public:
    void Initialize() override;
    void Update(float deltaTime) override;
    void Draw() override;
    void Cleanup() override;
};
