#include "lumina/simulation.h"

#include <algorithm>
#include <cmath>
#include "lumina/level.h"
#include "lumina/scene.h"

namespace lumina {

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 2.0f * kPi;
constexpr float kFourPi = 4.0f * kPi;
}  // namespace

Simulation::Simulation(SimulationMode mode) : mode_(mode) {
    if (mode == SimulationMode::EngineValvetrain) {
        initEngineScene();
    } else {
        setLevel(1);
    }
}

Simulation::~Simulation() = default;

void Simulation::setLevel(int level) {
    currentLevel_ = std::clamp(level, 1, 5);
    playing_ = false;
    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    interferenceCollision_ = false;
    reversedPolarity_ = false;
    consecutiveCleanCycles_ = 0;
    crossedBelt_ = false;

    activeLevel_ = createLevel(currentLevel_);
    if (activeLevel_) {
        mode_ = activeLevel_->mode();
        targetMotorSpeed_ = activeLevel_->targetMotorSpeed();
        activeLevel_->setup(*this);
    }
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
    parts_.resize(kClassicTotalParts);
    // 14T Reverse Idler Gear (r = 17, teeth = 14)
    parts_[kPartIdler] = makeGear(500.0f, 500.0f, 14, 0.0f);
    parts_[kPartIdler].placed = false; // Kept in toolbox until dragged or solved in Level 3
    speeds_.assign(parts_.size(), 0.0f);
    connections_.clear();

    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    playing_ = false;
    reversedPolarity_ = false;
    crossedBelt_ = false;

    autoDetectConnections();
    solveKinematics();
}

void Simulation::initEngineScene() {
    parts_.clear();
    parts_.resize(kEnginePartCount);

    // Spaced out nicely for 1360 x 760 canvas
    parts_[kEngineCrank] = makeGear(320.0f, 560.0f, 12, 0.0f);
    parts_[kEngineCam] = makeGear(320.0f, 200.0f, 24, 0.0f);
    parts_[kEngineTensioner] = makePulley(440.0f, 380.0f, 24);
    parts_[kEngineValvetrain] = makePiston(840.0f, 320.0f, 55);
    parts_[kEngineAlternator] = makeAlternator(560.0f, 560.0f, 28);
    parts_[kEngineBulb] = makeBulb(1160.0f, 360.0f, 46, false);

    // Internal valvetrain subcomponents (LSA: ICL = 105 deg ATDC, ECL = 105 deg BBDC)
    parts_[kEngineIntakeCam] = makeCamLobe(760.0f, 200.0f, 24, 0.6544985f);
    parts_[kEngineExhaustCam] = makeCamLobe(920.0f, 200.0f, 24, 2.4870942f);
    parts_[kEngineIntakeValve] = makeValve(760.0f, 240.0f, 18);
    parts_[kEngineExhaustValve] = makeValve(920.0f, 240.0f, 18);
    parts_[kEnginePiston] = makePiston(840.0f, 320.0f, 55);

    for (auto& p : parts_) p.placed = true;

    speeds_.assign(kEnginePartCount, 0.0f);
    connections_.clear();

    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    playing_ = false;

    crankAngle_ = 0.0f;
    camAngle_ = 0.0f;
    alternatorAngle_ = 0.0f;
    intakeValveLift_ = 0.0f;
    exhaustValveLift_ = 0.0f;
    pistonY_ = 275.0f;
    rodAngle_ = 0.0f;
    strokeNumber_ = 1;
    combustionActive_ = false;
    beltTensioned_ = true;
    alternatorDriven_ = true;
    wiringConnected_ = true;
    interferenceCollision_ = false;
    consecutiveCleanCycles_ = 0;

    autoDetectConnections();
    solveKinematics();
}

