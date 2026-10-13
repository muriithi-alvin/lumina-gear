# Lumina Gear CP1: Code Guide

This guide explains how to set up your computer, how to run the project in VS Code, and what every file does. Read it from top to bottom once, then use it as a reference.

---

## Part 1: What You Need to Install

Everything below is free. You only need to do this once.

| # | Tool | Why you need it |
|---|---|---|
| 1 | **Visual Studio Code** | The editor you will use. |
| 2 | **A C++ compiler** | Turns our C++ code into a program. Choose **one** option (see below). |
| 3 | **CMake** (version 3.10 or newer) | Reads `CMakeLists.txt` and builds the project. |
| 4 | **Git** | Version control, as described in the team Git guide. |
| 5 | **VS Code extensions** | "C/C++" and "CMake Tools", both from Microsoft. |

No graphics library is needed for CP1. The project uses only standard C++.

### Step 1: Install VS Code

Download it from https://code.visualstudio.com and run the installer. While installing, tick **Add to PATH**.

### Step 2: Install a C++ compiler (choose ONE)

**Option A (recommended): Microsoft C++ Build Tools (MSVC)**

1. Download "Build Tools for Visual Studio" from https://visualstudio.microsoft.com/downloads (look under *Tools for Visual Studio*).
2. In the installer, tick **Desktop development with C++** and click Install. This is a large download (several GB).

Or, from a terminal (PowerShell):

```powershell
winget install Microsoft.VisualStudio.2022.BuildTools --override "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

**Option B: MinGW-w64 through MSYS2 (smaller download)**

1. Install MSYS2 from https://www.msys2.org
2. Open the **MSYS2 UCRT64** terminal and run:

   ```bash
   pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja
   ```

3. Add `C:\msys64\ucrt64\bin` to your Windows **Path** (Settings > System > About > Advanced system settings > Environment Variables > Path > New).

Use **only one** of the two options. Do not mix them.

### Step 3: Install CMake

If you chose Option B, CMake is already installed. Otherwise:

```powershell
winget install Kitware.CMake
```

(or download it from https://cmake.org/download and tick **Add CMake to the system PATH**).

### Step 4: Install Git

```powershell
winget install Git.Git
```

(or download it from https://git-scm.com).

### Step 5: Install the VS Code extensions

Open VS Code, press `Ctrl+Shift+X`, then search for and install:

- **C/C++** (publisher: Microsoft)
- **CMake Tools** (publisher: Microsoft)

### Step 6: Check everything works

**Close and reopen** your terminal (or VS Code) so the new programs are found, then run:

```powershell
git --version
cmake --version
```

For Option A, open **"Developer PowerShell for VS 2022"** from the Start menu and run `cl`. For Option B, run `g++ --version`. If a version number or compiler banner appears, you are ready.

### Which one is correct for my computer?

You only need one compiler. If you are unsure, use Option A. VS Code's CMake Tools can find it by itself.

---

## Part 2: Open and Run the Project in VS Code

### Open the project

1. Start VS Code.
2. Choose **File > Open Folder** and select the `lumina-gear` folder.
3. Click **Yes, I trust the authors** if VS Code asks.

### Select the compiler (called a "kit")

1. Press `Ctrl+Shift+P` and type **CMake: Select a Kit**.
2. Choose your compiler:
   - For Option A: **Visual Studio Build Tools 2022 Release - amd64**
   - For Option B: **GCC ... ucrt64**

If no kit appears, choose **CMake: Scan for Kits** first.

### Build and run

1. Press `Ctrl+Shift+P`, type **CMake: Configure**, and press Enter. (Wait for "Configuring done".)
2. Press `Ctrl+Shift+P`, type **CMake: Build**, and press Enter. (Or press `F7`.)
3. Run the program: **CMake: Run Without Debugging**. Choose `lumina_gear` if asked.

Two images appear in the folder where the program runs (usually the `build` folder, or `build\Debug` / `build\Release` with MSVC):

- `lumina_gear_cp1_powered.bmp`
- `lumina_gear_cp1_unpowered.bmp`

Double-click either one to open it in the Windows Photos app.

### Run the tests

Press `Ctrl+Shift+P`, type **CMake: Run Tests**. The result should say `100% tests passed`.

### Using the terminal instead

Open a terminal in VS Code (**Terminal > New Terminal**) and run:

```powershell
cmake -S . -B build
cmake --build build --config Release
build\Release\lumina_gear.exe        # with MSVC
build\lumina_gear.exe                # with MinGW
cd build
ctest -C Release --output-on-failure
```

### Common problems

| Problem | Fix |
|---|---|
| `cmake` is not recognised | Close and reopen the terminal or VS Code. Check that CMake is in your Path. |
| "No kits found" | Run **CMake: Scan for Kits**. Check that the compiler is installed (Step 6). |
| `cl` is not recognised | Open **Developer PowerShell for VS 2022**, or just use the CMake Tools buttons in VS Code. |
| Cannot find the `.bmp` files | Search the `build` folder, including `Debug` and `Release` subfolders. |
| Red squiggles under `#include "lumina/..."` | Run **CMake: Configure** once. The C/C++ extension then learns the include folder. |
| Build is slow the first time | Normal. Later builds only rebuild changed files. |

