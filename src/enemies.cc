#include "include/enemies.h"
#include "include/assets.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include "raylib.h"
#include <cstddef>

void EnemySystem(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;

    if (!em.physics.initialized[i]) {
        if (v.count("MAX_SPEED")) {
            em.physics.vel[i].x = v.at("MAX_SPEED");
        } else {
            em.physics.vel[i].x = 100.0f;
        }
        em.physics.initialized[i] = true;
        return;
    }
}

void EnemyDrawing(EntityManager &em, size_t i) {}
