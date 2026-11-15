#include "lumina/levels/level2.h"
#include "lumina/simulation.h"
#include "lumina/scene.h"

namespace lumina {

SimulationMode Level2::mode() const {
    return SimulationMode::ClassicLumina;
}

void Level2::setup(Simulation& sim) {
    sim.initClassicScene();
    // Start with 12T gear on motor to visibly demonstrate the low-voltage condition (7.2V < 12V)
    auto& parts = sim.parts();
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

void Level2::solve(Simulation& sim) {
    sim.initClassicScene();
    auto& parts = sim.parts();
    parts[kPartMotor].placed = true;
    // Overdrive configuration: 18T gear mounted directly on motor shaft (370, 380)
    parts[kPartGear2].placed = true;
    parts[kPartGear2].x = 370.0f;
    parts[kPartGear2].y = 380.0f;
    // 12T gear mounted on driven pulley shaft (460, 380)
    parts[kPartGear1].placed = true;
    parts[kPartGear1].x = 460.0f;
    parts[kPartGear1].y = 380.0f;
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

bool Level2::isCompleted(const Simulation& sim) const {
    return sim.isPowered() && (sim.electricalVoltage() >= 12.0f) && !sim.isReversedPolarity();
}

const char* Level2::failureReason(const Simulation& sim) const {
    if (!sim.isPowered()) {
        return "POWER DISCONNECTED: Connect drive train from motor to generator.";
    }
    if (sim.isReversedPolarity()) {
        return "POLARITY REVERSED: Check rotational direction.";
    }
    if (sim.electricalVoltage() < 12.0f) {
        return "UNDERVOLTAGE (< 12.0V DC): Swap gear order! Mount 18T gear on motor shaft and 12T on pulley for overdrive.";
    }
    return "";
}

}  // namespace lumina

