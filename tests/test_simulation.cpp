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
    // 1. Identity
    const Mat3 id = mat3Identity();
    const Vec2 p0{10.0f, 25.0f};
    const Vec2 p0_t = mat3TransformPoint(id, p0);
    CHECK_NEAR(p0_t.x, 10.0f, 1e-4f);
    CHECK_NEAR(p0_t.y, 25.0f, 1e-4f);

    // 2. Translation
    const Mat3 tr = mat3Translate(15.0f, -8.0f);
    const Vec2 p1 = mat3TransformPoint(tr, p0);
    CHECK_NEAR(p1.x, 25.0f, 1e-4f);
    CHECK_NEAR(p1.y, 17.0f, 1e-4f);

    // 3. Rotation 90 degrees (pi / 2)
    const float kHalfPi = 1.57079632679f;
    const Mat3 rot90 = mat3Rotate(kHalfPi);
    const Vec2 unitX{1.0f, 0.0f};
    const Vec2 rotX = mat3TransformPoint(rot90, unitX);
    CHECK_NEAR(rotX.x, 0.0f, 1e-4f);
    CHECK_NEAR(rotX.y, 1.0f, 1e-4f);

    // 4. Rotation around pivot (10, 10)
    // Point (15, 10) rotated 90 deg around (10, 10) should be (10, 15)
    const Mat3 rotPivot = mat3RotateAround(kHalfPi, 10.0f, 10.0f);
    const Vec2 pPivot = mat3TransformPoint(rotPivot, {15.0f, 10.0f});
    CHECK_NEAR(pPivot.x, 10.0f, 1e-4f);
    CHECK_NEAR(pPivot.y, 15.0f, 1e-4f);

    // 5. Matrix multiplication associativity
    // (T * R) * p == T * (R * p)
    const Mat3 comp = mat3Multiply(tr, rot90);
    const Vec2 compPt = mat3TransformPoint(comp, unitX);
    const Vec2 stepPt = mat3TransformPoint(tr, mat3TransformPoint(rot90, unitX));
    CHECK_NEAR(compPt.x, stepPt.x, 1e-4f);
    CHECK_NEAR(compPt.y, stepPt.y, 1e-4f);
}

static void testKinematicsGearRatios() {
    Simulation sim;
    sim.setTargetMotorSpeed(6.0f);   // 6 rad/s
    sim.play();

    // Fast-forward acceleration until speed stabilizes
    for (int i = 0; i < 60; ++i) {
        sim.update(0.05f);
    }

    const float wMotor = sim.speed(kPartMotor);
    CHECK(wMotor > 5.9f);

    // Motor to Gear 1 (direct shaft): same speed
    const float wGear1 = sim.speed(kPartGear1);
    CHECK_NEAR(wGear1, wMotor, 1e-3f);

    // Gear 1 (12 teeth) to Gear 2 (18 teeth):
    // Ratio = -12 / 18 = -2/3. Must rotate in OPPOSITE direction!
    const float wGear2 = sim.speed(kPartGear2);
    const float expectedGear2 = -wGear1 * (12.0f / 18.0f);
    CHECK_NEAR(wGear2, expectedGear2, 1e-3f);
    CHECK(wGear2 < 0.0f);   // Negative = opposite direction

    // Gear 2 to Pulley (rigid shaft): same speed
    const float wPulley = sim.speed(kPartPulley);
    CHECK_NEAR(wPulley, wGear2, 1e-3f);

    // Pulley (r=40) to Wheel (r=22) via belt:
    // Ratio = 40 / 22. Same direction as Pulley!
    const float wWheel = sim.speed(kPartWheel);
    const float expectedWheel = wPulley * (40.0f / 22.0f);
    CHECK_NEAR(wWheel, expectedWheel, 1e-3f);

    // Overall ratio check: Wheel speed vs Motor speed
    const float netRatio = wWheel / wMotor;
    const float expectedNetRatio = -(12.0f / 18.0f) * (40.0f / 22.0f);
    CHECK_NEAR(netRatio, expectedNetRatio, 1e-3f);

    // Generator powers bulb
    CHECK(sim.isPowered());
    CHECK(sim.powerLevel() > 0.9f);
}

