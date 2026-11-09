#include <cmath>
#include <iostream>
#include "lumina/scene.h"
#include "lumina/simulation.h"
#include "lumina/transform.h"

using namespace lumina;

static int failures = 0;
#define CHECK(cond)                                                                      \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            std::cerr << "FAIL line " << __LINE__ << ": " #cond "\n";                    \
            ++failures;                                                                  \
        }                                                                                \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                            \
    do {                                                                                 \
        if (std::abs((a) - (b)) > (eps)) {                                              \
            std::cerr << "FAIL line " << __LINE__ << ": " #a " (" << (a)                \
                      << ") != " #b " (" << (b) << ")\n";                                \
            ++failures;                                                                  \
        }                                                                                \
    } while (0)

static void testTransforms() {
    const Mat3 id = mat3Identity();
    const Vec2 p0{10.0f, 25.0f};
    const Vec2 p0_t = mat3TransformPoint(id, p0);
    CHECK_NEAR(p0_t.x, 10.0f, 1e-4f);
    CHECK_NEAR(p0_t.y, 25.0f, 1e-4f);

    const Mat3 tr = mat3Translate(15.0f, -8.0f);
    const Vec2 p1 = mat3TransformPoint(tr, p0);
    CHECK_NEAR(p1.x, 25.0f, 1e-4f);
    CHECK_NEAR(p1.y, 17.0f, 1e-4f);

    const float kHalfPi = 1.57079632679f;
    const Mat3 rot90 = mat3Rotate(kHalfPi);
    const Vec2 unitX{1.0f, 0.0f};
    const Vec2 rotX = mat3TransformPoint(rot90, unitX);
    CHECK_NEAR(rotX.x, 0.0f, 1e-4f);
    CHECK_NEAR(rotX.y, 1.0f, 1e-4f);

    const Mat3 rotPivot = mat3RotateAround(kHalfPi, 10.0f, 10.0f);
    const Vec2 pPivot = mat3TransformPoint(rotPivot, {15.0f, 10.0f});
    CHECK_NEAR(pPivot.x, 10.0f, 1e-4f);
    CHECK_NEAR(pPivot.y, 15.0f, 1e-4f);

    const Mat3 comp = mat3Multiply(tr, rot90);
    const Vec2 compPt = mat3TransformPoint(comp, unitX);
    const Vec2 stepPt = mat3TransformPoint(tr, mat3TransformPoint(rot90, unitX));
    CHECK_NEAR(compPt.x, stepPt.x, 1e-4f);
    CHECK_NEAR(compPt.y, stepPt.y, 1e-4f);
}

static void testKinematicsGearRatios() {
    Simulation sim(SimulationMode::ClassicLumina);
    sim.setTargetMotorSpeed(6.0f);
    sim.play();

    for (int i = 0; i < 60; ++i) {
        sim.update(0.05f);
    }

    const float wMotor = sim.speed(kPartMotor);
    CHECK(wMotor > 5.9f);

    const float wGear1 = sim.speed(kPartGear1);
    CHECK_NEAR(wGear1, wMotor, 1e-3f);

    const float wGear2 = sim.speed(kPartGear2);
    const float expectedGear2 = -wGear1 * (12.0f / 18.0f);
    CHECK_NEAR(wGear2, expectedGear2, 1e-3f);
    CHECK(wGear2 < 0.0f);

    const float wPulley = sim.speed(kPartPulley);
    CHECK_NEAR(wPulley, wGear2, 1e-3f);

    const float wWheel = sim.speed(kPartWheel);
    const float expectedWheel = wPulley * (40.0f / 22.0f);
    CHECK_NEAR(wWheel, expectedWheel, 1e-3f);

    const float netRatio = wWheel / wMotor;
    const float expectedNetRatio = -(12.0f / 18.0f) * (40.0f / 22.0f);
    CHECK_NEAR(netRatio, expectedNetRatio, 1e-3f);

    CHECK(sim.isPowered());
    CHECK(sim.powerLevel() > 0.9f);
}

static void testContinuousRotation() {
    Simulation sim(SimulationMode::ClassicLumina);
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    const float initialGear1 = sim.part(kPartGear1).angle;
    const float initialGear2 = sim.part(kPartGear2).angle;

    for (int i = 0; i < 30; ++i) {
        sim.update(1.0f / 60.0f);
    }

    const float afterGear1 = sim.part(kPartGear1).angle;
    const float afterGear2 = sim.part(kPartGear2).angle;

    CHECK(afterGear1 != initialGear1);
    CHECK(afterGear2 != initialGear2);

    sim.pause();
    for (int i = 0; i < 180; ++i) {
        sim.update(1.0f / 60.0f);
    }

    CHECK_NEAR(sim.motorSpeed(), 0.0f, 1e-2f);
    CHECK_NEAR(sim.speed(kPartWheel), 0.0f, 1e-2f);
    CHECK(!sim.isPowered());
}

