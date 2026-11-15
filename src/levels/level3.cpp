#include "lumina/levels/level3.h"
#include "lumina/simulation.h"
#include "lumina/scene.h"

namespace lumina {

SimulationMode Level3::mode() const {
    return SimulationMode::ClassicLumina;
}

void Level3::setup(Simulation& sim) {
    sim.initClassicScene();
    auto& parts = sim.parts();
    parts[kPartMotor].placed = true;
    parts[kPartGear1].placed = true;
    parts[kPartGear1].x = 370.0f;
    parts[kPartGear1].y = 380.0f;
    parts[kPartGear2].placed = true;
    parts[kPartGear2].x = 460.0f;
    parts[kPartGear2].y = 380.0f;
    parts[kPartPulley].placed = true;
    parts[kPartPulley].x = 640.0f;
    parts[kPartPulley].y = 380.0f;
    parts[kPartWheel].placed = true;
    parts[kPartWheel].x = 1080.0f;
    parts[kPartWheel].y = 520.0f;
    parts[kPartBulb].placed = true;
    parts[kPartBulb].x = 1080.0f;
    parts[kPartBulb].y = 290.0f;
    if (parts.size() > kPartIdler) {
        parts[kPartIdler].placed = false;
    }
    sim.setCrossedBelt(false);
}

void Level3::solve(Simulation& sim) {
    sim.initClassicScene();
    auto& parts = sim.parts();
    parts[kPartMotor].placed = true;
    // Driver gear on motor shaft
    parts[kPartGear1].placed = true;
    parts[kPartGear1].x = 370.0f;
    parts[kPartGear1].y = 380.0f;

    // Insert 14T Idler Gear (r=42) in the middle to invert direction: 370 + 36 + 42 = 448
    if (parts.size() > kPartIdler) {
        parts[kPartIdler].placed = true;
        parts[kPartIdler].x = 448.0f;
        parts[kPartIdler].y = 380.0f;
    }

    // Driven 18T gear (r=54): 448 + 42 + 54 = 544
    parts[kPartGear2].placed = true;
    parts[kPartGear2].x = 544.0f;
    parts[kPartGear2].y = 380.0f;

    parts[kPartPulley].placed = true;
    parts[kPartPulley].x = 720.0f;
    parts[kPartPulley].y = 380.0f;

    parts[kPartWheel].placed = true;
    parts[kPartWheel].x = 1080.0f;
    parts[kPartWheel].y = 520.0f;

    parts[kPartBulb].placed = true;
    parts[kPartBulb].x = 1080.0f;
    parts[kPartBulb].y = 290.0f;

    sim.setCrossedBelt(false);
}

bool Level3::isCompleted(const Simulation& sim) const {
    return sim.isPowered() && !sim.isReversedPolarity();
}

const char* Level3::failureReason(const Simulation& sim) const {
    if (!sim.isPowered()) {
        return "POWER DISCONNECTED: Connect drive train from motor to generator.";
    }
    if (sim.isReversedPolarity()) {
        return "POLARITY REVERSED: Dynamo spinning counter-clockwise! Insert 14T Idler Gear or press 'B' to cross belt.";
    }
    return "";
}

}  // namespace lumina

