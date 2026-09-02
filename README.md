# MarioGame - Group 6 (CS202 Final Project)

## Introduction
A custom C++ and Raylib-based Super Mario clone built as a final project. It features multiple playable characters (Mario & Luigi), power-ups (Mushroom, Fire Flower, Starman), various enemies (Goombas, Koopas, Piranha Plants), a dynamic boss battle with a custom Dragon Boss, and a fully functional physics and collision engine.

## Features
- **Classic Platforming Physics:** Fine-tuned gravity, jumping, sliding, and collision detection.
- **2-Player Co-op Mode:** Play as Mario and Luigi simultaneously.
- **Power-ups & Combat:** Shoot fireballs, become invincible with Starman, and stomp on enemies.
- **Dynamic Camera System:** Tracks players seamlessly and adjusts automatically during cinematic boss fights.
- **Interactive UI:** Fully functional main menu, settings, save/load slots, sandbox mode, and level selection.

## Project Layout
The project follows a localized, component-based folder structure rather than separating headers and source files globally. This makes managing the codebase much easier and faster.

```text
MarioGame/
├── assets/         # Contains all game resources (audio, fonts, maps, textures)
├── build/          # Compiled binaries and build artifacts (CMake)
└── src/            # Source code directory
    ├── Animations/       # Sprite animation data and logic
    ├── AudioManager/     # Audio loading and playback
    ├── Camera/           # Viewport and camera tracking logic
    ├── core/             # Core game engine logic (GameStateManager)
    ├── GameplayState/    # In-game state, including Boss Battle controllers
    ├── Game_Objects/     # All entities (Player, Enemies, Items, Blocks, Projectiles)
    ├── Level/            # Level parsing and management
    ├── MainMenu/         # UI States (Menus, Settings, Save/Load, Level Select)
    ├── physics/          # Custom AABB collision engine and input handling
    ├── SaveManager/      # Auto-save and manual saving logic
    ├── TextureManager/   # Centralized texture caching and rendering
    └── World/            # TileMap and BlockGrid generation
```

## How to Build and Run

### Prerequisites
- [CMake](https://cmake.org/) (version 3.10+)
- A C++ Compiler (MinGW GCC, MSVC, or Clang)
- [Raylib](https://www.raylib.com/) dependencies (automatically handled by CMake FetchContent)

### Build Instructions
1. **Configure the build files:**
   Open a terminal in the project root. Depending on your compiler/OS, use one of the following:
   
   **For Windows (using MinGW):**
   Tell CMake to explicitly use the MinGW g++ compiler on your system:
   ```bash
   cmake -G "MinGW Makefiles" -S . -B build
   ```

   **For Windows (MSVC), macOS, or Linux:**
   Use the default generator:
   ```bash
   cmake -S . -B build
   ```

2. **Build the game:**
   Compile the target executable using CMake:
   ```bash
   cmake --build build --target MarioGame
   ```

3. **Run it:**
   Run the compiled executable directly from the `build` directory:
   ```bash
   # On Windows
   ./build/MarioGame.exe
   
   # On macOS / Linux
   ./build/MarioGame
   ```

## Controls
- **Player 1 (Mario):**
  - `A` / `D`: Move Left / Right
  - `W`: Jump
  - `F`: Shoot Fireball (when Fire Mario)
- **Player 2 (Luigi):**
  - `Left Arrow` / `Right Arrow`: Move Left / Right
  - `Up Arrow`: Jump
  - `J`: Shoot Fireball (when Fire Luigi)
- **General:**
  - `ESC`: Pause Game
  - `TAB`: Toggle Player 2 Mode

## Acknowledgements and Fair Use

**Fair Use Declaration:**  
This project is an educational, non-commercial fan-game created purely for academic and learning purposes as a university final project. All rights, trademarks, and copyrights to the "Mario" franchise, its characters, music, and associated assets belong entirely to **Nintendo**. This project is **strictly not intended for distribution, monetization, or marketing in any form**.

**AI Assistance:**  
Development of this project was assisted by **Antigravity**, an advanced agentic coding AI, used for writing, structuring, scaling, and debugging the C++ architecture.