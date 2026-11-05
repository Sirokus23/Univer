# Collision Lab

Interactive comparison of four **broad-phase collision detection** algorithms, written in C++17 with raylib:

| Key | Algorithm | Idea |
|----|-----------|------|
| 1 | **Brute Force** | test every pair, O(n²) |
| 2 | **Spatial Grid** | bucket balls into cells, test only neighbouring cells |
| 3 | **Quadtree** | recursively split crowded areas into 4 |
| 4 | **Sweep and Prune** | sort by left edge on the x axis, sweep left to right |

## Build and run

```
build.bat
```

or by hand, from the `src` folder:

```
g++ main.cpp Object.cpp Spatial.cpp Collision.cpp UI.cpp Modes.cpp -o main.exe -IC:/msys64/mingw64/include -LC:/msys64/mingw64/lib -lraylib -lopengl32 -lgdi32 -lwinmm -std=c++17 -O2 -pthread
```

`-O2` matters a lot: without it the numbers you measure are misleading.

## Start screen

* **Algorithm / Objects / Multithreading**: choose the starting setup (arrows or mouse; left click = next, right click = previous)
* **START**: the sandbox
* **HOW IT WORKS**: animated, step-by-step explanation of each algorithm (see below)
* **BENCHMARK**: runs every algorithm at 100 / 500 / 1000 / 2000 / 5000 objects and saves `benchmark_results.csv`

## Window

The game renders at a virtual 1920x1080 and is scaled to the window (black bars if the shape differs). It opens sized to fit your monitor, can be resized freely, and F11 toggles borderless fullscreen. Mouse input is mapped back to the virtual size, so the force field stays under the cursor at any scale.

## Sandbox controls

| Input | Action |
|-------|--------|
| Left mouse (hold) | repel balls around the cursor |
| Shift + left mouse, or middle mouse | attract balls |
| Mouse wheel | field size |
| Right mouse | spawn a ball (hold for a stream) |
| M | spawn a giant ball at the cursor (the grid grows its cells automatically) |
| UP / DOWN | +/- 100 balls |
| PgUp / PgDn | +/- 1000 balls |
| C | clear all |
| 1 / 2 / 3 / 4 | switch algorithm |
| T | multithreading on / off |
| H | density **heatmap** (balls per grid cell) |
| P | show **pairs** the broad phase handed to the narrow phase (red = real collision) |
| V | colour balls by **speed** |
| K | **trails** |
| G | **gravity** (a good worst case for the uniform grid) |
| B | cycle **bounciness** 1.0 / 0.8 / 0.5 / 0.2 |
| SPACE | pause |
| F12 | screenshot (built into raylib, saved next to the exe) |
| F11 | borderless fullscreen on / off |
| ESC | back to the menu |

The panel on the left shows comparisons per step, collisions, collision time, a live frame-time graph
(coloured by algorithm, so switching algorithms shows up as a colour change in the line) and a table with the
last measurement of every algorithm. Rows that were measured at a different object count are dimmed.

## HOW IT WORKS

* **Brute force**: one ball sends an arrow to every other ball; each arrow is a comparison and the caption shows the distance versus the sum of the radii.
* **Spatial grid**: balls are inserted into cells one by one, then each cell is compared with itself and its 4 forward neighbours.
* **Quadtree**: balls are inserted one by one (watch full leaves split into 4), then every ball queries a box around itself and only the balls in the touched leaves are tested.
* **Sweep and prune**: balls are sorted by left edge, then a band sweeps along the x axis; the sweep stops as soon as the next ball starts to the right of the band.

Keys: SPACE play/pause, LEFT/RIGHT step back/forward, UP/DOWN speed, R restart, 1-4 switch algorithm.
The step lists are computed with the same `SpatialGrid` and `Quadtree` classes the sandbox uses, and the tests check that their comparison counts match the real implementations.

## How the simulation works

* Fixed 120 Hz physics step (independent of frame rate), max 4 steps per frame.
* Each step: build the broad-phase structure → **detect** colliding pairs (read-only, so it can run on several threads) → **resolve** all pairs sequentially. This keeps results independent of loop order and thread count.
* Collision response is an impulse weighted by mass (mass ∝ r²) with adjustable restitution, plus a mass-weighted positional correction.
* All data lives in structure-of-arrays form (`Obiecte`) so the hot loops only touch positions and radii.

## Project layout

```
src/Object.*     balls (structure of arrays), physics, spawning, ball texture
src/Spatial.*    SpatialGrid (flat, counting sort) and Quadtree (pooled nodes, no per-frame allocation)
src/Collision.*  the four algorithms, threading, candidate recording, heatmap
src/UI.*         HUD, start screen, force-field ring
src/Modes.*      HOW IT WORKS explainer and BENCHMARK mode
src/main.cpp     window, state machine, input
tests/           headless tests (no window needed)
```

## Tests

`tests/run_tests.bat` (or `run_tests.sh`) builds the logic against a tiny fake of raylib and checks:

* all four algorithms find exactly the same colliding pairs as brute force (with and without threads, with giant balls)
* energy conservation, repulsion/attraction, damping, gravity, restitution
* the grid resizes for giants and shrinks back
* the explainer's step lists match the real algorithms' comparison counts
* the benchmark produces 20 results and a correct CSV

## Tuning

`Config` in `src/Object.h`: ball size, force strength, gravity, physics step, thread count. Keep `MarimeCelula >= 2 * RazaMax` (a `static_assert` checks it).