void Simulation::initEngineTimingBench() {
    initEngineScene();
    // Level 4: 4-Stroke Valvetrain Timing Bench ONLY!
    // Alternator and bulb are not present on this test bench.
    parts_[kEngineAlternator].placed = false;
    parts_[kEngineBulb].placed = false;
    // Starts with wrong 12T gear on camshaft (1:1 ratio) to demonstrate valve collision!
    parts_[kEngineCam].teeth = 12;
    parts_[kEngineCam].radius = static_cast<int>(gearPitchRadius(12));
    alternatorDriven_ = false;
    wiringConnected_ = false;
    autoDetectConnections();
    solveKinematics();
}

void Simulation::initMasterPowerPlantSetup() {
    initEngineScene();
    // Level 5: Full Master Power Plant grid!
    // Engine timing is correct (24T Cam), but Dynamo and 100W Lamp start unbuilt in the toolbox!
    parts_[kEngineCam].teeth = 24;
    parts_[kEngineCam].radius = static_cast<int>(gearPitchRadius(24));
    parts_[kEngineAlternator].placed = false;
    parts_[kEngineBulb].placed = false;
    alternatorDriven_ = false;
    wiringConnected_ = false;
    autoDetectConnections();
    solveKinematics();
}

void Simulation::clearWorkbench() {
    playing_ = false;
    currentMotorSpeed_ = 0.0f;
    powerLevel_ = 0.0f;
    interferenceCollision_ = false;
    reversedPolarity_ = false;

    for (auto& p : parts_) {
        p.placed = false;
    }

    autoDetectConnections();
    solveKinematics();
}

void Simulation::solveLevel() {
    if (activeLevel_) {
        activeLevel_->solve(*this);
    }
    autoDetectConnections();
    solveKinematics();
}

bool Simulation::isLevelCompleted() const {
    if (activeLevel_) {
        return activeLevel_->isCompleted(*this);
    }
    return false;
}

const char* Simulation::currentObjective() const {
    if (activeLevel_) {
        return activeLevel_->objective();
    }
    return "";
}

const char* Simulation::failureReason() const {
    if (activeLevel_) {
        const char* r = activeLevel_->failureReason(*this);
        if (r && r[0] != '\0') return r;
    }
    if (interferenceCollision_) {
        return "FATAL CRASH: Piston struck open valve at TDC! Need exact 2:1 reduction ratio.";
    }
    if (reversedPolarity_) {
        return "POLARITY REVERSED: Generator spinning backward! Re-route drive to spin Clockwise.";
    }
    if (!beltTensioned_ && mode_ == SimulationMode::EngineValvetrain) {
        return "BELT SLACK: Camshaft slipping! Tighten tensioner pulley.";
    }
    if (powerLevel_ < 0.1f) {
        return "INCOMPLETE CIRCUIT: Connect missing drives/wires to transmit power.";
    }
    return "";
}

bool Simulation::isPartPlaced(int slotIndex) const {
    if (slotIndex >= 0 && slotIndex < static_cast<int>(parts_.size())) {
        return parts_[slotIndex].placed;
    }
    return false;
}

void Simulation::setPartPlaced(int slotIndex, bool placed) {
    if (slotIndex >= 0 && slotIndex < static_cast<int>(parts_.size())) {
        parts_[slotIndex].placed = placed;
        autoDetectConnections();
        solveKinematics();
    }
}

void Simulation::placePartAt(int slotIndex, float x, float y) {
    if (slotIndex >= 0 && slotIndex < static_cast<int>(parts_.size())) {
        parts_[slotIndex].x = x;
        parts_[slotIndex].y = y;
        parts_[slotIndex].placed = true;
        // NOTE: No snapToMesh while placing/dragging!
        // Smooth dragging without flickering; snapping only occurs on onMouseUp.
        autoDetectConnections();
        solveKinematics();
    }
}

