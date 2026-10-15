# Lumina Gear

A 2D Mechanical Power and Illumination Simulator (Computer Graphics group project).

## Description

In Lumina Gear, users build a machine from a motor, gears, pulleys and a
lightbulb. When power reaches the bulb, the dark room gets brighter and the
bulb gives off a soft glow.

The project is built in four checkpoints:

| Checkpoint | Focus |
|---|---|
| CP1: Technical Foundation | Our own C++ drawing algorithms and the static scene |
| CP2: Transformation and Animation | Moving, rotating and connecting parts; power flowing through the machine |
| CP3: OpenGL Integration and Prototype | Moving the renderer to OpenGL |
| Final Project | Interface polish, efficiency rules, lighting and a demo |

## Group Members and Responsibilities

The team has six members working in three pairs. Each pair owns one checkpoint
and one area of the final project, and reviews the other pairs' work.

| Pair | Members | Checkpoint owned | Final project area |
|---|---|---|---|
| A | Muriithi Alvin (Project Lead), _[name]_ | CP1: Technical Foundation | Interface and testing |
| B | _[name]_, _[name]_ | CP2: Transformation and Animation | Simulation and animation |
| C | _[name]_, _[name]_ | CP3: OpenGL Integration and Prototype | Graphics and lighting |

The Project Lead also manages the GitHub repository, creates the checkpoint
branches, and merges each finished checkpoint into `main`.

**CP1 tasks (Pair A):**

| Partner | Responsibilities |
|---|---|
| Muriithi Alvin | CMake build, framebuffer and BMP output, Bresenham line, midpoint circle, polygon fill |
| _[name]_ | Motor, gear, pulley and bulb shapes, belt, toolbar and buttons, picking, alpha blending and glow |
| Both | Tests, README and screenshots |

## Technologies and Tools

- **C++17**, using only the standard library. There is no graphics library:
  every pixel is written to our own framebuffer by algorithms we wrote.
- **CMake** (3.10 or newer) to build, and **CTest** to run the tests.
- **Compilers:** MSVC (Visual Studio Build Tools) or MinGW-w64 g++ through
  MSYS2 on Windows; GCC or Clang on Linux and macOS.
- **Win32 API** (`user32`, `gdi32`), used only by the interactive window. It
  shows the finished framebuffer and reports the mouse. It does no drawing.
- **BMP image format**, written by our own code.
- **Git and GitHub** for version control (one branch per checkpoint, Pull
  Requests for every task).
- **VS Code** with the C/C++ and CMake Tools extensions.
- **OpenGL** is planned for CP3.

## Graphics Techniques and Algorithms Implemented

| Technique | Where | Used for |
|---|---|---|
| Bresenham line algorithm | `src/line.cpp` | Belts, shafts, outlines, spokes, filament. Thick lines are drawn as parallel Bresenham lines. |
| Midpoint circle algorithm | `src/circle.cpp` | Pulleys, hubs, bulb, glow rings. Filled discs use horizontal spans from the same algorithm. |
| Scanline polygon fill (even-odd rule) | `src/polygon.cpp` | Gear bodies, motor, buttons, toolbar |
| Gear outline from polar coordinates | `src/components.cpp` | Each tooth's four corners are found with `cos` and `sin` from the gear's angle, so rotation only changes one value |
| Tangent lines between two circles | `src/belt.cpp` | The two straight runs of a belt between pulleys of different sizes |
| Alpha blending | `src/framebuffer.cpp`, `src/effects.cpp` | Soft glow around the bulb (16 blended rings) |
| Darkness overlay with additive light | `src/effects.cpp` | A dark room that the bulb's light "pierces", fading with distance |
| Point-in-Circle picking | `include/lumina/picking.h` | Detecting a click on a part (compares squared distances, no square root) |
| 5x7 bitmap font | `src/ui.cpp` | Button labels |

| Powered | Unpowered |
|---|---|
| ![Powered scene](docs/screenshots/cp1_powered.png) | ![Unpowered scene](docs/screenshots/cp1_unpowered.png) |

## How to Compile and Run

You need a C++17 compiler and CMake 3.10 or newer. `docs/CODE-GUIDE.md`
explains how to install them and how to run the project in VS Code.

**Windows (Visual Studio or Build Tools):**

```bash
cmake -S . -B build
cmake --build build --config Release
build\Release\lumina_gear.exe
```

**Windows (MinGW-w64 through MSYS2):**

```bash
cmake -S . -B build -G Ninja
cmake --build build
build\lumina_gear.exe
```

(Use `-G "MinGW Makefiles"` if you do not have Ninja.)

**Linux or macOS:**

```bash
cmake -S . -B build
cmake --build build
./build/lumina_gear
```

The program writes two images in the folder you run it from:
`lumina_gear_cp1_powered.bmp` and `lumina_gear_cp1_unpowered.bmp`.

### Interactive window (Windows only)

The Windows build also makes `lumina_gear_window`, which shows the same scene
in a window: `build\lumina_gear_window.exe` (MinGW) or
`build\Release\lumina_gear_window.exe` (Visual Studio).

- **Play** powers the machine, **Pause** turns it off, and **Reset** turns it
  off and clears the selection.
- Click a part to select it. The title bar shows the power state and the
  selected part.
- Buttons and toolbar slots light up under the mouse.

### Run the tests

```bash
cd build
ctest --output-on-failure
```

(With Visual Studio add `-C Release`.) The tests compare our circle, line and
polygon algorithms against independent reference calculations. They also check
picking, gear meshing, blending, the button and toolbar hit areas, and the
window's hover and selection drawing.

## Current Progress

**CP1 (Technical Foundation) is complete** and merged into `main`.

- Every shape is drawn by our own line, circle and polygon algorithms.
- The scene shows a motor, two meshed gears, a pulley joined by a belt to a
  smaller wheel, and a lightbulb, with a toolbar and Play, Pause and Reset
  buttons.
- Powered and unpowered versions of the scene: the room is dark until the bulb
  is lit, then the bulb glows and lights the area around it.
- An interactive window on Windows with clickable buttons, part selection and
  hover highlights.
- Self-checking tests for the algorithms, picking, effects and interface.

**Next (CP2, Pair B):** rotate gears and pulleys each frame, drag parts onto
the canvas, connect them, pass speed through the chain using gear ratios, and
make the room's brightness depend on the power that reaches the bulb. The CP1
code was written with these in mind; see Part 6 of `docs/CODE-GUIDE.md`.

## Known Limitations and Issues

- **The scene is static.** Gears and pulleys do not rotate yet, and power is a
  simple on/off switch rather than something passed through the machine.
- **The layout is fixed.** Parts cannot be added, moved or removed. Toolbar
  slots light up under the mouse, but clicking them does nothing yet.
- **The interactive window is Windows only**, because it uses the Win32 API.
  On Linux and macOS the program only saves the BMP images.
- **No anti-aliasing.** Lines and circle edges have visible "staircase" steps,
  as expected from the basic algorithms.
- **Rendering runs on the CPU, one pixel at a time.** A full redraw takes
  about 175 ms in a Debug build, so the window only redraws the scene when the
  power state or selection changes. Use a Release build for faster response.
- **Images are only saved as BMP**, in the folder the program runs from, and
  each run overwrites the previous images.

See `docs/git-workflow.md` for the team's branch and Pull Request rules.
