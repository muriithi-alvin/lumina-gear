// scene.h - the CP1 demo scene that puts every component together.
#pragma once
#include "lumina/framebuffer.h"

namespace lumina {

constexpr int kSceneWidth = 1000;
constexpr int kSceneHeight = 600;

// powered = true: the bulb is lit and the room is brighter.
void renderScene(Framebuffer& fb, bool powered);

}  // namespace lumina
