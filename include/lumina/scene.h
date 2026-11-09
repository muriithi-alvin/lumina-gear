// scene.h - the Lumina scene renderer.
// Supports both the classic CP1/CP2 workbench and the Engine-to-Electricity Assembly Game.
#pragma once
#include <string>
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
    int hoveredButton = -1;   // 0 = Play, 1 = Pause, 2 = Reset, 3 = Clear, 4 = Assemble, 5 = Mode
    int hoveredSlot = -1;     // toolbar slot under mouse (0..5)
    int selectedPart = -1;    // part index being dragged/inspected
    float powerLevel = -1.0f;
    bool isMeshed = false;

    // Engine & Assembly specific state
    bool isEngineMode = false;
    bool showBlueprints = true;
    int mouseX = 0;
    int mouseY = 0;

    float crankAngle = 0.0f;
    float camAngle = 0.0f;
    float alternatorAngle = 0.0f;
    float intakeLift = 0.0f;
    float exhaustLift = 0.0f;
    float pistonY = 275.0f;
    float rodAngle = 0.0f;
    int stroke = 1;
    bool combustion = false;
    bool beltTensioned = true;
    bool alternatorDriven = true;
    bool wiringConnected = true;
    bool collision = false;
    bool assemblyComplete = true;

    float rpm = 0.0f;
    float alternatorRpm = 0.0f;
    float voltage = 0.0f;
    float hp = 0.0f;
    std::string objective;
};

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
