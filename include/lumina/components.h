// components.h - Mechanical and Valvetrain components.
#pragma once
#include "lumina/framebuffer.h"

namespace lumina {

enum class ComponentType { Motor, Gear, Pulley, Lightbulb, CamLobe, Valve, Piston };

// Colour palette shared by the whole project.
namespace palette {
constexpr Color background{47, 62, 70};      // dark slate grey
constexpr Color metalLight{176, 184, 190};
constexpr Color metalMid{122, 133, 141};
constexpr Color metalDark{74, 84, 91};
constexpr Color outline{22, 28, 32};
constexpr Color motorBody{58, 98, 138};
constexpr Color motorCap{38, 66, 94};
constexpr Color bulbOff{128, 142, 150};
constexpr Color bulbOn{255, 232, 120};
constexpr Color glow{255, 214, 90};
constexpr Color belt{128, 112, 96};
constexpr Color bronze{180, 120, 60};
constexpr Color combustionFire{255, 160, 30};
constexpr Color intakeAir{70, 160, 230};
constexpr Color exhaustGas{240, 90, 40};
}  // namespace palette

struct Component {
    ComponentType type = ComponentType::Gear;
    float x = 0, y = 0;   // centre
    int radius = 30;      // size of the part; also used for mouse picking
    int teeth = 12;       // gears only
    float angle = 0;      // rotation in radians
    bool lit = false;
};

// Gear sizes follow a fixed "module" so any two gears can mesh.
constexpr float kGearModule = 6.0f;
constexpr int kToothAddendum = 3;   // tooth height above the pitch circle
constexpr int kToothDedendum = 4;   // gap depth below the pitch circle

inline float gearPitchRadius(int teeth) { return kGearModule * teeth / 2.0f; }

// Convenience constructors.
Component makeMotor(float x, float y, int radius);
Component makeGear(float x, float y, int teeth, float angle = 0);
Component makePulley(float x, float y, int radius);
Component makeBulb(float x, float y, int radius, bool lit);

// Valvetrain component constructors
Component makeCamLobe(float x, float y, int radius, float angle = 0);
Component makeValve(float x, float y, int radius);
Component makePiston(float x, float y, int radius);

float meshAngle(int teeth, float towardsAngle);

void drawMotor(Framebuffer& fb, const Component& c);
void drawGear(Framebuffer& fb, const Component& c);
void drawPulley(Framebuffer& fb, const Component& c);
void drawBulb(Framebuffer& fb, const Component& c);
void drawComponent(Framebuffer& fb, const Component& c);

// Valvetrain specific drawing functions
void drawCamLobe(Framebuffer& fb, float cx, float cy, float radius, float angle);
void drawValve(Framebuffer& fb, float cx, float topY, float lift, float stemLength, Color springColor);
void drawPistonAndCylinder(Framebuffer& fb, float cx, float cy, float width, float height,
                           float pistonY, float rodAngle, bool combustion);

}  // namespace lumina
