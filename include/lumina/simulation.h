// simulation.h - Kinematics and continuous simulation engine for CP2.
// Handles power transmission, gear ratios, belt speeds, and rotational animation.
#pragma once

#include <vector>
#include "lumina/components.h"

namespace lumina {

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

class Simulation {
public:
    Simulation();

    // Initialize with the standard default scene components and connections
    void initDefaultScene();

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

private:
    void solveKinematics();

    std::vector<Component> parts_;
    std::vector<float> speeds_;            // current angular velocity (rad/s) for each part
    std::vector<Connection> connections_;

    bool playing_ = false;
    float currentMotorSpeed_ = 0.0f;       // rad/s
    float targetMotorSpeed_ = 4.0f;        // nominal ~0.64 rev/s (smooth and clearly visible)
    float powerLevel_ = 0.0f;              // 0.0 to 1.0 delivered to bulb
};

}  // namespace lumina
