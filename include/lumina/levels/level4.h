// level4.h - 4-Stroke Valvetrain Timing Bench (Avoid Crash!)
#pragma once

#include "lumina/level.h"

namespace lumina {

class Level4 : public Level {
public:
    int levelNumber() const override { return 4; }
    const char* title() const override { return "Level 4: 4-Stroke Valvetrain Timing (Avoid Crash!)"; }
    const char* objective() const override {
        return "LVL 4: VALVETRAIN TIMING BENCH! CAM GEAR HAS 12T (1:1 CRASH). MOUNT 24T CAM GEAR FOR 2:1 SYNCHRONIZED TIMING!";
    }
    SimulationMode mode() const override;
    float targetMotorSpeed() const override { return 4.0f; }

    void setup(Simulation& sim) override;
    void solve(Simulation& sim) override;
    bool isCompleted(const Simulation& sim) const override;
    const char* failureReason(const Simulation& sim) const override;
};

}  // namespace lumina

