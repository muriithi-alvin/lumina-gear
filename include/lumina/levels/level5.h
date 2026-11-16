// level5.h - Master Power Plant Grid Integration
#pragma once

#include "lumina/level.h"

namespace lumina {

class Level5 : public Level {
public:
    int levelNumber() const override { return 5; }
    const char* title() const override { return "Level 5: Master Power Plant Grid Integration"; }
    const char* objective() const override {
        return "LVL 5: MASTER POWER PLANT! ENGINE TIMING SYNCHRONIZED. ASSEMBLE DYNAMO & 100W LAMP TO POWER WORKSHOP!";
    }
    SimulationMode mode() const override;
    float targetMotorSpeed() const override { return 4.0f; }

    void setup(Simulation& sim) override;
    void solve(Simulation& sim) override;
    bool isCompleted(const Simulation& sim) const override;
    const char* failureReason(const Simulation& sim) const override;
};

}  // namespace lumina

