// level.h - Abstract base class and factory for progressive engineering levels
#pragma once

#include <memory>
#include <string>

namespace lumina {

class Simulation;
enum class SimulationMode;

class Level {
public:
    virtual ~Level() = default;

    virtual int levelNumber() const = 0;
    virtual const char* title() const = 0;
    virtual const char* objective() const = 0;
    virtual SimulationMode mode() const = 0;
    virtual float targetMotorSpeed() const { return 4.0f; }

    virtual void setup(Simulation& sim) = 0;
    virtual void solve(Simulation& sim) = 0;
    virtual bool isCompleted(const Simulation& sim) const = 0;
    virtual const char* failureReason(const Simulation& sim) const { (void)sim; return ""; }
};

std::unique_ptr<Level> createLevel(int levelNumber);

}  // namespace lumina

