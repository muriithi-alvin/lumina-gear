#include "lumina/simulation.h"

#include <algorithm>
#include <cmath>
#include "lumina/scene.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
}  // namespace

Simulation::Simulation() {
    initDefaultScene();
}

void Simulation::initDefaultScene() {
    // Generate default components matching CP1
    parts_ = sceneParts(false);
    speeds_.assign(parts_.size(), 0.0f);
    connections_.clear();

    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    playing_ = false;

    autoDetectConnections();
    solveKinematics();
}

void Simulation::play() {
    playing_ = true;
}

void Simulation::pause() {
    playing_ = false;
}

void Simulation::togglePlay() {
    playing_ = !playing_;
}

void Simulation::reset() {
    playing_ = false;
    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    initDefaultScene();
}

void Simulation::setPartPosition(int index, float x, float y) {
    if (index >= 0 && index < static_cast<int>(parts_.size())) {
        parts_[index].x = x;
        parts_[index].y = y;
        autoDetectConnections();
        solveKinematics();
    }
}

bool Simulation::isPartMeshed(int partIndex) const {
    for (const auto& c : connections_) {
        if (c.type == ConnectionType::GearMesh &&
            (c.sourceIndex == partIndex || c.targetIndex == partIndex)) {
            return true;
        }
    }
    return false;
}

void Simulation::snapToMesh(int partIndex) {
    if (partIndex < 0 || partIndex >= static_cast<int>(parts_.size())) return;
    if (parts_[partIndex].type != ComponentType::Gear) return;

    for (size_t j = 0; j < parts_.size(); ++j) {
        if (static_cast<int>(j) == partIndex) continue;
        if (parts_[j].type != ComponentType::Gear) continue;

        const float dx = parts_[partIndex].x - parts_[j].x;
        const float dy = parts_[partIndex].y - parts_[j].y;
        const float dist = std::hypot(dx, dy);
        const float expectedDist = gearPitchRadius(parts_[partIndex].teeth) + gearPitchRadius(parts_[j].teeth);

        // Snap window within +- 18 pixels
        if (dist > 1.0f && std::abs(dist - expectedDist) <= 18.0f) {
            const float angle = std::atan2(dy, dx);
            parts_[partIndex].x = parts_[j].x + expectedDist * std::cos(angle);
            parts_[partIndex].y = parts_[j].y + expectedDist * std::sin(angle);
            parts_[partIndex].angle = meshAngle(parts_[partIndex].teeth, angle + kPi);
            autoDetectConnections();
            solveKinematics();
            break;
        }
    }
}

void Simulation::addConnection(int src, int dst, ConnectionType type) {
    connections_.push_back({src, dst, type});
}

void Simulation::clearConnections() {
    connections_.clear();
}

void Simulation::autoDetectConnections() {
    clearConnections();
    if (parts_.size() < kPartCount) return;

    const Component& motor = parts_[kPartMotor];
    const Component& gear1 = parts_[kPartGear1];
    const Component& gear2 = parts_[kPartGear2];
    const Component& pulley = parts_[kPartPulley];
    const Component& wheel = parts_[kPartWheel];
    const Component& bulb = parts_[kPartBulb];

    // 1. Motor -> Gear 1 (Drive Shaft)
    // Motor shaft connects to Gear 1 if within reach
    const float motorShaftDist = std::hypot(motor.x - gear1.x, motor.y - gear1.y);
    if (motorShaftDist <= 240.0f) {
        addConnection(kPartMotor, kPartGear1, ConnectionType::Shaft);
    }

    // 2. Gear 1 -> Gear 2 (Teeth Meshing)
    // Meshing requires distance within tolerance of pitch radii sum (36 + 54 = 90 px)
    const float gearDist = std::hypot(gear1.x - gear2.x, gear1.y - gear2.y);
    const float expectedPitchSum = gearPitchRadius(gear1.teeth) + gearPitchRadius(gear2.teeth);
    if (std::abs(gearDist - expectedPitchSum) <= 12.0f) {
        addConnection(kPartGear1, kPartGear2, ConnectionType::GearMesh);
    }

    // 3. Gear 2 -> Large Pulley (Connecting Axle Shaft)
    const float pulleyShaftDist = std::hypot(gear2.x - pulley.x, gear2.y - pulley.y);
    if (pulleyShaftDist <= 280.0f) {
        addConnection(kPartGear2, kPartPulley, ConnectionType::Shaft);
    }

    // 4. Large Pulley -> Small Wheel (Belt Drive)
    // Must be separated and within belt reach
    const float beltDist = std::hypot(pulley.x - wheel.x, pulley.y - wheel.y);
    if (beltDist > static_cast<float>(pulley.radius + wheel.radius) && beltDist <= 550.0f) {
        addConnection(kPartPulley, kPartWheel, ConnectionType::Belt);
    }

    // 5. Small Wheel -> Lightbulb (Electrical Generator)
    const float wireDist = std::hypot(wheel.x - bulb.x, wheel.y - (bulb.y + bulb.radius * 1.3f));
    if (wireDist <= 320.0f) {
        addConnection(kPartWheel, kPartBulb, ConnectionType::Generator);
    }
}

