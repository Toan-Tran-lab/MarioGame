#pragma once
#include <vector>
#include <memory>
#include "Game Objects/Derived Objects/Static Objects/Block.h"

class BlockGrid {
private:
    std::vector<std::vector<std::unique_ptr<Block>>> grid_;
    int width_;
    int height_;
    int tileSize_;

public:
    BlockGrid() : width_(0), height_(0), tileSize_(0) {}

    void Init(int width, int height, int tileSize) {
        width_ = width;
        height_ = height;
        tileSize_ = tileSize;
        grid_.clear();
        grid_.resize(height_);
        for (int row = 0; row < height_; ++row) {
            grid_[row].resize(width_);
        }
    }

    void SetBlock(int col, int row, std::unique_ptr<Block> block) {
        if (col >= 0 && col < width_ && row >= 0 && row < height_) {
            grid_[row][col] = std::move(block);
        }
    }

    Block* GetBlock(int col, int row) const {
        if (col >= 0 && col < width_ && row >= 0 && row < height_) {
            return grid_[row][col].get();
        }
        return nullptr;
    }
    
    // Quick bounds check
    bool IsSolidAt(int col, int row) const {
        Block* b = GetBlock(col, row);
        return b && b->IsSolid();
    }

    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    int GetTileSize() const { return tileSize_; }

    void Update(float dt) {
        for (int row = 0; row < height_; ++row) {
            for (int col = 0; col < width_; ++col) {
                if (grid_[row][col]) {
                    grid_[row][col]->Update(dt);
                }
            }
        }
    }

    void Draw() {
        for (int row = 0; row < height_; ++row) {
            for (int col = 0; col < width_; ++col) {
                if (grid_[row][col]) {
                    grid_[row][col]->Draw();
                }
            }
        }
    }
};