bool Simulation::isAssemblyComplete() const {
    if (mode_ != SimulationMode::EngineValvetrain) {
        for (const auto& p : parts_) if (!p.placed) return false;
        return powerLevel_ > 0.05f;
    }
    return parts_[kEngineCrank].placed && parts_[kEngineCam].placed &&
           parts_[kEngineTensioner].placed && parts_[kEngineValvetrain].placed &&
           parts_[kEngineAlternator].placed && parts_[kEngineBulb].placed &&
           beltTensioned_ && alternatorDriven_ && wiringConnected_;
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
    alternatorAngle_ = 0.0f;
    intakeValveLift_ = 0.0f;
    exhaustValveLift_ = 0.0f;
    pistonY_ = 275.0f;
    rodAngle_ = 0.0f;
    strokeNumber_ = 1;
    combustionActive_ = false;
    interferenceCollision_ = false;
    reversedPolarity_ = false;
    consecutiveCleanCycles_ = 0;
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
        if (partIndex == kEngineTensioner) return beltTensioned_;
        if (partIndex == kEngineAlternator) return alternatorDriven_;
        if (partIndex == kEngineBulb) return wiringConnected_;
        return parts_[partIndex].placed;
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
        if (partIndex == kEngineCam) {
            if (std::hypot(parts_[kEngineCam].x - 320.0f, parts_[kEngineCam].y - 200.0f) < 45.0f) {
                parts_[kEngineCam].x = 320.0f;
                parts_[kEngineCam].y = 200.0f;
            }
        } else if (partIndex == kEngineCrank) {
            if (std::hypot(parts_[kEngineCrank].x - 320.0f, parts_[kEngineCrank].y - 560.0f) < 45.0f) {
                parts_[kEngineCrank].x = 320.0f;
                parts_[kEngineCrank].y = 560.0f;
            }
        } else if (partIndex == kEngineTensioner) {
            if (std::hypot(parts_[kEngineTensioner].x - 440.0f, parts_[kEngineTensioner].y - 380.0f) < 45.0f) {
                parts_[kEngineTensioner].x = 440.0f;
                parts_[kEngineTensioner].y = 380.0f;
            }
        } else if (partIndex == kEngineValvetrain) {
            if (std::hypot(parts_[kEngineValvetrain].x - 840.0f, parts_[kEngineValvetrain].y - 320.0f) < 45.0f) {
                parts_[kEngineValvetrain].x = 840.0f;
                parts_[kEngineValvetrain].y = 320.0f;
            }
        } else if (partIndex == kEngineAlternator) {
            if (std::hypot(parts_[kEngineAlternator].x - 560.0f, parts_[kEngineAlternator].y - 560.0f) < 50.0f) {
                parts_[kEngineAlternator].x = 560.0f;
                parts_[kEngineAlternator].y = 560.0f;
            }
        } else if (partIndex == kEngineBulb) {
            if (std::hypot(parts_[kEngineBulb].x - 1160.0f, parts_[kEngineBulb].y - 360.0f) < 50.0f) {
                parts_[kEngineBulb].x = 1160.0f;
                parts_[kEngineBulb].y = 360.0f;
            }
        }
        autoDetectConnections();
        solveKinematics();
        return;
    }

    // Classic Workbench Snapping:
    if (partIndex == kPartGear1 || partIndex == kPartGear2) {
        // If dropped near motor shaft mount (370, 380) and slot is open: snap to motor shaft!
        const int other = (partIndex == kPartGear1) ? kPartGear2 : kPartGear1;
        const bool otherAtMotor = parts_[other].placed && (std::hypot(parts_[other].x - 370.0f, parts_[other].y - 380.0f) < 40.0f);
        if (!otherAtMotor && std::hypot(parts_[partIndex].x - 370.0f, parts_[partIndex].y - 380.0f) < 45.0f) {
            parts_[partIndex].x = 370.0f;
            parts_[partIndex].y = 380.0f;
            parts_[partIndex].angle = 0.0f;
            autoDetectConnections();
            solveKinematics();
            return;
        }
    } else if (partIndex == kPartPulley) {
        if (std::hypot(parts_[kPartPulley].x - 640.0f, parts_[kPartPulley].y - 380.0f) < 45.0f) {
            parts_[kPartPulley].x = 640.0f;
            parts_[kPartPulley].y = 380.0f;
            autoDetectConnections();
            solveKinematics();
            return;
        }
    } else if (partIndex == kPartWheel) {
        if (std::hypot(parts_[kPartWheel].x - 1080.0f, parts_[kPartWheel].y - 520.0f) < 45.0f) {
            parts_[kPartWheel].x = 1080.0f;
            parts_[kPartWheel].y = 520.0f;
            autoDetectConnections();
            solveKinematics();
            return;
        }
    } else if (partIndex == kPartBulb) {
        if (std::hypot(parts_[kPartBulb].x - 1080.0f, parts_[kPartBulb].y - 290.0f) < 45.0f) {
            parts_[kPartBulb].x = 1080.0f;
            parts_[kPartBulb].y = 290.0f;
            autoDetectConnections();
            solveKinematics();
            return;
        }
    }

    // Freeform gear mesh snapping: snap to nearby meshing gears anywhere on workbench!
    if (parts_[partIndex].type == ComponentType::Gear) {
        for (size_t j = 0; j < parts_.size(); ++j) {
            if (static_cast<int>(j) == partIndex || !parts_[j].placed) continue;
            if (parts_[j].type != ComponentType::Gear) continue;

            const float dx = parts_[partIndex].x - parts_[j].x;
            const float dy = parts_[partIndex].y - parts_[j].y;
            const float dist = std::hypot(dx, dy);
            const float expectedDist = gearPitchRadius(parts_[partIndex].teeth) + gearPitchRadius(parts_[j].teeth);

            if (dist > 1.0f && std::abs(dist - expectedDist) <= 22.0f) {
                const float angle = std::atan2(dy, dx);
                parts_[partIndex].x = parts_[j].x + expectedDist * std::cos(angle);
                parts_[partIndex].y = parts_[j].y + expectedDist * std::sin(angle);
                parts_[partIndex].angle = meshAngle(parts_[partIndex].teeth, angle + kPi);
                autoDetectConnections();
                solveKinematics();
                return;
            }
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

        const Component& crank = parts_[kEngineCrank];
        const Component& cam = parts_[kEngineCam];
        const Component& tens = parts_[kEngineTensioner];
        const Component& vtrain = parts_[kEngineValvetrain];
        const Component& alt = parts_[kEngineAlternator];
        const Component& bulb = parts_[kEngineBulb];

        // Tensioner must be placed in tension corridor
        beltTensioned_ = tens.placed && (tens.x >= 380.0f && tens.x <= 500.0f && tens.y >= 300.0f && tens.y <= 460.0f);

        const float dCrankCam = std::hypot(crank.x - cam.x, crank.y - cam.y);
        const bool sprocketsAligned = crank.placed && cam.placed && (dCrankCam >= 300.0f && dCrankCam <= 420.0f);

        if (sprocketsAligned && beltTensioned_ && vtrain.placed) {
            addConnection(kEngineCrank, kEngineCam, ConnectionType::Belt);
            addConnection(kEngineCam, kEngineIntakeCam, ConnectionType::Shaft);
            addConnection(kEngineCam, kEngineExhaustCam, ConnectionType::Shaft);
        }

        const float dCrankAlt = std::hypot(crank.x - alt.x, crank.y - alt.y);
        alternatorDriven_ = crank.placed && alt.placed && (dCrankAlt >= 160.0f && dCrankAlt <= 340.0f);
        if (alternatorDriven_) {
            addConnection(kEngineCrank, kEngineAlternator, ConnectionType::Belt);
        }

        const float dAltBulb = std::hypot(alt.x - bulb.x, alt.y - (bulb.y + bulb.radius));
        wiringConnected_ = alt.placed && bulb.placed && (dAltBulb <= 750.0f);
        if (wiringConnected_ && alternatorDriven_) {
            addConnection(kEngineAlternator, kEngineBulb, ConnectionType::Generator);
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

    // 1. Shaft from Motor to whichever gear is placed closest to the motor
    int motorGear = -1;
    float bestMotorDist = 260.0f;
    for (int g : {kPartGear1, kPartGear2}) {
        if (parts_[g].placed) {
            const float d = std::hypot(motor.x - parts_[g].x, motor.y - parts_[g].y);
            if (d <= bestMotorDist) {
                bestMotorDist = d;
                motorGear = g;
            }
        }
    }
    if (motor.placed && motorGear >= 0) {
        addConnection(kPartMotor, motorGear, ConnectionType::Shaft);
    }

    // 2. Shaft from Pulley to whichever gear is placed closest to the pulley (excluding motorGear)
    int pulleyGear = -1;
    float bestPulleyDist = 320.0f;
    for (int g : {kPartGear1, kPartGear2}) {
        if (parts_[g].placed && g != motorGear) {
            const float d = std::hypot(pulley.x - parts_[g].x, pulley.y - parts_[g].y);
            if (d <= bestPulleyDist) {
                bestPulleyDist = d;
                pulleyGear = g;
            }
        }
    }
    if (pulley.placed && pulleyGear >= 0) {
        addConnection(pulleyGear, kPartPulley, ConnectionType::Shaft);
    }

    // 3. Meshes between gears (supporting direct mesh or via Idler)
    const bool hasIdler = (parts_.size() > kPartIdler && parts_[kPartIdler].placed);
    const int otherGear = (motorGear >= 0) ? ((motorGear == kPartGear1) ? kPartGear2 : kPartGear1) : -1;
    const int drivenGear = (pulleyGear >= 0) ? pulleyGear : (otherGear >= 0 && parts_[otherGear].placed ? otherGear : -1);

    if (hasIdler) {
        const Component& idler = parts_[kPartIdler];
        if (motorGear >= 0) {
            const float d1 = std::hypot(parts_[motorGear].x - idler.x, parts_[motorGear].y - idler.y);
            const float exp1 = gearPitchRadius(parts_[motorGear].teeth) + gearPitchRadius(idler.teeth);
            if (std::abs(d1 - exp1) <= 18.0f) {
                addConnection(motorGear, kPartIdler, ConnectionType::GearMesh);
            }
        }
        if (drivenGear >= 0) {
            const float d2 = std::hypot(idler.x - parts_[drivenGear].x, idler.y - parts_[drivenGear].y);
            const float exp2 = gearPitchRadius(idler.teeth) + gearPitchRadius(parts_[drivenGear].teeth);
            if (std::abs(d2 - exp2) <= 18.0f) {
                addConnection(kPartIdler, drivenGear, ConnectionType::GearMesh);
            }
        }
    } else {
        if (motorGear >= 0 && drivenGear >= 0 && motorGear != drivenGear) {
            const float d = std::hypot(parts_[motorGear].x - parts_[drivenGear].x,
                                       parts_[motorGear].y - parts_[drivenGear].y);
            const float expected = gearPitchRadius(parts_[motorGear].teeth) + gearPitchRadius(parts_[drivenGear].teeth);
            if (std::abs(d - expected) <= 18.0f) {
                addConnection(motorGear, drivenGear, ConnectionType::GearMesh);
            }
        } else if (gear1.placed && gear2.placed) {
            const float d = std::hypot(gear1.x - gear2.x, gear1.y - gear2.y);
            const float expected = gearPitchRadius(gear1.teeth) + gearPitchRadius(gear2.teeth);
            if (std::abs(d - expected) <= 18.0f) {
                addConnection(kPartGear1, kPartGear2, ConnectionType::GearMesh);
            }
        }
    }

    if (pulley.placed && wheel.placed) {
        const float d = std::hypot(pulley.x - wheel.x, pulley.y - wheel.y);
        if (d > static_cast<float>(pulley.radius + wheel.radius) && d <= 700.0f) {
            addConnection(kPartPulley, kPartWheel, ConnectionType::Belt);
        }
    }

    if (wheel.placed && bulb.placed) {
        const float d = std::hypot(wheel.x - bulb.x, wheel.y - (bulb.y + bulb.radius * 1.3f));
        if (d <= 450.0f) addConnection(kPartWheel, kPartBulb, ConnectionType::Generator);
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
    reversedPolarity_ = false;
    for (auto& p : parts_) {
        if (p.type == ComponentType::Lightbulb) p.lit = false;
    }

    for (size_t i = 0; i < parts_.size(); ++i) {
        if (parts_[i].type == ComponentType::Motor && parts_[i].placed) {
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
                        const float dir = crossedBelt_ ? -1.0f : 1.0f;
                        expectedDstSpeed = dir * srcSpeed * (radA / radB);
                    }
                    break;
                }
                case ConnectionType::Generator: {
                    // Check polarity in Level 3: Wheel spinning with positive speed means reversed polarity (need CW/negative speed)
                    if (currentLevel_ == 3 && srcSpeed > 0.0f) {
                        reversedPolarity_ = true;
                        powerLevel_ = 0.0f;
                    } else {
                        reversedPolarity_ = false;
                        const float nominalWheelSpeed = 4.0f * (12.0f / 18.0f) * (40.0f / 22.0f);
                        powerLevel_ = std::clamp(std::abs(srcSpeed) / nominalWheelSpeed, 0.0f, 1.0f);
                        const float volt = powerLevel_ * 14.4f;
                        if (currentLevel_ == 2 && volt < 12.0f) {
                            parts_[conn.targetIndex].lit = false;
                        } else {
                            parts_[conn.targetIndex].lit = (powerLevel_ > 0.05f);
                        }
                    }
                    break;
                }
                case ConnectionType::Wire:
                    break;
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

    const bool engineRuns = parts_[kEngineCrank].placed && parts_[kEngineValvetrain].placed;
    if (engineRuns) {
        speeds_[kEngineCrank] = currentMotorSpeed_;
    } else {
        speeds_[kEngineCrank] = 0.0f;
    }

    if (beltTensioned_ && parts_[kEngineCam].placed && engineRuns) {
        // Gear ratio based on actual teeth count of camshaft gear!
        const int camTeeth = parts_[kEngineCam].teeth;
        const float camRatio = (camTeeth > 0) ? (12.0f / static_cast<float>(camTeeth)) : 0.5f;

        speeds_[kEngineCam] = camRatio * currentMotorSpeed_;
        speeds_[kEngineTensioner] = currentMotorSpeed_ * 1.5f;
        speeds_[kEngineIntakeCam] = speeds_[kEngineCam];
        speeds_[kEngineExhaustCam] = speeds_[kEngineCam];
    } else {
        speeds_[kEngineCam] = 0.0f;
        speeds_[kEngineTensioner] = 0.0f;
        speeds_[kEngineIntakeCam] = 0.0f;
        speeds_[kEngineExhaustCam] = 0.0f;
    }

    if (alternatorDriven_ && parts_[kEngineAlternator].placed && engineRuns) {
        speeds_[kEngineAlternator] = currentMotorSpeed_ * 1.5f;
        parts_[kEngineAlternator].lit = (std::abs(speeds_[kEngineAlternator]) > 1.0f);
    } else {
        speeds_[kEngineAlternator] = 0.0f;
        parts_[kEngineAlternator].lit = false;
    }

    if (wiringConnected_ && parts_[kEngineBulb].placed && alternatorDriven_ && engineRuns && beltTensioned_ && !interferenceCollision_) {
        powerLevel_ = std::clamp(std::abs(currentMotorSpeed_) / targetMotorSpeed_, 0.0f, 1.0f);
        parts_[kEngineBulb].lit = (powerLevel_ > 0.05f);
    } else {
        powerLevel_ = 0.0f;
        parts_[kEngineBulb].lit = false;
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
        const float oldCrank = crankAngle_;
        crankAngle_ += speeds_[kEngineCrank] * dt;
        crankAngle_ = std::fmod(crankAngle_, kFourPi);
        if (crankAngle_ < 0.0f) crankAngle_ += kFourPi;

        if (crankAngle_ < oldCrank && playing_) {
            // Completed one full 4-stroke cycle
            if (!interferenceCollision_ && beltTensioned_) {
                consecutiveCleanCycles_++;
            }
        }

        // Camshaft advances based on its kinematic speed
        camAngle_ += speeds_[kEngineCam] * dt;
        camAngle_ = std::fmod(camAngle_, kTwoPi);
        if (camAngle_ < 0.0f) camAngle_ += kTwoPi;

        alternatorAngle_ += speeds_[kEngineAlternator] * dt;
        alternatorAngle_ = std::fmod(alternatorAngle_, kTwoPi);

        parts_[kEngineCrank].angle = std::fmod(crankAngle_, kTwoPi);
        parts_[kEngineCam].angle = camAngle_;
        parts_[kEngineTensioner].angle += speeds_[kEngineTensioner] * dt;
        parts_[kEngineIntakeCam].angle = std::fmod(camAngle_ + 0.6544985f, kTwoPi);
        parts_[kEngineExhaustCam].angle = std::fmod(camAngle_ + 2.4870942f, kTwoPi);
        parts_[kEngineAlternator].angle = alternatorAngle_;

        // Valve lifts derived directly from rotating cam lobes (opening over 180 deg crank duration)
        const float cosIn = std::cos(parts_[kEngineIntakeCam].angle - 1.5707963f);
        const float tIn = (cosIn > 0.7071f) ? ((cosIn - 0.7071f) / (1.0f - 0.7071f)) : 0.0f;
        intakeValveLift_ = 16.0f * tIn * tIn;

        const float cosEx = std::cos(parts_[kEngineExhaustCam].angle - 1.5707963f);
        const float tEx = (cosEx > 0.7071f) ? ((cosEx - 0.7071f) / (1.0f - 0.7071f)) : 0.0f;
        exhaustValveLift_ = 16.0f * tEx * tEx;

        // Piston reciprocating motion (TDC = 275px, BDC = 351px)
        pistonY_ = 275.0f + 38.0f * (1.0f - std::cos(crankAngle_));
        rodAngle_ = std::asin(std::clamp((38.0f / 105.0f) * std::sin(crankAngle_), -1.0f, 1.0f));

        // Stroke indicator derived from crank angle
        const float psi = crankAngle_;
        if (psi < kPi) strokeNumber_ = 1;
        else if (psi < 2.0f * kPi) strokeNumber_ = 2;
        else if (psi < 3.0f * kPi) strokeNumber_ = 3;
        else strokeNumber_ = 4;

        combustionActive_ = (psi >= 2.0f * kPi && psi < 2.0f * kPi + 0.65f &&
                             beltTensioned_ && parts_[kEngineValvetrain].placed && !interferenceCollision_);

        // =====================================================================
        // VALVE-TO-PISTON INTERFERENCE COLLISION CHECK!
        // =====================================================================
        // If piston is near TDC (< 285px) and a valve is depressed (> 5px):
        // Piston collides into the valve head!
        if (pistonY_ < 285.0f && (intakeValveLift_ > 5.0f || exhaustValveLift_ > 5.0f) && parts_[kEngineValvetrain].placed) {
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
            if (parts_[i].angle < 0.0f) parts_[i].angle += kTwoPi;
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
    return std::abs(currentMotorSpeed_) * 300.0f;
}

float Simulation::alternatorRpm() const {
    return engineRpm() * 1.5f;
}

float Simulation::electricalVoltage() const {
    if (powerLevel_ <= 0.01f || reversedPolarity_) return 0.0f;
    return powerLevel_ * 14.4f; // 0.0V to 14.4V DC proportional to generator speed
}

float Simulation::horsepower() const {
    if (interferenceCollision_) return 0.0f;
    return powerLevel_ * 185.0f;
}

}  // namespace lumina
