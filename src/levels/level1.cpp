#include "lumina/levels/level1.h"
#include "lumina/simulation.h"
#include "lumina/scene.h"

namespace lumina {

SimulationMode Level1::mode() const {
    return SimulationMode::ClassicLumina;
}

void Level1::setup(Simulation& sim) {
    sim.initClassicScene();
}

void Level1::solve(Simulation& sim) {
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

bool Level1::isCompleted(const Simulation& sim) const {
    const auto& parts = sim.parts();
    if (parts.size() < kPartCount) return false;
    return parts[kPartMotor].placed && parts[kPartGear1].placed &&
           parts[kPartGear2].placed && parts[kPartPulley].placed &&
           parts[kPartWheel].placed && parts[kPartBulb].placed &&
           sim.isPowered() && !sim.isReversedPolarity();
}

const char* Level1::failureReason(const Simulation& sim) const {
    const auto& parts = sim.parts();
    if (parts.size() < kPartCount) return "Workbench incomplete";
    if (!parts[kPartGear1].placed || !parts[kPartGear2].placed) {
        return "GEARS MISSING: Drag 12T and 18T gears between motor and pulley.";
    }
    if (!parts[kPartPulley].placed || !parts[kPartWheel].placed) {
        return "BELT DRIVE MISSING: Mount pulleys to link gear shaft to generator.";
    }
    if (!parts[kPartBulb].placed) {
        return "LOAD MISSING: Place the target bulb above generator wheel.";
    }
    if (!sim.isPowered()) {
        return "POWER DISCONNECTED: Align gears into mesh and connect belt drive.";
    }
    return "";
}

}  // namespace lumina

