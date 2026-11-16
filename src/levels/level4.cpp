#include "lumina/levels/level4.h"
#include "lumina/simulation.h"
#include "lumina/scene.h"

namespace lumina {

SimulationMode Level4::mode() const {
    return SimulationMode::EngineValvetrain;
}

void Level4::setup(Simulation& sim) {
    sim.initEngineTimingBench();
}

void Level4::solve(Simulation& sim) {
    sim.initEngineTimingBench();
    auto& parts = sim.parts();
    parts[kEngineCam].teeth = 24;
    parts[kEngineCam].radius = static_cast<int>(gearPitchRadius(24));
    parts[kEngineCam].placed = true;
    parts[kEngineCam].x = 320.0f;
    parts[kEngineCam].y = 200.0f;

    parts[kEngineTensioner].placed = true;
    parts[kEngineTensioner].x = 420.0f;
    parts[kEngineTensioner].y = 380.0f;

    sim.setInterferenceCollision(false);
}

bool Level4::isCompleted(const Simulation& sim) const {
    const auto& parts = sim.parts();
    if (parts.size() <= kEngineTensioner) return false;
    return parts[kEngineCam].placed && (parts[kEngineCam].teeth == 24) &&
           parts[kEngineTensioner].placed && sim.isBeltTensioned() &&
           !sim.isInterferenceCollision() && (sim.consecutiveCleanCycles() >= 2 || (sim.engineRpm() > 500.0f && !sim.isInterferenceCollision()));
}

const char* Level4::failureReason(const Simulation& sim) const {
    if (sim.isInterferenceCollision()) {
        return "FATAL CRASH: Piston struck open valve at TDC! Cam gear was 12T (1:1). Mount 24T Cam Sprocket!";
    }
    const auto& parts = sim.parts();
    if (parts.size() > kEngineCam && parts[kEngineCam].teeth != 24) {
        return "CRITICAL MISMATCH: Camshaft must turn at 1:2 speed. Swap 12T gear for 24T Cam Sprocket.";
    }
    if (!sim.isBeltTensioned()) {
        return "BELT SLACK: Camshaft slipping! Tighten tensioner pulley.";
    }
    return "";
}

}  // namespace lumina

