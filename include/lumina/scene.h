// scene.h - the Lumina scene renderer.
// Supports both the classic CP1/CP2 workbench and the 5-Level Engineering Physics Campaign.
#pragma once
#include <string>
#include <vector>

#include "lumina/components.h"
#include "lumina/framebuffer.h"

namespace lumina {

// 1360 x 760 fills 78% of a 1536 x 864 screen, providing ample building room!
constexpr int kSceneWidth = 1360;
constexpr int kSceneHeight = 760;

// The parts of the classic machine, used as indexes into sceneParts().
enum ScenePart {
    kPartMotor = 0,
    kPartGear1 = 1,
    kPartGear2 = 2,
    kPartPulley = 3,
    kPartWheel = 4,
    kPartBulb = 5,
    kPartIdler = 6,
    kClassicTotalParts = 7,
    kPartCount = 6 // Baseline count for CP1/CP2 tests!
};

// What the interactive window shows on top of the plain scene.
struct SceneView {
    bool powered = false;
    int hoveredButton = -1;
    int hoveredSlot = -1;     // toolbar slot under mouse (0..6)
    int selectedPart = -1;    // part index being dragged/inspected
    float powerLevel = -1.0f;
    bool isMeshed = false;
    bool crossedBelt = false; // Level 3 Direction-Reversing Crossed Belt

    // Game Campaign State
    int currentLevel = 1;     // 1 to 5
    bool levelCompleted = false;
    bool isEngineMode = false;
    int mouseX = 0;
    int mouseY = 0;

    // Telemetry & Physics State
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
    bool reversedPolarity = false;

    float rpm = 0.0f;
    float alternatorRpm = 0.0f;
    float voltage = 0.0f;
    float hp = 0.0f;
    std::string objective;
    std::string failureReason;

    int tipCardX = -1;
    int tipCardY = -1;
    bool tipCardMinimized = false;
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

void getTipCardBounds(const std::vector<Component>& parts, const SceneView& view, int& x, int& y, int& w, int& h);

}  // namespace lumina
