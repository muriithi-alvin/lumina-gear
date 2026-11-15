# Lumina Gear

A 2D Mechanical Power, 4-Stroke Valvetrain & Illumination Physics Simulator  
*Computer Graphics Course Project — Checkpoint 1, Checkpoint 2 & Checkpoint 3 (Complete)*

---

## 📖 Table of Contents
1. [Project Overview](#project-overview)
2. [Project Evolution: Proposed Idea vs. Current Transformation](#project-evolution-proposed-idea-vs-current-transformation)
3. [Checkpoints Status Summary](#checkpoints-status-summary)
4. [How the Game is Played](#how-the-game-is-played)
   - [Core Gameplay Mechanics](#core-gameplay-mechanics)
   - [The 5-Level Progressive Engineering Campaign](#the-5-level-progressive-engineering-campaign)
   - [Complete Controls Reference](#complete-controls-reference)
5. [Team Structure & Responsibilities](#team-structure--responsibilities)
6. [Computer Graphics Techniques & Algorithms Implemented](#computer-graphics-techniques--algorithms-implemented)
   - [Custom Software Rasterization (CP1 & CP2)](#custom-software-rasterization-cp1--cp2)
   - [OpenGL Hardware-Accelerated Pipeline (CP3)](#opengl-hardware-accelerated-pipeline-cp3)
7. [System Architecture & Level Modularization](#system-architecture--level-modularization)
8. [How to Compile and Run](#how-to-compile-and-run)
9. [Automated Testing & Verification](#automated-testing--verification)
10. [Checkpoint 3 (CP3: OpenGL Integration & Hardware Lighting Prototype)](#checkpoint-3-cp3-opengl-integration--hardware-lighting-prototype)

---

## 🌟 Project Overview

**Lumina Gear** is an interactive 2D engineering physics and computer graphics simulator built completely from first principles in **modern C++17**. The project implements a **dual-renderer graphics engine**:
1. **CPU Software Rasterizer (CP1 & CP2):** A custom, pixel-by-pixel rendering engine computing line rasterization, circle geometry, scanline polygon filling, and inverse-square lighting entirely on the CPU without third-party graphics libraries.
2. **GPU Hardware OpenGL Engine (CP3):** A modern hardware-accelerated pipeline utilizing native OpenGL contexts, 2D orthographic projections, GPU triangle fans, and hardware additive alpha blending for dynamic lighting and glow diffusion.

Players assemble functional machines from motors, spur gears, timing belt pulleys, alternators, and 4-stroke internal combustion valvetrains. As mechanical power propagates through gear ratios and belt drives, generators convert angular velocity into electrical potential ($0\text{V} - 14.4\text{V}$ DC) to illuminate a dark workshop using radial inverse-square lighting.

---

## 🔄 Project Evolution: Proposed Idea vs. Current Transformation

| Feature / System | Checkpoint 1 (Proposed Baseline) | Current Transformation (CP1 + CP2 + CP3 Complete) | Engineering Justification |
| :--- | :--- | :--- | :--- |
| **System State** | Static, read-only scene with fixed components. | Continuous, real-time physics and kinematics engine ($\Delta t$ integration). | Real-time angular velocity transfer and continuous animation. |
| **Interactivity** | Play / Pause toggled a simple boolean state. | Direct manipulation: 2D translation drag-and-drop, magnetic mesh snapping, and part deletion. | Transforms the static viewer into a hands-on physics workbench. |
| **Level Structure** | Single hardcoded layout. | Modular 5-Level Progressive Engineering Physics Campaign. | Progressively introduces gear ratios, polarity, and engine timing. |
| **Mechanical Scope** | Motor $\to$ 2 Gears $\to$ Pulley $\to$ Belt $\to$ Generator $\to$ Bulb. | Expanded with a complete **4-Stroke Internal Combustion Valvetrain Engine** bench. | Demonstrates harmonic reciprocating motion, cam lobes, and valve lift. |
| **Collision Physics** | None (components could overlap statically). | **Real-Time Interference Collision Detection** at Top Dead Center (TDC). | Simulates physical valve-piston crashes with impact sparks. |
| **Display Pipeline** | Fixed-size window; blurry nearest-neighbor stretch when maximized. | Aspect-ratio preserving letterbox/pillarbox with Win32 GDI `HALFTONE` blitting. | Ensures razor-sharp display at any monitor resolution or aspect ratio. |
| **User Interface** | Fixed bottom helper banner that covered lower components. | **Modular Dynamic Tip Card**: Draggable anywhere, collapsible (<kbd>H</kbd>), and auto-dodges parts. | Eliminates component obstruction on the lower engine deck. |
| **Rendering Backend** | Single CPU software framebuffer. | **Dual Pipeline**: Custom CPU Software Rasterizer + **OpenGL Hardware Accelerated GPU Engine** (`lumina_gl.exe`). | Proves both first-principles graphics algorithms and modern GPU hardware acceleration. |

---

## 🏁 Checkpoints Status Summary

| Checkpoint | Target | Focus & Core Achievements | Status |
| :---: | :--- | :--- | :---: |
| **CP1** | **Technical Foundation** | First-principles C++ software rasterization: Bresenham lines, Midpoint circles, scanline polygon filling, BMP writer, static scene. | **COMPLETE** |
| **CP2** | **Transformation & Animation** | Continuous kinematics engine, 2D affine rotation matrices, harmonic piston motion, 4-stroke valvetrain, TDC collision detection, 5-level campaign. | **COMPLETE** |
| **CP3** | **OpenGL Integration & Prototype** | Native WGL context, hardware 2D orthographic projection, GPU triangle fans and quads, additive alpha-blended radial glow, `lumina_gl.exe`. | **COMPLETE** |
| **Final** | **Refinement & Polish** | Dual-renderer execution, flicker-free drag/drop, crisp halftone blitting, full test suite pass (100%). | **COMPLETE** |

---

## 🎮 How the Game is Played

### Core Gameplay Mechanics
In Lumina Gear, power flows through interconnected physical and electrical loops:
1. **Mechanical Drive:** The electric motor or engine crankshaft provides input angular velocity ($\omega$).
2. **Speed & Torque Transformation:** Meshing gears alter speed and torque according to tooth counts:
   $$\omega_2 = -\omega_1 \cdot \left(\frac{N_1}{N_2}\right)$$
3. **Flexible Belt Transmission:** Pulleys transmit rotation across distances. Crossed belts invert direction:
   $$\omega_{\text{wheel}} = \pm \omega_{\text{pulley}} \cdot \left(\frac{r_{\text{pulley}}}{r_{\text{wheel}}}\right)$$
4. **Electrical Generation:** An alternator/dynamo converts generator rotational speed into DC voltage ($0\text{V} - 14.4\text{V}$ DC).
5. **Illumination Load:** The workshop lamp lights up when receiving $\ge 12.0\text{V}$ DC with correct positive polarity.

---

### The 5-Level Progressive Engineering Campaign

#### Level 1: Baseline Drive Train (Target 12V DC)
- **Objective:** Assemble the original assignment machine to power the illumination bulb.
- **Challenge:** Mount the electric motor, 12T driver gear, 18T driven gear, drive pulley, timing belt, generator wheel, and target bulb.
- **Physics Learned:** Direction inversion through meshed spur gears and speed reduction ($12\text{T} \to 18\text{T} = 1:1.5$).

#### Level 2: Overdrive Speed Step-Up (Target $\ge$ 12.0V DC)
- **Objective:** Overcome a low-speed motor to generate full operating voltage.
- **Challenge:** The motor speed is reduced to half-speed (600 RPM). The baseline $12\text{T} \to 18\text{T}$ reduction produces only 7.2V DC (bulb stays off).
- **Solution:** Swap the gear order to establish an **overdrive step-up** ($18\text{T} \text{ Motor} \to 12\text{T} \text{ Pulley}$). The $1.5\times$ speed multiplier steps generator speed up, achieving 14.4V DC and illuminating the bulb.

#### Level 3: Rotational Polarity & Idler Reversal
- **Objective:** Correct reverse rotational polarity when the motor spins counter-clockwise.
- **Challenge:** Motor spins CCW (-). A standard 2-gear mesh spins the generator backward, triggering a reverse-polarity safety shutdown.
- **Solution A (Crossed Belt):** Press <kbd>B</kbd> or click the belt midpoint to cross the timing belt into a figure-eight, reversing pulley rotation.
- **Solution B (14T Idler Gear):** Drag the 14T Idler Gear between Gear 1 and Gear 2, inserting a second direction inversion that restores clockwise rotation.

#### Level 4: 4-Stroke Valvetrain Timing Bench (Avoid Crash!)
- **Objective:** Synchronize overhead camshaft rotation to prevent catastrophic valve-piston interference.
- **Challenge:** The bench starts with an incorrect 12-tooth cam sprocket (1:1 ratio). Pressing Play causes the poppet valve to open while the piston is at Top Dead Center (TDC), triggering a **fatal collision** with impact sparks!
- **Solution:** Mount the **24-tooth Cam Sprocket** from the toolbox to establish the exact 2:1 reduction ratio ($N_{\text{cam}} / N_{\text{crank}} = 24 / 12 = 2.0$). The camshaft turns at exactly half crankshaft speed, cycling through Intake, Compression, Power, and Exhaust with zero collisions.

#### Level 5: Master Power Plant Grid Integration
- **Objective:** Integrate a heavy-duty internal combustion engine with the municipal electrical grid.
- **Challenge:** The engine timing is verified, but the electrical generation system is unbuilt in the toolbox.
- **Solution:** Mount the 12V Dynamo Alternator to the lower engine deck ($x=520, y=560$), tension the drive belt, and wire the 100W Workshop Lamp ($x=1080, y=500$). At 1200 RPM, the engine produces 185 HP, generating 14.4V DC and illuminating the entire facility.

---

### Complete Controls Reference

| Control | Action | Function |
| :--- | :--- | :--- |
| **Mouse Left Drag** | Drag component | Move any placed component or pick new parts from the left toolbox. |
| **Mouse Left Release** | Drop component | Automatically snaps gears into mesh or onto drive shafts if within range. |
| **Mouse Right Click** | Click on part | Delete/remove the clicked component from the workbench. |
| **Drag to Toolbox** | Drag over toolbar | Dragging any canvas part back over the toolbar removes it. |
| <kbd>Del</kbd> / <kbd>Backspace</kbd> | Press key | Delete the currently selected component. |
| <kbd>Space</kbd> | Press key | **Play / Pause** simulation playback. |
| <kbd>1</kbd>, <kbd>2</kbd>, <kbd>3</kbd>, <kbd>4</kbd>, <kbd>5</kbd> | Press key | Instantly switch between Campaign Levels 1 through 5. |
| <kbd>S</kbd> | Press key | **Auto-Solve** current level (instantly positions correct physics configuration). |
| <kbd>C</kbd> | Press key | **Clear** workbench (removes parts to build from scratch). |
| <kbd>R</kbd> | Press key | **Reset** simulation rotation angles back to zero. |
| <kbd>B</kbd> | Press key | **Cross / Uncross Belt** (toggles figure-eight belt drive in Level 3). |
| <kbd>H</kbd> | Press key | **Minimize / Expand Tip Card** (collapses card into a minimal pill). |
| **Tip Card Drag** | Drag header | Click and drag the briefing card anywhere on the canvas. |

---

## 👥 Team Structure & Responsibilities

The team consists of three dedicated pairs covering the full software and hardware graphics stack:

| Pair | Members | Checkpoint Owned | Primary Technical Contributions | Status |
| :--- | :--- | :--- | :--- | :---: |
| **Pair A** | **Muriithi Alvin** (166326) [Lead]<br>**Kariuki Mary Wambui** (166382) | **CP1: Technical Foundation** | Software rasterizer primitives (Bresenham line, Midpoint circle, scanline fill), BMP writer, static scene foundation. | **COMPLETE** |
| **Pair B** | **Tevin Tajeu** (151785)<br>**Jude Makau** (166487) | **CP2: Transformation, Kinematics & Valvetrain** | Continuous simulation engine, 4-stroke valvetrain dynamics, TDC collision detection, flicker-free drag/drop, GDI halftone scaling, modular level architecture. | **COMPLETE** |
| **Pair C** | **Billy John Igiraneza** (167022)<br>**Glenn Karenju** (109511) | **CP3: OpenGL Integration & Prototype** | Native WGL context creation, hardware 2D orthographic projection, GPU triangle fans and quads, additive alpha-blended radial glow, `lumina_gl.exe`. | **COMPLETE** |

---

## 📐 Computer Graphics Techniques & Algorithms Implemented

### Custom Software Rasterization (CP1 & CP2)

#### 1. Midpoint Circle Algorithm (`src/circle.cpp`)
- **Used by:** Gear rims, pulley hubs, motor rotor, lightbulb globe, alternator casing.
- **Mathematical Principle:** Evaluates the circle decision parameter $p_k = 1 - r$. If $p_k < 0$, the next pixel is $(x_k + 1, y_k)$; otherwise $(x_k + 1, y_k - 1)$. Employs 8-way symmetry using pure integer arithmetic with zero floating-point operations.

#### 2. Bresenham's Line Algorithm & Thick Lines (`src/line.cpp`)
- **Used by:** Drive shafts, timing belt boundaries, valve stems, connecting rods, spark particles, wiring.
- **Mathematical Principle:** Increments by unit steps along the major axis while tracking integer error terms ($2\Delta y - \Delta x$). Thick lines compute perpendicular unit vectors to render robust parallel sweeps.

#### 3. Scanline Polygon Filling (`src/polygon.cpp`)
- **Used by:** Gear involute teeth, piston crown, cylinder walls, valve poppets, UI cards.
- **Mathematical Principle:** Implements the even-odd parity rule. Finds edge intersections along horizontal scanlines, sorts intersection $x$-coordinates, and fills interior spans.

#### 4. 2D Affine Transformation Operators (`src/components.cpp`, `src/scene.cpp`)
- **Used by:** Rotating gears, cam lobes, crank throws, connecting rods.
- **Mathematical Principle:** 2D homogeneous transformation matrix for rotation around pivot $(x_c, y_c)$ by angle $\theta$:
  $$\begin{bmatrix} x' \\ y' \\ 1 \end{bmatrix} = \begin{bmatrix} 1 & 0 & x_c \\ 0 & 1 & y_c \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} \cos\theta & -\sin\theta & 0 \\ \sin\theta & \cos\theta & 0 \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} 1 & 0 & -x_c \\ 0 & 1 & -y_c \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} x \\ y \\ 1 \end{bmatrix}$$

#### 5. Harmonic Reciprocating Kinematics & Connecting Rod Obliquity
- **Used by:** Piston reciprocating stroke and articulated connecting rod.
- **Kinematic Equations:**
  $$y(\theta) = y_{\text{TDC}} + r(1 - \cos\theta)$$
  $$\phi(\theta) = \arcsin\left(\frac{r}{L} \sin\theta\right)$$
  *(where $r = 38\text{px}$ crank radius, $L = 105\text{px}$ rod length).*

#### 6. Inverse-Square Radial Lighting & Dynamic Darkening (`src/scene.cpp`)
- **Used by:** Workshop lamp glow, room illumination, power stroke combustion flash.
- **Mathematical Principle:** Perceptual radial falloff with 16 blended alpha rings:
  $$I(d) = I_0 \cdot \left(1.0 - \left(\frac{d}{R_{\max}}\right)^2\right)$$
  $$C_{\text{final}} = C_{\text{base}} \times (\text{darkness} + \text{glow})$$

#### 7. Crisp Halftone Letterboxing (`src/main_window.cpp`)
- **Used by:** Win32 interactive full-screen display.
- **Mathematical Principle:** Computes uniform scaling $s = \min(w / W_0, h / H_0)$ with centered offsets $(d_x, d_y)$. Uses Win32 `SetStretchBltMode(dc, HALFTONE)` to resample pixels crisply, preventing bilinear blur when maximized.

---

### OpenGL Hardware-Accelerated Pipeline (CP3)

#### 1. Hardware 2D Orthographic Projection (`src/gl_renderer.cpp`)
- Sets up the GPU projection matrix to match the $1360 \times 760$ coordinate space:
  ```cpp
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, 1360.0, 760.0, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  ```

#### 2. GPU Triangle Fans & Hardware Quads
- Gear hubs, pulleys, and cam lobes are rasterized using `GL_TRIANGLE_FAN` with polar coordinates $(\cos\theta, \sin\theta)$.
- Gear teeth, belt runs, and piston structures are rendered as hardware quad strips (`GL_QUADS` and `GL_TRIANGLE_STRIP`).

#### 3. Hardware Additive Alpha Blending (`src/gl_renderer.cpp`)
- The lightbulb glow and workshop illumination utilize GPU blending:
  ```cpp
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Pure additive light accumulation
  ```
- Renders 18 concentric radial triangle fans with quadratic alpha falloff, delivering smooth, glowing illumination at high frame rates.

---

## 🏛️ System Architecture & Level Modularization

To ensure that multiple team members can contribute levels simultaneously without merge conflicts, the campaign is decoupled into standalone modules:

```
Luminar-flow/
├── include/
│   └── lumina/
│       ├── level.h                 # Abstract base Level interface & factory
│       ├── gl_renderer.h           # CP3: Hardware OpenGL renderer interface
│       ├── simulation.h            # Continuous kinematics simulation engine
│       ├── scene.h                 # Scene composition and tip card bounds
│       ├── components.h            # 2D component geometric definitions
│       └── levels/                 # [LEVEL HEADERS FOLDER]
│           ├── level1.h            # Level 1: Baseline Drive Train
│           ├── level2.h            # Level 2: Overdrive Speed Step-Up
│           ├── level3.h            # Level 3: Rotational Polarity & Idler
│           ├── level4.h            # Level 4: 4-Stroke Valvetrain Timing Bench
│           └── level5.h            # Level 5: Master Power Plant Grid
└── src/
    ├── levels/                     # [LEVEL SOURCE IMPLEMENTATIONS]
    │   ├── level_registry.cpp      # C++17 auto-detecting level factory
    │   ├── level1.cpp              # Level 1 Implementation
    │   ├── level2.cpp              # Level 2 Implementation
    │   ├── level3.cpp              # Level 3 Implementation
    │   ├── level4.cpp              # Level 4 Implementation
    │   └── level5.cpp              # Level 5 Implementation
    ├── gl_renderer.cpp             # CP3: OpenGL hardware graphics pipeline
    ├── main_opengl.cpp             # CP3: Interactive OpenGL executable
    ├── simulation.cpp              # Physics propagation & kinematics solver
    ├── scene.cpp                   # Software rendering pipeline & tip cards
    └── main_window.cpp             # Win32 GDI window & direct manipulation
```

### Auto-Detecting Registry (`src/levels/level_registry.cpp`)
Using C++17 `__has_include`, the project compiles cleanly whether all levels or only a subset are present. Whenever a teammate drops `levelX.h` and `levelX.cpp` into their respective folders, CMake automatically compiles them via `file(GLOB ... src/levels/*.cpp)` with zero manual edits required.

---

## 🛠️ How to Compile and Run

### Prerequisites
- **C++17 Compiler:** MSVC (Visual Studio 2022 / Build Tools) or MinGW-w64 GCC.
- **CMake:** Version 3.10 or newer.
- **OpenGL:** Standard system OpenGL (`opengl32.lib`, `<GL/gl.h>` built into Windows).

### Build Instructions (Windows - Visual Studio)
```bash
# 1. Configure the build
cmake -S . -B build

# 2. Compile all targets in Release mode
cmake --build build --config Release
```

### Launching the Executables

#### 1. Software Rasterizer Window (CP1 & CP2)
```bash
build\Release\lumina_gear_window.exe
```
*Runs the custom CPU software rasterizer proving from-scratch line, circle, scanline fill, and continuous kinematics.*

#### 2. OpenGL Hardware-Accelerated Window (CP3)
```bash
build\Release\lumina_gl.exe
```
*Runs the hardware-accelerated OpenGL prototype with GPU-rendered geometry and hardware additive alpha-blended lighting.*

#### 3. Headless BMP Generator
```bash
build\Release\lumina_gear.exe
```
*Generates baseline output images: `lumina_gear_cp1_powered.bmp`, `lumina_gear_cp1_unpowered.bmp`, and `lumina_assembly_complete.bmp`.*

---

## 🧪 Automated Testing & Verification

The project includes an extensive test suite verifying algorithm correctness, picking, electrical potential, and valvetrain timing:

```bash
# Run all automated tests via CTest
cd build
ctest -C Release --output-on-failure
```

Or run the test executables directly:
```bash
# Kinematics, levels, and valve collision tests
build\Release\test_simulation.exe

# Software rasterization geometry and scanline tests
build\Release\test_algorithms.exe
```
**Current Status:** `100% Tests Passed` (0 failures).

---

## 💡 Checkpoint 3 (CP3: OpenGL Integration & Hardware Lighting Prototype)

**Checkpoint 3 (CP3)** represents the transition of Lumina Gear from a CPU software framebuffer into a **hardware-accelerated GPU pipeline**. 

### Key Technical Achievements in CP3:
1. **Zero External Dependencies:** Built using native Win32 WGL context creation (`ChoosePixelFormat`, `SetPixelFormat`, `wglCreateContext`, `wglMakeCurrent`) and standard system `opengl32.lib`.
2. **GPU Geometry Rendering:** Component data from `Simulation` is converted into GPU primitives (`GL_TRIANGLE_FAN` hubs, `GL_QUADS` gear teeth, and quad strip belts).
3. **Hardware Lighting Model:** Implements hardware additive alpha blending (`glBlendFunc(GL_SRC_ALPHA, GL_ONE)`) with multi-ring radial triangle fans, producing realistic, luminous light diffusion around the bulb filament.
4. **Architectural Parity:** `lumina_gl.exe` connects to the exact same `Simulation` kinematics engine as `lumina_gear_window.exe`, allowing instant cross-comparison between CPU software rendering and GPU hardware acceleration.

---

*Developed for the Computer Graphics Course (4.2) at Strathmore University.*