---

## Part 3: The Big Picture

### What the program does

CP1 draws one picture, twice. The picture is the Lumina Gear scene: a motor, two gears, a pulley, a belt and a lightbulb. One copy shows the machine **powered** (bulb on, room brighter) and the other **unpowered** (everything dim).

We do not use any drawing library. Instead:

1. We make a **grid of pixels in memory** (the *framebuffer*).
2. Our own algorithms colour individual pixels: lines, circles, filled shapes.
3. We build bigger things (gears, bulb, buttons) from those shapes.
4. At the end we save the grid as a `.bmp` picture file.

### How the files depend on each other

```
framebuffer   <- the pixel grid. Everything draws into this.
   ^
   |-- line       (Bresenham line)
   |-- circle     (midpoint circle)   uses framebuffer
   |-- polygon    (scanline fill)     uses framebuffer + line
   |-- picking    (click test)        stands alone, just maths
   |
   |-- components (motor, gear, pulley, bulb)   uses circle, line, polygon
   |-- belt       (belt between circles)        uses line
   |-- effects    (glow and darkness)           uses circle, framebuffer
   |-- ui         (toolbar, buttons, text)      uses components, circle, line, polygon
   |
   `-- scene      (puts it all together)        uses everything above
          ^
          |
        main      (runs the scene, saves the images)
