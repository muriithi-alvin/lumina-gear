// Lumina Gear - Scene Renderer
// Draws both the classic checkpoint scene and the Engine Valvetrain assembly.
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

    // Also render an Engine Valvetrain frame
    {
        Simulation sim(SimulationMode::EngineValvetrain);
        sim.play();
        // Advance into power stroke combustion
        for (int i = 0; i < 45; ++i) {
            sim.update(0.016f);
        }

        SceneView view;
        view.isEngineMode = true;
        view.powered = true;
        view.crankAngle = sim.crankAngle();
        view.camAngle = sim.camAngle();
        view.intakeLift = sim.intakeValveLift();
        view.exhaustLift = sim.exhaustValveLift();
        view.pistonY = sim.pistonY();
        view.rodAngle = sim.connectingRodAngle();
        view.stroke = sim.strokeNumber();
        view.combustion = sim.isCombustion();
        view.beltTensioned = sim.isBeltTensioned();
        view.collision = sim.isInterferenceCollision();
        view.rpm = sim.engineRpm();
        view.hp = sim.horsepower();

        Framebuffer fb(kSceneWidth, kSceneHeight);
        renderScene(fb, sim.parts(), view);
        const char* name = "lumina_valvetrain_engine.bmp";
        if (fb.saveBMP(name)) std::cout << "Saved " << name << "\n";
        else { std::cerr << "Could not write " << name << "\n"; ok = false; }
    }

    return ok ? 0 : 1;
}
