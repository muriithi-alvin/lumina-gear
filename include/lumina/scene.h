// scene.h - the Lumina scene renderer.
// Supports both the classic CP1/CP2 workbench and the Engine Valvetrain assembly.
#pragma once
#include <vector>

#include "lumina/components.h"
#include "lumina/framebuffer.h"

namespace lumina {

constexpr int kSceneWidth = 1000;
constexpr int kSceneHeight = 600;

// The parts of the classic machine, used as indexes into sceneParts().
enum ScenePart { kPartMotor, kPartGear1, kPartGear2, kPartPulley, kPartWheel, kPartBulb, kPartCount };

// What the interactive window shows on top of the plain scene.
struct SceneView {
    bool powered = false;
    int hoveredButton = -1;   // 0 = Play, 1 = Pause, 2 = Reset, 3 = Mode, -1 = none
    int hoveredSlot = -1;     // toolbar slot under the mouse, -1 = none
    int selectedPart = -1;    // a ScenePart or EnginePartIndex, -1 = none
    float powerLevel = -1.0f; // -1 = default; 0.0 to 1.0 when animated
    bool isMeshed = false;    // visual feedback for gear mesh or belt tension

    // Engine Valvetrain specific state
    bool isEngineMode = false;
    float crankAngle = 0.0f;
    float camAngle = 0.0f;
    float intakeLift = 0.0f;
    float exhaustLift = 0.0f;
    float pistonY = 275.0f;
    float rodAngle = 0.0f;
    int stroke = 1;
    bool combustion = false;
    bool beltTensioned = true;
    bool collision = false;
    float rpm = 0.0f;
    float hp = 0.0f;
};

// The machine's parts in their scene positions, indexed by ScenePart.
std::vector<Component> sceneParts(bool powered);

int scenePartAt(int px, int py);
int scenePartAt(const std::vector<Component>& parts, int px, int py);

void renderScene(Framebuffer& fb, bool powered);
void renderScene(Framebuffer& fb, const SceneView& view);
void renderScene(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view);

void renderCanvas(Framebuffer& fb, const SceneView& view);
void renderCanvas(Framebuffer& fb, const std::vector<Component>& parts, const SceneView& view);
void renderInterface(Framebuffer& fb, const SceneView& view);

}  // namespace lumina
