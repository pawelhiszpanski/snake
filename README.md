# Classic Snake Engine (C / SDL2)

A low-level implementation of the classic Snake game written in C, utilizing the SDL2 graphics library. This project focuses on manual memory management, pointer arithmetic, and delta-time game loops.

## Tech Stack
* **Language:** C
* **Graphics Library:** SDL2 (Simple DirectMedia Layer)
* **Data Structures:** Singly Linked Lists (Dynamic body generation)

## Core Mechanics & Engineering
* **Dynamic Memory Allocation:** The snake's body is built using a linked list (`malloc`/`free`). As the snake eats, new segments are dynamically allocated and appended to the tail.
* **Delta-Time Movement:** The game loop uses `SDL_GetTicks()` to calculate delta-time, ensuring the game runs consistently regardless of the CPU's framerate.
* **Dynamic Difficulty:** The game includes a speed-up factor. The update interval decreases progressively based on world-time, making the game harder the longer you survive.
* **File I/O High Scores:** Saves and retrieves the top 3 global scores using local text file parsing (`fscanf`/`fprintf`).
* **Custom Rendering:** Uses raw bitmap blitting (`SDL_BlitSurface`) and custom mathematical functions to draw circles and rectangles pixel-by-pixel.

## Features
* Smooth 2D grid movement.
* Standard food (Blue dots) and timed bonus food (Red dots) mechanics.
* Real-time UI displaying current score, FPS, and bonus timers.
* Persistent Top-3 Leaderboard with name input.

## How to Build and Run
1. Ensure you have the **SDL2 library** installed on your system.
2. If using Visual Studio, configure the `Include` and `Library` directories to point to your local SDL2 installation.
3. Link `SDL2.lib` and `SDL2main.lib`.
4. Ensure the `assets/` folder (containing `.bmp` files) and `SDL2.dll` are located in the same directory as your compiled executable.
