// level2.h - Overdrive Speed Step-Up (Target >= 12V DC)
#pragma once

#include "lumina/level.h"

namespace lumina {

class Level2 : public Level {
public:
    int levelNumber() const override { return 2; }
    const char* title() const override { return "Level 2: Overdrive Speed Step-Up (Target >= 12V DC)"; }
    const char* objective() const override {
        return "LVL 2: MOTOR IS HALF-SPEED (600 RPM). CONFIGURE OVERDRIVE (18T MOTOR -> 12T PULLEY) FOR >= 12V DC!";
    }
    SimulationMode mode() const override;
    float targetMotorSpeed() const override { return 2.0f; }

    void setup(Simulation& sim) override;
    void solve(Simulation& sim) override;
    bool isCompleted(const Simulation& sim) const override;
    const char* failureReason(const Simulation& sim) const override;
};

}  // namespace lumina

