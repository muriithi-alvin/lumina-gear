// level3.h - Rotational Polarity & Idler Gear Reversal
#pragma once

#include "lumina/level.h"

namespace lumina {

class Level3 : public Level {
public:
    int levelNumber() const override { return 3; }
    const char* title() const override { return "Level 3: Rotational Polarity & Idler Reversal"; }
    const char* objective() const override {
        return "LVL 3: MOTOR SPINS CCW (REVERSED)! INSERT 14T IDLER GEAR OR PRESS 'B' FOR CROSSED BELT!";
    }
    SimulationMode mode() const override;
    float targetMotorSpeed() const override { return -4.0f; }

    void setup(Simulation& sim) override;
    void solve(Simulation& sim) override;
    bool isCompleted(const Simulation& sim) const override;
    const char* failureReason(const Simulation& sim) const override;
};

}  // namespace lumina