static void testContinuousRotation() {
    Simulation sim;
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    // Initial angles
    const float initialGear1 = sim.part(kPartGear1).angle;
    const float initialGear2 = sim.part(kPartGear2).angle;

    // Run for 0.5s
    for (int i = 0; i < 30; ++i) {
        sim.update(1.0f / 60.0f);
    }

    const float afterGear1 = sim.part(kPartGear1).angle;
    const float afterGear2 = sim.part(kPartGear2).angle;

    // Angles must have changed
    CHECK(afterGear1 != initialGear1);
    CHECK(afterGear2 != initialGear2);

    // Pause and let inertia slow it down
    sim.pause();
    for (int i = 0; i < 180; ++i) {
        sim.update(1.0f / 60.0f);
    }

    // Now speed should be 0
    CHECK_NEAR(sim.motorSpeed(), 0.0f, 1e-2f);
    CHECK_NEAR(sim.speed(kPartWheel), 0.0f, 1e-2f);
    CHECK(!sim.isPowered());
}

static void testDragAndDropTranslation() {
    Simulation sim;
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    for (int i = 0; i < 60; ++i) sim.update(0.05f);
    CHECK(sim.isPowered());
    CHECK(sim.isPartMeshed(kPartGear2));

    // 1. Drag Gear 2 far away
    sim.setPartPosition(kPartGear2, 600.0f, 330.0f);
    sim.update(0.05f);

    // Disconnected: Gear 2 stops, downstream stops, bulb unpowered
    CHECK(!sim.isPartMeshed(kPartGear2));
    CHECK_NEAR(sim.speed(kPartGear2), 0.0f, 1e-4f);
    CHECK_NEAR(sim.speed(kPartWheel), 0.0f, 1e-4f);
    CHECK(!sim.isPowered());
    CHECK_NEAR(sim.powerLevel(), 0.0f, 1e-4f);

    // 2. Drag Gear 2 back near Gear 1 (within snap threshold)
    const float g1x = sim.part(kPartGear1).x;
    sim.setPartPosition(kPartGear2, g1x + 95.0f, 330.0f);
    sim.snapToMesh(kPartGear2);

    // Verify distance snapped to exactly 90.0 (36 + 54)
    const float actualDist = std::hypot(sim.part(kPartGear2).x - sim.part(kPartGear1).x,
                                        sim.part(kPartGear2).y - sim.part(kPartGear1).y);
    CHECK_NEAR(actualDist, 90.0f, 1e-3f);
    CHECK(sim.isPartMeshed(kPartGear2));

    // Gear 2 receives power again
    sim.update(0.05f);
    CHECK(sim.speed(kPartGear2) < -1.0f);
    CHECK(sim.isPowered());
}

static void testDynamicLightingLinkedToPower() {
    Simulation sim;
    sim.setTargetMotorSpeed(4.0f);
    sim.play();

    // As motor accelerates, power increases smoothly
    float prevPower = 0.0f;
    for (int i = 0; i < 40; ++i) {
        sim.update(0.02f);
        const float currPower = sim.powerLevel();
        CHECK(currPower >= prevPower);   // monotonically increasing during spin-up
        prevPower = currPower;
    }
    CHECK(sim.powerLevel() > 0.8f);

    // Canvas render with dynamic power level
    Framebuffer fb(kSceneWidth, kSceneHeight);
    SceneView view;
    view.powered = true;
    view.powerLevel = 0.5f;   // 50% power

    renderCanvas(fb, sim.parts(), view);
    // At 50% power, bulb is lit, glow is active
    CHECK(fb.getPixel(static_cast<int>(sim.part(kPartBulb).x),
                      static_cast<int>(sim.part(kPartBulb).y)).r > 200);
}

int main() {
    testTransforms();
    testKinematicsGearRatios();
    testContinuousRotation();
    testDragAndDropTranslation();
    testDynamicLightingLinkedToPower();

    if (failures == 0) {
        std::cout << "All CP2 tests passed successfully!\n";
        return 0;
    }
    std::cerr << failures << " test failure(s)\n";
    return 1;
}
