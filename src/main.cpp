// Lumina Gear - Scene Renderer
#include <iostream>

#include "lumina/framebuffer.h"
#include "lumina/scene.h"
#include "lumina/simulation.h"

int main() {
    using namespace lumina;
    bool ok = true;
    for (bool powered : {true, false}) {
        Framebuffer fb(kSceneWidth, kSceneHeight);
        renderScene(fb, powered);
        const char* name = powered ? "lumina_gear_cp1_powered.bmp" : "lumina_gear_cp1_unpowered.bmp";
        if (fb.saveBMP(name)) std::cout << "Saved " << name << "\n";
        else { std::cerr << "Could not write " << name << "\n"; ok = false; }
    }

    // Render Level 5 Power Plant
    {
        Simulation sim(SimulationMode::EngineValvetrain);
        sim.setLevel(5);
        sim.play();
        for (int i = 0; i < 45; ++i) sim.update(0.016f);

        SceneView view;
        view.currentLevel = 5;
        view.isEngineMode = true;
        view.powered = sim.isPowered();
        view.powerLevel = sim.powerLevel();
        view.crankAngle = sim.crankAngle();
        view.camAngle = sim.camAngle();
        view.alternatorAngle = sim.crankAngle() * 1.5f;
        view.intakeLift = sim.intakeValveLift();
        view.exhaustLift = sim.exhaustValveLift();
        view.pistonY = sim.pistonY();
        view.rodAngle = sim.connectingRodAngle();
        view.stroke = sim.strokeNumber();
        view.combustion = sim.isCombustion();
        view.beltTensioned = sim.isBeltTensioned();
        view.alternatorDriven = sim.isAlternatorDriven();
        view.wiringConnected = sim.isWiringConnected();
        view.collision = sim.isInterferenceCollision();
        view.rpm = sim.engineRpm();
        view.alternatorRpm = sim.alternatorRpm();
        view.voltage = sim.electricalVoltage();
        view.hp = sim.horsepower();
        view.objective = sim.currentObjective();

        Framebuffer fb(kSceneWidth, kSceneHeight);
        renderScene(fb, sim.parts(), view);
        const char* name = "lumina_assembly_complete.bmp";
        if (fb.saveBMP(name)) std::cout << "Saved " << name << "\n";
        else { std::cerr << "Could not write " << name << "\n"; ok = false; }
    }

    return ok ? 0 : 1;
}
