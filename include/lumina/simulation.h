// simulation.h - Kinematics and continuous simulation engine for CP2.
// Supports the 5-Level Progressive Engineering Physics Campaign.
#pragma once

#include <memory>
#include <string>
#include <vector>
#include "lumina/components.h"

namespace lumina {

class Level;

enum class SimulationMode {
    ClassicLumina,
    EngineValvetrain
};

enum class ConnectionType {
    Shaft,       // Rigid common axle: omegaB = omegaA
    GearMesh,    // Meshed gears: omegaB = -omegaA * (teethA / teethB)
    Belt,        // Pulleys connected by open belt: omegaB = omegaA * (radiusA / radiusB)
    Generator,   // Alternator driving lightbulb electrical circuit
    Wire
};

struct Connection {
    int sourceIndex = -1;
    int targetIndex = -1;
    ConnectionType type = ConnectionType::Shaft;
};

// Index mappings for Engine Valvetrain Assembly
enum EnginePartIndex {
    kEngineCrank = 0,         // Crankshaft Drive Sprocket (12 teeth)
    kEngineCam = 1,           // Camshaft Driven Sprocket (24 teeth)
    kEngineTensioner = 2,     // Belt Tensioner / Idler Pulley
    kEngineValvetrain = 3,    // Cylinder & Valvetrain assembly base
    kEngineAlternator = 4,    // Alternator generator unit
    kEngineBulb = 5,          // Workshop illumination bulb
    kEngineIntakeCam = 6,     // Intake Cam Lobe
    kEngineExhaustCam = 7,    // Exhaust Cam Lobe
    kEngineIntakeValve = 8,   // Intake Poppet Valve
    kEngineExhaustValve = 9,  // Exhaust Poppet Valve
    kEnginePiston = 10,       // Piston & Cylinder
    kEnginePartCount = 11
};

class Simulation {
public:
    explicit Simulation(SimulationMode mode = SimulationMode::ClassicLumina);
    ~Simulation();

    // Level Management
    int currentLevel() const { return currentLevel_; }
    void setLevel(int level);
    bool isLevelCompleted() const;
    const char* currentObjective() const;
    const char* failureReason() const;

    // Mode management
    SimulationMode mode() const { return mode_; }
    void setMode(SimulationMode m);

    // Scene & Level initialization
    void initDefaultScene();
    void initClassicScene();
    void initEngineScene();
    void initEngineTimingBench();
    void initMasterPowerPlantSetup();
    void clearWorkbench();
    void solveLevel();
    bool isPartPlaced(int slotIndex) const;
    void setPartPlaced(int slotIndex, bool placed);
    void placePartAt(int slotIndex, float x, float y);
    void removePart(int index) { setPartPlaced(index, false); }
    bool isAssemblyComplete() const;
    bool isCrossedBelt() const { return crossedBelt_; }
    void toggleCrossedBelt() { crossedBelt_ = !crossedBelt_; solveKinematics(); }
    void setCrossedBelt(bool crossed) { crossedBelt_ = crossed; solveKinematics(); }

    // Update simulation by delta-time dt (in seconds)
    void update(float dt);

    // Playback control
    void play();
    void pause();
    void togglePlay();
    void reset();

    bool isPlaying() const { return playing_; }
    bool isPowered() const { return powerLevel_ > 0.05f; }
    float powerLevel() const { return powerLevel_; }

    // Access to simulated components
    const std::vector<Component>& parts() const { return parts_; }
    std::vector<Component>& parts() { return parts_; }
    const Component& part(int index) const { return parts_[index]; }
    Component& part(int index) { return parts_[index]; }
    size_t partCount() const { return parts_.size(); }

    // 2D Translation / Drag-and-drop support
    void setPartPosition(int index, float x, float y);
    bool isPartMeshed(int partIndex) const;
    void snapToMesh(int partIndex);

    // Kinematic speeds (in radians/sec)
    float speed(int index) const {
        return (index >= 0 && index < static_cast<int>(speeds_.size())) ? speeds_[index] : 0.0f;
    }
    float motorSpeed() const { return currentMotorSpeed_; }
    float targetMotorSpeed() const { return targetMotorSpeed_; }
    void setTargetMotorSpeed(float radPerSec) { targetMotorSpeed_ = radPerSec; }

    // Dynamic connection management
    void addConnection(int src, int dst, ConnectionType type);
    void clearConnections();
    const std::vector<Connection>& connections() const { return connections_; }
    void autoDetectConnections();

    // Telemetry and electrical stats
    float crankAngle() const { return crankAngle_; }
    float camAngle() const { return camAngle_; }
    float intakeValveLift() const { return intakeValveLift_; }
    float exhaustValveLift() const { return exhaustValveLift_; }
    float pistonY() const { return pistonY_; }
    float connectingRodAngle() const { return rodAngle_; }
    bool isCombustion() const { return combustionActive_; }
    bool isBeltTensioned() const { return beltTensioned_; }
    bool isAlternatorDriven() const { return alternatorDriven_; }
    bool isWiringConnected() const { return wiringConnected_; }
    bool isInterferenceCollision() const { return interferenceCollision_; }
    bool isReversedPolarity() const { return reversedPolarity_; }
    void setInterferenceCollision(bool c) { interferenceCollision_ = c; }
    void setReversedPolarity(bool r) { reversedPolarity_ = r; }
    int consecutiveCleanCycles() const { return consecutiveCleanCycles_; }
    const char* strokeName() const;
    int strokeNumber() const { return strokeNumber_; }
    float engineRpm() const;
    float alternatorRpm() const;
    float electricalVoltage() const;
    float horsepower() const;

private:
    void solveKinematics();
    void solveClassicKinematics();
    void solveEngineKinematics();

    int currentLevel_ = 1;
    std::unique_ptr<Level> activeLevel_;
    SimulationMode mode_ = SimulationMode::ClassicLumina;
    std::vector<Component> parts_;
    std::vector<float> speeds_;
    std::vector<Connection> connections_;

    bool playing_ = false;
    float currentMotorSpeed_ = 0.0f;
    float targetMotorSpeed_ = 4.0f;
    float powerLevel_ = 0.0f;

    // Engine & Electrical state variables
    float crankAngle_ = 0.0f;
    float camAngle_ = 0.0f;
    float alternatorAngle_ = 0.0f;
    float intakeValveLift_ = 0.0f;
    float exhaustValveLift_ = 0.0f;
    float pistonY_ = 275.0f;
    float rodAngle_ = 0.0f;
    int strokeNumber_ = 1;
    bool combustionActive_ = false;
    bool beltTensioned_ = true;
    bool alternatorDriven_ = true;
    bool wiringConnected_ = true;
    bool interferenceCollision_ = false;
    bool reversedPolarity_ = false;
    bool crossedBelt_ = false;

    int consecutiveCleanCycles_ = 0;
};

}  // namespace lumina
