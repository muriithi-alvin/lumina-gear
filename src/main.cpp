// Lumina Gear - CP1: Technical Foundation
// Draws the static scene with our own algorithms and saves it as images.
#include <iostream>

#include "lumina/framebuffer.h"
#include "lumina/scene.h"

int main() {
    using namespace lumina;
    bool ok = true;
    for (bool powered : {true, false}) {
        Framebuffer fb(kSceneWidth, kSceneHeight);
        renderScene(fb, powered);
        const char* name = powered ? "lumina_gear_cp1_powered.bmp" : "lumina_gear_cp1_unpowered.bmp";
        if (fb.saveBMP(name)) std::cout << "Saved " << name << "\n";
        else { std::cerr << "Could not write " << name << "\n"; ok = false; }
    }
    return ok ? 0 : 1;
}