static void testDragAndDropTranslation() {
    Simulation sim(SimulationMode::ClassicLumina);
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    for (int i = 0; i < 60; ++i) sim.update(0.05f);
    CHECK(sim.isPowered());
    CHECK(sim.isPartMeshed(kPartGear2));

    sim.setPartPosition(kPartGear2, 600.0f, 330.0f);
    sim.update(0.05f);

    CHECK(!sim.isPartMeshed(kPartGear2));
    CHECK_NEAR(sim.speed(kPartGear2), 0.0f, 1e-4f);
    CHECK_NEAR(sim.speed(kPartWheel), 0.0f, 1e-4f);
    CHECK(!sim.isPowered());
    CHECK_NEAR(sim.powerLevel(), 0.0f, 1e-4f);

    const float g1x = sim.part(kPartGear1).x;
    sim.setPartPosition(kPartGear2, g1x + 95.0f, 330.0f);
    sim.snapToMesh(kPartGear2);

    const float actualDist = std::hypot(sim.part(kPartGear2).x - sim.part(kPartGear1).x,
                                        sim.part(kPartGear2).y - sim.part(kPartGear1).y);
    CHECK_NEAR(actualDist, 90.0f, 1e-3f);
    CHECK(sim.isPartMeshed(kPartGear2));

    sim.update(0.05f);
    CHECK(sim.speed(kPartGear2) < -1.0f);
    CHECK(sim.isPowered());
}

static void testDynamicLightingLinkedToPower() {
    Simulation sim(SimulationMode::ClassicLumina);
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    float prevPower = 0.0f;
    for (int i = 0; i < 40; ++i) {
        sim.update(0.02f);
        const float currPower = sim.powerLevel();
        CHECK(currPower >= prevPower);
        prevPower = currPower;
    }
    CHECK(sim.powerLevel() > 0.8f);

    Framebuffer fb(kSceneWidth, kSceneHeight);
    SceneView view;
    view.powered = true;
    view.powerLevel = 0.5f;

    renderCanvas(fb, sim.parts(), view);
    CHECK(fb.getPixel(static_cast<int>(sim.part(kPartBulb).x),
                      static_cast<int>(sim.part(kPartBulb).y)).r > 200);
}

static void testEngineValvetrainKinematics() {
    Simulation sim(SimulationMode::EngineValvetrain);
    CHECK(sim.mode() == SimulationMode::EngineValvetrain);
    CHECK(sim.partCount() == kEnginePartCount);
    CHECK(sim.isBeltTensioned());

    // 1. Sprocket tooth count check: 12T Crank, 24T Cam
    CHECK(sim.part(kEngineCrank).teeth == 12);
    CHECK(sim.part(kEngineCam).teeth == 24);

    // 2. Start engine and verify exact 2:1 camshaft speed ratio
    sim.setTargetMotorSpeed(4.0f);
    sim.play();
    for (int i = 0; i < 60; ++i) sim.update(0.05f);

    const float wCrank = sim.speed(kEngineCrank);
    const float wCam = sim.speed(kEngineCam);
    CHECK(wCrank > 3.9f);
    CHECK_NEAR(wCam, 0.5f * wCrank, 1e-3f);  // Exactly half speed!

    // 3. Piston stroke limits: TDC = 275px, BDC = 351px
    for (int i = 0; i < 120; ++i) {
        sim.update(0.02f);
        const float py = sim.pistonY();
        CHECK(py >= 274.9f && py <= 351.1f);
    }

    // 4. Belt Tensioner Drag detachment
    sim.setPartPosition(kEngineTensioner, 600.0f, 310.0f); // pull tensioner away
    sim.update(0.05f);
    CHECK(!sim.isBeltTensioned());
    CHECK_NEAR(sim.speed(kEngineCam), 0.0f, 1e-4f); // camshaft slips!

    // Drag tensioner back and snap
    sim.setPartPosition(kEngineTensioner, 350.0f, 315.0f);
    sim.snapToMesh(kEngineTensioner);
    CHECK(sim.isBeltTensioned());
    sim.update(0.05f);
    CHECK(sim.speed(kEngineCam) > 1.5f);
}

int main() {
    testTransforms();
    testKinematicsGearRatios();
    testContinuousRotation();
    testDragAndDropTranslation();
    testDynamicLightingLinkedToPower();
    testEngineValvetrainKinematics();

    if (failures == 0) {
        std::cout << "All CP2 tests (including Engine Valvetrain kinematics) passed successfully!\n";
        return 0;
    }
    std::cerr << failures << " test failure(s)\n";
    return 1;
}
