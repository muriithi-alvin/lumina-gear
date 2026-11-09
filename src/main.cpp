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

    // 1. Render Complete Working Engine Power Plant (with Lightbulb glowing)
    {
        Simulation sim(SimulationMode::EngineValvetrain);
        sim.play();
        for (int i = 0; i < 45; ++i) {
            sim.update(0.016f);
        }

        SceneView view;
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
        view.assemblyComplete = sim.isAssemblyComplete();
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

    // 2. Render Clear Workbench with Blueprint Assembly Targets
    {
        Simulation sim(SimulationMode::EngineValvetrain);
        sim.clearWorkbench();

        SceneView view;
        view.isEngineMode = true;
        view.powered = false;
        view.powerLevel = 0.0f;
        view.showBlueprints = true;
        view.objective = sim.currentObjective();

        Framebuffer fb(kSceneWidth, kSceneHeight);
        renderScene(fb, sim.parts(), view);
        const char* name = "lumina_assembly_blueprint.bmp";
        if (fb.saveBMP(name)) std::cout << "Saved " << name << "\n";
        else { std::cerr << "Could not write " << name << "\n"; ok = false; }
    }

    return ok ? 0 : 1;
}
