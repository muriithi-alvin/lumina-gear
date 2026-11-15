// level1.h - Baseline Drive Train (Classic CP1/CP2)
#pragma once

#include "lumina/level.h"

namespace lumina {

class Level1 : public Level {
public:
    int levelNumber() const override { return 1; }
    const char* title() const override { return "Level 1: Baseline Drive Train (Target 12V DC)"; }
    const char* objective() const override {
        return "LVL 1: ASSEMBLE MOTOR, GEARS, BELT PULLEYS & BULB TO POWER THE GRID!";
    }
    SimulationMode mode() const override;

    void setup(Simulation& sim) override;
    void solve(Simulation& sim) override;
    bool isCompleted(const Simulation& sim) const override;
    const char* failureReason(const Simulation& sim) const override;
};

}  // namespace lumina

