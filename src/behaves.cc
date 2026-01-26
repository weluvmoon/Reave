#include "include/behaves.h"
#include "include/data.h"
#include "include/entities.h"
#include <raylib.h>

void BehaveSystem(EntityManager &em, size_t i) {
    int id = em.rendering.typeID[i];

    auto nameIt = IdToName.find(id);
    if (nameIt == IdToName.end()) {
        TraceLog(LOG_WARNING, "BEHAVE: Entity index %zu has unknown ID %d", i,
                 id);
        return;
    }

    auto cfgIt = em.ConfigMap.find(nameIt->second);
    if (cfgIt == em.ConfigMap.end())
        return;

    auto &v = em.vars[i].values;
    const std::vector<std::string> &behaviors = cfgIt->second.behaviors;

    for (const std::string &behavior : behaviors) {
        // Tile Behaves
        if (behavior == "one_way") {
            if (em.rendering.varID[i] == 0) {
            } else if (em.rendering.varID[i] == 1) {
            } else if (em.rendering.varID[i] == 2) {
            } else if (em.rendering.varID[i] == 3) {
            }
        }

        // Enenmy Behaves
        if (behavior == "flip_on_wall") {
            if (em.physics.walled[i]) {
                em.physics.vel[i].x *= -1.0f;
                em.SyncRect(em, i);
            }
        }
        if (behavior == "jump_on_ground") {
            if (em.physics.grounded[i]) {
                if (v.count("JUMP_VAR")) {
                    TraceLog(LOG_INFO, "Entity %zu jumping with velocity: %.2f",
                             i, v.at("JUMP_VAR"));
                    em.physics.vel[i].y = v.at("JUMP_VAR");
                } else {
                    TraceLog(LOG_WARNING, "Entity %zu does not have JUMP_VAR!",
                             i);
                }
            }
        }
    }
}
