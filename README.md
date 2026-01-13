# SpeedRacer - a Racing Game for a future Q-Learning Project

A 2D top-down racing game built with C++ and RayLib for reinforcement learning experiments.

## Features
- Realistic car physics (acceleration, friction, turning)
- Surface-based friction (track, grass, walls)
- Checkpoint system with lap timing
-  Collision detection

## Controls
- **↑**: Accelerate
- **↓**: Brake/Reverse
- **←/→**: Steer
- **R**: Restart

## Building
Requires [RayLib](https://www.raylib.com/) installed.

**Windows (MSYS2/MinGW):**
```bash
g++ src/main.cpp -o racing-game -lraylib -lopengl32 -lgdi32 -lwinmm
```

**Linux:**
```bash
g++ src/main.cpp -o racing-game -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

## Project Structure
```
├── src/          # Source code
│   └── main.cpp
├── assets/       # Game assets
│   ├── testTrack.png
│   └── racecarTransparent.png
└── README.md
```

## Future Plans
-  Q-Learning AI agent
-  Headless training mode
- Multiple tracks 
