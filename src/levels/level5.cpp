#include "lumina/levels/level5.h"
#include "lumina/simulation.h"
#include "lumina/scene.h"

namespace lumina {

SimulationMode Level5::mode() const {
    return SimulationMode::EngineValvetrain;
}

void Level5::setup(Simulation& sim) {
    sim.initMasterPowerPlantSetup();
}

void Level5::solve(Simulation& sim) {
    sim.initMasterPowerPlantSetup();
    auto& parts = sim.parts();
    parts[kEngineCam].teeth = 24;
    parts[kEngineCam].radius = static_cast<int>(gearPitchRadius(24));
    parts[kEngineCam].placed = true;
    parts[kEngineCam].x = 320.0f;
    parts[kEngineCam].y = 200.0f;

    parts[kEngineTensioner].placed = true;
    parts[kEngineTensioner].x = 420.0f;
    parts[kEngineTensioner].y = 380.0f;

    parts[kEngineAlternator].placed = true;
    parts[kEngineAlternator].x = 520.0f;
    parts[kEngineAlternator].y = 560.0f;

    parts[kEngineBulb].placed = true;
    parts[kEngineBulb].x = 1080.0f;
    parts[kEngineBulb].y = 500.0f;

    sim.setInterferenceCollision(false);
}

bool Level5::isCompleted(const Simulation& sim) const {
    const auto& parts = sim.parts();
    if (parts.size() <= kEngineBulb) return false;
    return parts[kEngineCam].placed && (parts[kEngineCam].teeth == 24) &&
           parts[kEngineTensioner].placed && parts[kEngineAlternator].placed &&
           parts[kEngineBulb].placed && sim.isBeltTensioned() &&
           !sim.isInterferenceCollision() && sim.isPowered();
}

const char* Level5::failureReason(const Simulation& sim) const {
    if (sim.isInterferenceCollision()) {
        return "FATAL CRASH: Valvetrain timing collision! Check cam gear.";
    }
    const auto& parts = sim.parts();
    if (parts.size() <= kEngineBulb) return "Incomplete plant";
    if (!parts[kEngineAlternator].placed) {
        return "DYNAMO MISSING: Drag 12V Alternator to lower engine deck.";
    }
    if (!parts[kEngineBulb].placed) {
        return "100W LAMP MISSING: Mount illumination lamp to power workshop.";
    }
    if (!sim.isBeltTensioned()) {
        return "BELT SLACK: Cam timing belt slipping.";
    }
    if (!sim.isPowered()) {
        return "CIRCUIT INCOMPLETE: Connect alternator and lamp wiring.";
    }
    return "";
}

}  // namespace lumina

