#include "include/behaves.h"
#include "include/collision.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include <raylib.h>

void BehaveSystem(EntityManager &em, size_t i) {
    // 1. Identify the entity and its configuration
    int id = em.rendering.typeID[i];
    auto nameIt = IdToName.find(id);
    if (nameIt == IdToName.end())
        return;

    auto cfgIt = em.ConfigMap.find(nameIt->second);
    if (cfgIt == em.ConfigMap.end())
        return;

    const EntityConfig &cfg = cfgIt->second;

    auto &v = em.vars[i];
    auto &b = em.behs[i];

    for (auto const &[behavior, behaviorValue] : cfg.customBehs) {

        // --- Tile Behaviors ---
        if (behavior == "one_way") {
            if (em.rendering.varID[i] == 0) {
            } else if (em.rendering.varID[i] == 1) {
            } else if (em.rendering.varID[i] == 2) {
            } else if (em.rendering.varID[i] == 3) {
            }
        }

        // --- Enemy Behaviors ---
        if (behavior == "flip_on_wall") {
            if (em.physics.walled[i]) {
                em.physics.vel[i].x *= -1.0f;
                em.SyncRect(em, i);
            }
        }

        if (behavior == "chase_player") {
            float chaseSize = 400.0f * em.rendering.scale[i];
            Rectangle chaseRect = {em.physics.pos[i].x - chaseSize / 2,
                                   em.physics.pos[i].y - chaseSize / 2,
                                   chaseSize, chaseSize};

            auto colRes = cS.CheckCollisions(em, chaseRect, i);

            if (colRes.typeID != EntityTys::TYCHARACTER)
                return;

            if (em.physics.pos[i].x > colRes.pos.x) {
                em.physics.vel[i].x = v.get("MAX_SPEED");
            } else if (em.physics.pos[i].x < colRes.pos.x) {
                em.physics.vel[i].x = -v.get("MAX_SPEED");
            }
        }

        if (behavior == "jump_on_ground") {
            if (em.physics.grounded[i]) {
                if (v.has("JUMP_VAR")) {
                    float jumpVel = v.get("JUMP_VAR");
                    em.physics.vel[i].y = jumpVel;
                } else if (cfg.customVars.count("JUMP_VAR")) {
                    em.physics.vel[i].y = cfg.customVars.at("JUMP_VAR");
                }
            }
        }
    }
}
