#include "include/data.h"
#include "include/entities.h"
#include "include/tiles.h"
#include <raylib.h>

void TileSystem(EntityManager &em, size_t i) {
    if (em.rendering.typeID[i] != EntityTys::TYTILE)
        return;

    if (!em.physics.initialized[i]) {
        em.physics.gravity[i] = 0;
        em.stats.maxHealth[i] = 100.0f;

        em.physics.initialized[i] = true;
    }
}
