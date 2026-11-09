// simulation.h - Kinematics and continuous simulation engine for CP2.
// Supports both Classic Mechanical Power mode and Engine Valvetrain Timing mode.
#pragma once

#include <vector>
#include "lumina/components.h"

namespace lumina {

enum class SimulationMode {
    ClassicLumina,
    EngineValvetrain
};

enum class ConnectionType {
    Shaft,       // Rigid common axle: omegaB = omegaA
    GearMesh,    // Meshed gears: omegaB = -omegaA * (teethA / teethB)
    Belt,        // Pulleys connected by open belt: omegaB = omegaA * (radiusA / radiusB)
    Generator    // Wheel driving lightbulb generator
};

struct Connection {
    int sourceIndex = -1;
    int targetIndex = -1;
    ConnectionType type = ConnectionType::Shaft;
};

// Index mappings for Engine Valvetrain mode
enum EnginePartIndex {
    kEngineCrank = 0,         // Crankshaft Drive Sprocket (12 teeth)
    kEngineCam = 1,           // Camshaft Driven Sprocket (24 teeth)
    kEngineTensioner = 2,     // Belt Tensioner / Idler Pulley
    kEngineIntakeCam = 3,     // Intake Cam Lobe
    kEngineExhaustCam = 4,    // Exhaust Cam Lobe
    kEngineIntakeValve = 5,   // Intake Poppet Valve
    kEngineExhaustValve = 6,  // Exhaust Poppet Valve
    kEnginePiston = 7,        // Piston & Cylinder assembly
    kEnginePartCount = 8
};

class Simulation {
public:
    explicit Simulation(SimulationMode mode = SimulationMode::ClassicLumina);

    // Mode management
    SimulationMode mode() const { return mode_; }
    void setMode(SimulationMode m);

    // Initialize scenes
    void initDefaultScene();
    void initClassicScene();
    void initEngineScene();

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

    // Automatically inspects current parts positions to detect meshes and belt links
    void autoDetectConnections();

    // Engine Valvetrain telemetry and state
    float crankAngle() const { return crankAngle_; }
    float camAngle() const { return camAngle_; }
    float intakeValveLift() const { return intakeValveLift_; }
    float exhaustValveLift() const { return exhaustValveLift_; }
    float pistonY() const { return pistonY_; }
    float connectingRodAngle() const { return rodAngle_; }
    bool isCombustion() const { return combustionActive_; }
    bool isBeltTensioned() const { return beltTensioned_; }
    bool isInterferenceCollision() const { return interferenceCollision_; }
    const char* strokeName() const;
    int strokeNumber() const { return strokeNumber_; }
    float engineRpm() const;
    float horsepower() const;

private:
    void solveKinematics();
    void solveClassicKinematics();
    void solveEngineKinematics();

    SimulationMode mode_ = SimulationMode::ClassicLumina;
    std::vector<Component> parts_;
    std::vector<float> speeds_;
    std::vector<Connection> connections_;

    bool playing_ = false;
    float currentMotorSpeed_ = 0.0f;       // rad/s
    float targetMotorSpeed_ = 4.0f;        // nominal ~0.64 rev/s
    float powerLevel_ = 0.0f;              // 0.0 to 1.0

    // Engine state variables
    float crankAngle_ = 0.0f;              // radians
    float camAngle_ = 0.0f;                // radians (crank / 2 in 4-stroke)
    float intakeValveLift_ = 0.0f;         // px lift
    float exhaustValveLift_ = 0.0f;        // px lift
    float pistonY_ = 275.0f;               // TDC = 275, BDC = 351
    float rodAngle_ = 0.0f;                // radians
    int strokeNumber_ = 1;                 // 1: Intake, 2: Compression, 3: Power, 4: Exhaust
    bool combustionActive_ = false;
    bool beltTensioned_ = true;
    bool interferenceCollision_ = false;
};

}  // namespace lumina
