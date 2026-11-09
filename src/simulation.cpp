#include "lumina/simulation.h"

#include <algorithm>
#include <cmath>
#include "lumina/scene.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
constexpr float kFourPi = 4.0f * kPi;
}  // namespace

Simulation::Simulation(SimulationMode mode) : mode_(mode) {
    initDefaultScene();
}

void Simulation::setMode(SimulationMode m) {
    if (mode_ != m) {
        mode_ = m;
        playing_ = false;
        currentMotorSpeed_ = 0.0f;
        powerLevel_ = 0.0f;
        initDefaultScene();
    }
}

void Simulation::initDefaultScene() {
    if (mode_ == SimulationMode::EngineValvetrain) {
        initEngineScene();
    } else {
        initClassicScene();
    }
}

void Simulation::initClassicScene() {
    parts_ = sceneParts(false);
    speeds_.assign(parts_.size(), 0.0f);
    connections_.clear();

    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    playing_ = false;

    autoDetectConnections();
    solveKinematics();
}

void Simulation::initEngineScene() {
    parts_.clear();
    parts_.resize(kEnginePartCount);

    // 1. Crankshaft drive sprocket (12 teeth)
    parts_[kEngineCrank] = makeGear(240.0f, 460.0f, 12, 0.0f);

    // 2. Camshaft driven sprocket (24 teeth) - 2:1 ratio
    parts_[kEngineCam] = makeGear(240.0f, 160.0f, 24, 0.0f);

    // 3. Timing belt tensioner / idler pulley
    parts_[kEngineTensioner] = makePulley(340.0f, 310.0f, 24);

    // 4. Intake cam lobe
    parts_[kEngineIntakeCam] = makeCamLobe(570.0f, 160.0f, 22, 0.0f);

    // 5. Exhaust cam lobe (90 deg cam phase offset = 180 deg crank)
    parts_[kEngineExhaustCam] = makeCamLobe(710.0f, 160.0f, 22, 1.5707963f);

    // 6. Intake poppet valve
    parts_[kEngineIntakeValve] = makeValve(570.0f, 195.0f, 18);

    // 7. Exhaust poppet valve
    parts_[kEngineExhaustValve] = makeValve(710.0f, 195.0f, 18);

    // 8. Piston and cylinder assembly
    parts_[kEnginePiston] = makePiston(640.0f, 260.0f, 45);

    speeds_.assign(kEnginePartCount, 0.0f);
    connections_.clear();

    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    playing_ = false;

    crankAngle_ = 0.0f;
    camAngle_ = 0.0f;
    intakeValveLift_ = 0.0f;
    exhaustValveLift_ = 0.0f;
    pistonY_ = 275.0f;
    rodAngle_ = 0.0f;
    strokeNumber_ = 1;
    combustionActive_ = false;
    beltTensioned_ = true;
    interferenceCollision_ = false;

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
    crankAngle_ = 0.0f;
    camAngle_ = 0.0f;
    intakeValveLift_ = 0.0f;
    exhaustValveLift_ = 0.0f;
    pistonY_ = 275.0f;
    rodAngle_ = 0.0f;
    strokeNumber_ = 1;
    combustionActive_ = false;
    interferenceCollision_ = false;
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
    if (mode_ == SimulationMode::EngineValvetrain) {
        return beltTensioned_;
    }
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

    if (mode_ == SimulationMode::EngineValvetrain) {
        if (partIndex == kEngineTensioner) {
            // Snap tensioner back to optimal alignment
            const float dx = parts_[kEngineTensioner].x - 340.0f;
            const float dy = parts_[kEngineTensioner].y - 310.0f;
            if (std::hypot(dx, dy) < 45.0f) {
                parts_[kEngineTensioner].x = 340.0f;
                parts_[kEngineTensioner].y = 310.0f;
                autoDetectConnections();
                solveKinematics();
            }
        }
        return;
    }

    if (parts_[partIndex].type != ComponentType::Gear) return;

    for (size_t j = 0; j < parts_.size(); ++j) {
        if (static_cast<int>(j) == partIndex) continue;
        if (parts_[j].type != ComponentType::Gear) continue;

        const float dx = parts_[partIndex].x - parts_[j].x;
        const float dy = parts_[partIndex].y - parts_[j].y;
        const float dist = std::hypot(dx, dy);
        const float expectedDist = gearPitchRadius(parts_[partIndex].teeth) + gearPitchRadius(parts_[j].teeth);

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

    if (mode_ == SimulationMode::EngineValvetrain) {
        if (parts_.size() < kEnginePartCount) return;

        const Component& tens = parts_[kEngineTensioner];
        // Tensioner must be in valid tension pocket
        beltTensioned_ = (tens.x >= 295.0f && tens.x <= 385.0f && tens.y >= 250.0f && tens.y <= 370.0f);

        const Component& crank = parts_[kEngineCrank];
        const Component& cam = parts_[kEngineCam];
        const float dCrankCam = std::hypot(crank.x - cam.x, crank.y - cam.y);
        const bool sprocketsAligned = (dCrankCam >= 270.0f && dCrankCam <= 330.0f);

        if (beltTensioned_ && sprocketsAligned) {
            addConnection(kEngineCrank, kEngineCam, ConnectionType::Belt);
            addConnection(kEngineCam, kEngineIntakeCam, ConnectionType::Shaft);
            addConnection(kEngineCam, kEngineExhaustCam, ConnectionType::Shaft);
        }
        return;
    }

    if (parts_.size() < kPartCount) return;

    const Component& motor = parts_[kPartMotor];
    const Component& gear1 = parts_[kPartGear1];
    const Component& gear2 = parts_[kPartGear2];
    const Component& pulley = parts_[kPartPulley];
    const Component& wheel = parts_[kPartWheel];
    const Component& bulb = parts_[kPartBulb];

    const float motorShaftDist = std::hypot(motor.x - gear1.x, motor.y - gear1.y);
    if (motorShaftDist <= 240.0f) {
        addConnection(kPartMotor, kPartGear1, ConnectionType::Shaft);
    }

    const float gearDist = std::hypot(gear1.x - gear2.x, gear1.y - gear2.y);
    const float expectedPitchSum = gearPitchRadius(gear1.teeth) + gearPitchRadius(gear2.teeth);
    if (std::abs(gearDist - expectedPitchSum) <= 12.0f) {
        addConnection(kPartGear1, kPartGear2, ConnectionType::GearMesh);
    }

    const float pulleyShaftDist = std::hypot(gear2.x - pulley.x, gear2.y - pulley.y);
    if (pulleyShaftDist <= 280.0f) {
        addConnection(kPartGear2, kPartPulley, ConnectionType::Shaft);
    }

    const float beltDist = std::hypot(pulley.x - wheel.x, pulley.y - wheel.y);
    if (beltDist > static_cast<float>(pulley.radius + wheel.radius) && beltDist <= 550.0f) {
        addConnection(kPartPulley, kPartWheel, ConnectionType::Belt);
    }

    const float wireDist = std::hypot(wheel.x - bulb.x, wheel.y - (bulb.y + bulb.radius * 1.3f));
    if (wireDist <= 320.0f) {
        addConnection(kPartWheel, kPartBulb, ConnectionType::Generator);
    }
}

void Simulation::solveKinematics() {
    if (mode_ == SimulationMode::EngineValvetrain) {
        solveEngineKinematics();
    } else {
        solveClassicKinematics();
    }
}

void Simulation::solveClassicKinematics() {
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

    for (size_t i = 0; i < parts_.size(); ++i) {
        if (parts_[i].type == ComponentType::Motor) {
            speeds_[i] = currentMotorSpeed_;
        }
    }

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
                        expectedDstSpeed = -srcSpeed * (static_cast<float>(teethA) / static_cast<float>(teethB));
                    }
                    break;
                }

                case ConnectionType::Belt: {
                    const float radA = static_cast<float>(parts_[conn.sourceIndex].radius);
                    const float radB = static_cast<float>(parts_[conn.targetIndex].radius);
                    if (radB > 0.0f) {
                        expectedDstSpeed = srcSpeed * (radA / radB);
                    }
                    break;
                }

                case ConnectionType::Generator: {
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

void Simulation::solveEngineKinematics() {
    if (speeds_.size() != parts_.size()) {
        speeds_.assign(parts_.size(), 0.0f);
    } else {
        std::fill(speeds_.begin(), speeds_.end(), 0.0f);
    }

    if (interferenceCollision_) {
        currentMotorSpeed_ = 0.0f;
        powerLevel_ = 0.0f;
        return;
    }

    speeds_[kEngineCrank] = currentMotorSpeed_;

    if (beltTensioned_) {
        // Crankshaft has 12 teeth, Camshaft has 24 teeth -> Exact 2:1 ratio (0.5x speed)
        speeds_[kEngineCam] = 0.5f * currentMotorSpeed_;
        speeds_[kEngineTensioner] = currentMotorSpeed_ * 1.5f;
        speeds_[kEngineIntakeCam] = speeds_[kEngineCam];
        speeds_[kEngineExhaustCam] = speeds_[kEngineCam];
        powerLevel_ = std::clamp(std::abs(currentMotorSpeed_) / targetMotorSpeed_, 0.0f, 1.0f);
    } else {
        // Belt is slack/slipped: Camshaft loses power and slips out of time!
        speeds_[kEngineCam] = 0.0f;
        speeds_[kEngineTensioner] = 0.0f;
        speeds_[kEngineIntakeCam] = 0.0f;
        speeds_[kEngineExhaustCam] = 0.0f;
        powerLevel_ = 0.0f;
    }
}

void Simulation::update(float dt) {
    dt = std::clamp(dt, 0.0f, 0.1f);
    if (dt <= 0.0f) return;

    if (mode_ == SimulationMode::EngineValvetrain && interferenceCollision_) {
        currentMotorSpeed_ = 0.0f;
        playing_ = false;
        return;
    }

    const float target = playing_ ? targetMotorSpeed_ : 0.0f;
    const float k = playing_ ? 4.0f : 2.5f;
    currentMotorSpeed_ += (target - currentMotorSpeed_) * (1.0f - std::exp(-k * dt));

    if (!playing_ && std::abs(currentMotorSpeed_) < 0.01f) {
        currentMotorSpeed_ = 0.0f;
    }

    solveKinematics();

    if (mode_ == SimulationMode::EngineValvetrain) {
        // Advance crank rotation
        crankAngle_ += speeds_[kEngineCrank] * dt;
        crankAngle_ = std::fmod(crankAngle_, kFourPi);
        if (crankAngle_ < 0.0f) crankAngle_ += kFourPi;

        if (beltTensioned_) {
            camAngle_ = 0.5f * crankAngle_;
        } else {
            camAngle_ += speeds_[kEngineCam] * dt;
            camAngle_ = std::fmod(camAngle_, kTwoPi);
            if (camAngle_ < 0.0f) camAngle_ += kTwoPi;
        }

        parts_[kEngineCrank].angle = std::fmod(crankAngle_, kTwoPi);
        parts_[kEngineCam].angle = std::fmod(camAngle_, kTwoPi);
        parts_[kEngineTensioner].angle += speeds_[kEngineTensioner] * dt;
        parts_[kEngineIntakeCam].angle = parts_[kEngineCam].angle;
        parts_[kEngineExhaustCam].angle = std::fmod(parts_[kEngineCam].angle + 1.5707963f, kTwoPi);

        // 4-Stroke cycle kinematics (0 to 4*pi)
        const float psi = crankAngle_;
        if (psi < kPi) {
            // Stroke 1: INTAKE (0 to pi = 0 to 180 deg)
            strokeNumber_ = 1;
            intakeValveLift_ = 16.0f * std::sin(psi);
            exhaustValveLift_ = 0.0f;
            combustionActive_ = false;
        } else if (psi < 2.0f * kPi) {
            // Stroke 2: COMPRESSION (pi to 2*pi = 180 to 360 deg)
            strokeNumber_ = 2;
            intakeValveLift_ = 0.0f;
            exhaustValveLift_ = 0.0f;
            combustionActive_ = false;
        } else if (psi < 3.0f * kPi) {
            // Stroke 3: POWER / COMBUSTION (2*pi to 3*pi = 360 to 540 deg)
            strokeNumber_ = 3;
            intakeValveLift_ = 0.0f;
            exhaustValveLift_ = 0.0f;
            combustionActive_ = (psi >= 2.0f * kPi && psi < 2.0f * kPi + 0.65f && beltTensioned_);
        } else {
            // Stroke 4: EXHAUST (3*pi to 4*pi = 540 to 720 deg)
            strokeNumber_ = 4;
            intakeValveLift_ = 0.0f;
            exhaustValveLift_ = 16.0f * std::sin(psi - 3.0f * kPi);
            combustionActive_ = false;
        }

        // Piston reciprocating motion: TDC = 275px, BDC = 351px (Stroke = 76px)
        pistonY_ = 275.0f + 38.0f * (1.0f - std::cos(crankAngle_));
        rodAngle_ = std::asin(std::clamp((38.0f / 105.0f) * std::sin(crankAngle_), -1.0f, 1.0f));

        // Valve-to-piston interference check:
        // If piston is near TDC (< 288px) and a valve is depressed (> 6px), collision occurs!
        if (pistonY_ < 288.0f && !beltTensioned_ && (intakeValveLift_ > 6.0f || exhaustValveLift_ > 6.0f)) {
            interferenceCollision_ = true;
            currentMotorSpeed_ = 0.0f;
            playing_ = false;
        }
        return;
    }

    // Classic mode integration
    for (size_t i = 0; i < parts_.size(); ++i) {
        if (std::abs(speeds_[i]) > 1e-6f) {
            parts_[i].angle += speeds_[i] * dt;
            parts_[i].angle = std::fmod(parts_[i].angle, kTwoPi);
            if (parts_[i].angle < 0.0f) {
                parts_[i].angle += kTwoPi;
            }
        }
    }
}

const char* Simulation::strokeName() const {
    switch (strokeNumber_) {
        case 1: return "INTAKE";
        case 2: return "COMPRESSION";
        case 3: return "POWER";
        case 4: return "EXHAUST";
        default: return "IDLE";
    }
}

float Simulation::engineRpm() const {
    // 4.0 rad/s ~ 1200 RPM
    return std::abs(currentMotorSpeed_) * 300.0f;
}

float Simulation::horsepower() const {
    return powerLevel_ * 185.0f;
}

}  // namespace lumina
