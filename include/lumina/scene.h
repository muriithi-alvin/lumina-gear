// scene.h - the CP1 demo scene that puts every component together.
#pragma once
#include <vector>

#include "lumina/components.h"
#include "lumina/framebuffer.h"

namespace lumina {

constexpr int kSceneWidth = 1000;
constexpr int kSceneHeight = 600;

// The parts of the machine, used as indexes into sceneParts().
enum ScenePart { kPartMotor, kPartGear1, kPartGear2, kPartPulley, kPartWheel, kPartBulb, kPartCount };

// What the interactive window shows on top of the plain scene.
// The defaults draw exactly the same picture as renderScene(fb, powered).
struct SceneView {
    bool powered = false;
    int hoveredButton = -1;   // 0 = Play, 1 = Pause, 2 = Reset, -1 = none
    int hoveredSlot = -1;     // toolbar slot under the mouse, -1 = none
    int selectedPart = -1;    // a ScenePart, -1 = none
};

// The machine's parts in their scene positions, indexed by ScenePart.
std::vector<Component> sceneParts(bool powered);

// Which part is under (px, py)? Checks the top-most part first and returns
// a ScenePart, or -1 when the point is not on a part.
int scenePartAt(int px, int py);

// powered = true: the bulb is lit and the room is brighter.
void renderScene(Framebuffer& fb, bool powered);
void renderScene(Framebuffer& fb, const SceneView& view);

// renderScene() is these two steps. The canvas is slow to draw and only
// depends on power and the selection; the interface is quick and covers its
// own area completely. So the window keeps the canvas and redraws only the
// interface while the mouse hovers.
void renderCanvas(Framebuffer& fb, const SceneView& view);
void renderInterface(Framebuffer& fb, const SceneView& view);

}  // namespace lumina