void Simulation::solveKinematics() {
    if (speeds_.size() != parts_.size()) {
        speeds_.assign(parts_.size(), 0.0f);
    } else {
        std::fill(speeds_.begin(), speeds_.end(), 0.0f);
    }

    powerLevel_ = 0.0f;
    for (auto& p : parts_) {
        if (p.type == ComponentType::Lightbulb) {
            p.lit = false;
        }
    }

    // Set motor speed
    for (size_t i = 0; i < parts_.size(); ++i) {
        if (parts_[i].type == ComponentType::Motor) {
            speeds_[i] = currentMotorSpeed_;
        }
    }

    // Propagate speeds through connection graph
    // Multiple passes to ensure chain propagation
    bool changed = true;
    int maxPasses = static_cast<int>(parts_.size()) * 2;
    while (changed && maxPasses-- > 0) {
        changed = false;
        for (const auto& conn : connections_) {
            if (conn.sourceIndex < 0 || conn.sourceIndex >= static_cast<int>(parts_.size()) ||
                conn.targetIndex < 0 || conn.targetIndex >= static_cast<int>(parts_.size())) {
                continue;
            }

            const float srcSpeed = speeds_[conn.sourceIndex];
            float expectedDstSpeed = 0.0f;

            switch (conn.type) {
                case ConnectionType::Shaft:
                    expectedDstSpeed = srcSpeed;
                    break;

                case ConnectionType::GearMesh: {
                    const int teethA = parts_[conn.sourceIndex].teeth;
                    const int teethB = parts_[conn.targetIndex].teeth;
                    if (teethB > 0) {
                        // Meshed gears rotate in opposite directions!
                        expectedDstSpeed = -srcSpeed * (static_cast<float>(teethA) / static_cast<float>(teethB));
                    }
                    break;
                }

                case ConnectionType::Belt: {
                    const float radA = static_cast<float>(parts_[conn.sourceIndex].radius);
                    const float radB = static_cast<float>(parts_[conn.targetIndex].radius);
                    if (radB > 0.0f) {
                        // Open belt transmits rotation in the same direction
                        expectedDstSpeed = srcSpeed * (radA / radB);
                    }
                    break;
                }

                case ConnectionType::Generator: {
                    // Power generation based on speed of generator wheel
                    // Nominal speed = targetMotorSpeed * (12/18) * (40/22)
                    const float nominalWheelSpeed = std::max(0.1f, targetMotorSpeed_ * (12.0f / 18.0f) * (40.0f / 22.0f));
                    powerLevel_ = std::clamp(std::abs(srcSpeed) / nominalWheelSpeed, 0.0f, 1.0f);
                    parts_[conn.targetIndex].lit = (powerLevel_ > 0.05f);
                    break;
                }
            }

            if (conn.type != ConnectionType::Generator) {
                if (std::abs(speeds_[conn.targetIndex] - expectedDstSpeed) > 1e-4f) {
                    speeds_[conn.targetIndex] = expectedDstSpeed;
                    changed = true;
                }
            }
        }
    }
}

void Simulation::update(float dt) {
    // Clamp dt to reasonable bounds (e.g. 100ms max) to prevent physics jumps
    dt = std::clamp(dt, 0.0f, 0.1f);
    if (dt <= 0.0f) return;

    // Smooth motor acceleration / deceleration
    const float target = playing_ ? targetMotorSpeed_ : 0.0f;
    const float k = playing_ ? 4.0f : 2.5f;   // response rate (inertia)
    currentMotorSpeed_ += (target - currentMotorSpeed_) * (1.0f - std::exp(-k * dt));

    if (!playing_ && std::abs(currentMotorSpeed_) < 0.01f) {
        currentMotorSpeed_ = 0.0f;
    }

    // Solve kinematics (speed propagation and ratios)
    solveKinematics();

    // Integrate angles for each part
    for (size_t i = 0; i < parts_.size(); ++i) {
        if (std::abs(speeds_[i]) > 1e-6f) {
            parts_[i].angle += speeds_[i] * dt;
            // Wrap angle to [0, 2*pi)
            parts_[i].angle = std::fmod(parts_[i].angle, kTwoPi);
            if (parts_[i].angle < 0.0f) {
                parts_[i].angle += kTwoPi;
            }
        }
    }
}

}  // namespace lumina
