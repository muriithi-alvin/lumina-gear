#include "lumina/level.h"

#if __has_include("lumina/levels/level1.h")
#include "lumina/levels/level1.h"
#define LUMINA_HAS_LEVEL1 1
#endif

#if __has_include("lumina/levels/level2.h")
#include "lumina/levels/level2.h"
#define LUMINA_HAS_LEVEL2 1
#endif

#if __has_include("lumina/levels/level3.h")
#include "lumina/levels/level3.h"
#define LUMINA_HAS_LEVEL3 1
#endif

#if __has_include("lumina/levels/level4.h")
#include "lumina/levels/level4.h"
#define LUMINA_HAS_LEVEL4 1
#endif

#if __has_include("lumina/levels/level5.h")
#include "lumina/levels/level5.h"
#define LUMINA_HAS_LEVEL5 1
#endif

namespace lumina {

std::unique_ptr<Level> createLevel(int levelNumber) {
    switch (levelNumber) {
#ifdef LUMINA_HAS_LEVEL1
        case 1: return std::make_unique<Level1>();
#endif
#ifdef LUMINA_HAS_LEVEL2
        case 2: return std::make_unique<Level2>();
#endif
#ifdef LUMINA_HAS_LEVEL3
        case 3: return std::make_unique<Level3>();
#endif
#ifdef LUMINA_HAS_LEVEL4
        case 4: return std::make_unique<Level4>();
#endif
#ifdef LUMINA_HAS_LEVEL5
        case 5: return std::make_unique<Level5>();
#endif
        default:
#ifdef LUMINA_HAS_LEVEL1
            return std::make_unique<Level1>();
#else
            return nullptr;
#endif
    }
}

}  // namespace lumina