```

Lower files know nothing about the files above them. This is why the team can work on different files at the same time.

### What a pixel and a coordinate mean

- The picture is 1000 pixels wide and 600 pixels tall.
- Position `(x, y)` counts **from the top-left corner**. `x` grows to the right and `y` grows **downwards**.
- A colour is three numbers from 0 to 255: red, green and blue.

---

## Part 4: File-by-File Explanation

### Build and project files

| File | What it does |
|---|---|
| `CMakeLists.txt` | The recipe for building the project. It collects all files in `src/` (except `main.cpp`) into a library called `lumina`. It then builds the program `lumina_gear` (from `main.cpp`) and one test program for each `tests/test_*.cpp` file. Both use the library. The library is shared so that tests and the program use exactly the same code. |
| `.gitignore` | Lists files Git must not track: the `build/` folder, compiled files and `.bmp` images (they are re-created every run). |
| `README.md` | Short project overview and build instructions. |
| `docs/CODE-GUIDE.md` | This guide. |
| `docs/git-workflow.md` | The team's Git rules. |
| `docs/project-lead-guide.md` | Extra steps for the project lead. |
| `docs/screenshots/` | The two sample pictures used in the README. |

---

### `include/lumina/framebuffer.h` and `src/framebuffer.cpp`: the pixel grid

**Purpose:** Hold the picture in memory and save it to a file.

- `struct Color` holds red, green, blue (0 to 255).
- `class Framebuffer` holds `width x height` colours in a list.
  - `clear(c)` fills the whole picture with one colour.
  - `setPixel(x, y, c)` replaces one pixel. If the position is outside the picture, it does nothing, so drawing partly off-screen is safe.
  - `getPixel(x, y)` reads one pixel.
  - `blendPixel(x, y, c, alpha)` mixes a colour with what is already there. The formula is `result = new colour x alpha + old colour x (1 - alpha)`. With `alpha = 0.25` you get 25% new colour and 75% old. This is **alpha blending**.
  - `addPixel(x, y, c, strength)` adds colour on top: `result = old + new x strength`, capped at 255. This is **additive blending** and is how light brightens a surface.
  - `saveBMP(path)` writes the picture as a 24-bit BMP file. A BMP starts with a 54-byte header (size, width, height), followed by the pixel rows. BMP stores the **bottom row first** and the colours in **blue, green, red** order, and each row is padded to a multiple of 4 bytes. The code handles all three details.

---

### `include/lumina/line.h` and `src/line.cpp`: Bresenham's line algorithm

**Purpose:** Draw a straight line between two points using only whole numbers.

**The idea:** A pixel grid cannot show a perfect diagonal, so we need to decide at each step whether to move sideways, up or down. Bresenham keeps one running number, `err`, that measures how far the pixels have drifted from the true line. When the drift grows past a limit, the algorithm steps in the other direction and corrects `err`.

`drawLine(fb, x0, y0, x1, y1, color)`:
- `dx` and `dy` are the distances in each direction. `sx` and `sy` are `+1` or `-1`, so the same code works for every slope and direction.
- In a loop: colour the current pixel, stop if we reached the end, otherwise step in x and/or y depending on `err`.

`drawThickLine(...)`: draws several parallel copies of the line, shifted sideways, to make a thicker line (used for shafts and the belt).

**Used for:** belts, shafts, outlines, gear spokes, bulb filament, button borders.

---

### `include/lumina/circle.h` and `src/circle.cpp`: the midpoint circle algorithm

**Purpose:** Draw circles using only whole numbers.

**The idea:** A circle is symmetric, so we only calculate **one eighth** of it (an *octant*) and mirror each point into the other seven. We start at the top `(0, r)` and move right. At each step we ask: *is the point halfway between the two candidate pixels inside or outside the circle?* A variable `d` (the decision variable) answers this. If `d < 0`, we stay on the same row; otherwise we move one row down.

Functions:
- `drawCircle`: draws only the outline, mirroring each octant point eight ways.
- `fillCircle`: draws a solid disc. First `rowHalfWidths(r)` finds, for each row, how far the circle extends left and right. Then each row is filled with a horizontal line of pixels.
- `fillCircleBlend`: the same disc, but blended with alpha. Each row is blended **once** (blending twice would double the effect).

**Used for:** pulleys, gear hubs, the bulb, and the rings that make up the glow.

---

### `include/lumina/polygon.h` and `src/polygon.cpp`: filling shapes

**Purpose:** Fill any flat shape given by its corner points.

**The idea (scanline fill):** Go down the picture one row at a time. For each row:
1. Find where the row crosses each edge of the shape.
2. Sort the crossing positions from left to right.
3. Fill between the 1st and 2nd crossing, then between the 3rd and 4th, and so on. This is the **even-odd rule**, and it works for shapes with notches (like a gear).

Details: the row is checked at its **centre** (`y + 0.5`) so a corner is never counted twice, and flat edges are skipped.

Functions:
- `fillPolygon`: the scanline fill.
- `drawPolygon`: outlines a shape with Bresenham lines.
- `fillRect` / `drawRect`: rectangles (a rectangle is just a four-corner polygon).
- `struct Point`: a corner position `(x, y)`, as decimals.

**Used for:** gear bodies, motor body, bulb base, toolbar, buttons, text pixels.

---

### `include/lumina/picking.h`: click detection

**Purpose:** Decide whether the mouse is on a round part.

`pointInCircle(px, py, cx, cy, radius)` checks `dx*dx + dy*dy <= radius*radius`, where `dx` and `dy` are the distances from the mouse to the circle centre. This compares **squared** distances, so no slow square root is needed. In CP2 this will decide which gear to drag.

---

### `include/lumina/components.h` and `src/components.cpp`: the four parts

**Purpose:** Draw the Motor, Gear, Pulley and Lightbulb, and describe each part with data.

**Data:**
- `enum class ComponentType`: Motor, Gear, Pulley, Lightbulb.
- `namespace palette`: the shared colours (so the whole project looks consistent).
- `struct Component`: one part. It stores `type`, centre `x, y`, `radius` (also used for clicking), `teeth` (gears), `angle` (rotation in radians; CP2 will animate this) and `lit` (bulb on or off).
- `makeMotor`, `makeGear`, `makePulley`, `makeBulb`: shortcuts that fill in a `Component`.

**How each part is drawn:**
- **Gear (`drawGear`):** builds one big polygon. For every tooth it adds four corners (root-left, tip-left, tip-right, root-right) at the right angles using `cos` and `sin`. Teeth are wider at the root and narrower at the tip. The polygon is filled, outlined, and then a hub circle, three spokes and an axle hole are drawn on top. Because corners are calculated from `angle`, rotating a gear in CP2 only means changing `angle`.
- **Pulley (`drawPulley`):** several circles inside each other (rim, dark groove, lighter face) plus one marker spoke so rotation will be visible.
- **Motor (`drawMotor`):** rectangles for the body, rear cap, front plate and cooling fins, plus a small circle for the shaft. The **centre of the motor is its shaft**.
- **Bulb (`drawBulb`):** a trapezoid screw base with thread lines, a circle of glass (yellow when lit, grey when off), a filament made of three lines, and a small shine dot.
- `drawComponent`: chooses the right drawing function for any `Component`.

**Gear sizing and meshing:**
- All gears share one **module** (`kGearModule = 6`), meaning the same tooth size. A gear with `n` teeth has pitch radius `6 x n / 2`.
- Two gears mesh when their centres are `pitchRadius(a) + pitchRadius(b)` apart.
- `meshAngle(...)` turns the second gear by half a tooth so that a tooth of one gear enters a gap of the other. A test checks that the gears do not overlap.

---

### `include/lumina/belt.h` and `src/belt.cpp`: the belt

**Purpose:** Draw a belt around two circles.

`drawBelt` draws the two **outer tangent lines**, the lines that touch both circles from outside. For circle radii `r1` and `r2` and centre distance `d`, the touching points lie at angle `phi +/- acos((r1 - r2) / d)` from the line joining the centres, where `phi` is the direction from circle 1 to circle 2. The belt is drawn **before** the pulleys so it looks wrapped around them.

---

### `include/lumina/effects.h` and `src/effects.cpp`: glow and darkness

**Purpose:** The signature Lumina Gear look.

- `drawGlow(fb, cx, cy, maxRadius, color, intensity)`: draws 16 circles, from big to small, each **semi-transparent**. Where they overlap near the middle, the colours build up, so the glow is soft at the edge and strong in the centre. This is the **alpha blending** technique.
- `applyDarkness(fb, x, y, w, h, darkness, lights, lightColor)`: the **raster operation**. For every pixel in the canvas:
  1. Work out how much light reaches it from each `Light`: `(1 - distance / radius)^2`, which fades smoothly to zero at the edge.
  2. Blend black over the pixel, **less** where there is more light (`darkness x (1 - light)`).
  3. Add warm colour (additive blend) where there is light.

  The effect: the room is dark, and the bulb's light seems to cut a bright hole through it.
- `struct Light`: position, reach (`radius`) and `intensity` (0 = off, 1 = full). In CP2 the intensity will come from the power that reaches the bulb.

---

### `include/lumina/ui.h` and `src/ui.cpp`: toolbar, buttons and text

**Purpose:** The interface around the canvas.

- **Layout numbers** (`kTopBarHeight = 48`, `kToolbarWidth = 90`, `kToolSlotSize = 80`) are shared so that drawing and click detection agree.
- `Button` holds a position, size and label. `playButton()`, `pauseButton()`, `resetButton()` give the three top-bar buttons.
- `pointInRect` and `hitButton`: is the mouse inside a rectangle or button?
- `toolbarSlotAt(x, y)`: which toolbar icon (0 = Motor, 1 = Gear, 2 = Pulley, 3 = Bulb) is under the mouse, or `-1`.
- **Text:** we have no font library, so letters are drawn from a **5x7 grid of 1s and 0s** (`kGlyphs`). `drawText` colours a small square for every `1`. Only the letters needed (A, E, L, P, R, S, T, U, Y) exist so far. To add a letter, add a new line in `kGlyphs`.
- `drawButton`, `drawTopBar`, `drawToolbar`: draw the interface using rectangles, lines and the real component drawing functions (small versions as icons).

---

### `include/lumina/scene.h` and `src/scene.cpp`: putting it together

**Purpose:** Build the full picture in layers. `renderScene(fb, powered)` runs these steps:

1. **Background:** fill with dark slate grey and draw a faint dot grid.
2. **Layout:** create the parts and decide where they go. The gears are placed with the touching distance and the second gear uses `meshAngle`.
3. **Layer 1 (behind the parts):** the shafts (motor to gear, gear to pulley), the belt, and the wire from the small wheel to the bulb.
4. **Layer 2 (the parts):** motor, gears, pulleys.
5. **Layer 3 (light):** if `powered`, the glow is drawn. Then the bulb itself.
6. **Darkness overlay:** applied to the canvas only. If powered, the bulb is added as a light so it pierces the darkness, and the overall darkness is lower.
7. **Layer 4 (interface):** toolbar and top bar are drawn **last**, so the darkness never covers them.

The order matters. Anything drawn later appears on top of what was drawn before.

---

### `src/main.cpp`: the starting point

The program starts here. It renders the scene twice (once with `powered = true`, once with `false`), saves each as a `.bmp` file, prints a message, and exits. It returns `1` (error) if a file cannot be saved.

---

### `tests/test_algorithms.cpp`: automatic checks

A single program that checks our code. `CHECK(condition)` prints an error if the condition is false. The tests include:

| Test | What it checks |
|---|---|
| `testFramebuffer` | out-of-range pixels are ignored, blending maths, colours cap at 255, BMP saves |
| `testLine` / `testLineAccuracy` | line endpoints, pixel counts, and for 300 random lines that every pixel is within 0.71 pixel of the ideal line |
| `testCircleAgainstReference` | for circle radii 1 to 60, our midpoint circle (outline and filled) matches an independent calculation exactly |
| `testCircle` | rough shape, area range, radius 0, off-screen safety, blending only once |
| `testPolygon` | rectangle count, triangle area, notched shapes, U shape (4 crossings per row), corners on a pixel centre |
| `testPicking` | inside, on the edge and just outside a circle |
| `testUiHits` | button and toolbar hit areas |
| `testGearsMesh` | two meshed gears barely overlap |
| `testEffects` | darkness, light piercing, glow centre brighter than edge |
| `testBelt` | belt lines appear on both sides, and none through the middle |

To add a test, write a new `static void testSomething()` and call it from `main()`.

---

## Part 5: What Happens When You Press Run

1. `main()` creates a 1000 x 600 `Framebuffer`.
2. `renderScene()` draws the layers (Part 4, `scene.cpp`), each using the lower files: shapes become gears, gears become a machine, and the effects add light.
3. `saveBMP()` writes the pixel grid to a file.
4. The program repeats steps 1 to 3 for the unpowered version and exits.

---

## Part 6: How CP1 Connects to the Next Checkpoints

| Next step | What to build on |
|---|---|
| **CP2: movement** | Change `Component::angle` every frame. The drawing code already uses it. |
| **CP2: dragging** | Use `pointInCircle` to find the part under the mouse, then change its `x` and `y`. |
| **CP2: gear ratio** | Output speed = input speed x (input teeth / output teeth). |
| **CP2: brightness** | Feed the power reaching the bulb into `Light::intensity` and `darkness`. |
| **CP3: OpenGL** | Keep these files as a reference. Replace `Framebuffer` drawing with OpenGL, but the component data (`Component`) and maths can stay. |
