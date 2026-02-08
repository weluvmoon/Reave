#include "include/level.h"
#include "include/data.h"
#include "include/entities.h"

bool LevelManager::Save(const std::string &filename) {
    nlohmann::json save;
    nlohmann::json entitiesArray = nlohmann::json::array();
    size_t count = em.physics.pos.size();

    for (size_t i = 0; i < count; ++i) {
        nlohmann::json entity;

        entity["pos"] = {em.physics.pos[i].x, em.physics.pos[i].y};
        entity["size"] = {em.physics.siz[i].x, em.physics.siz[i].y};

        entity["gravity"] = em.physics.gravity[i];

        entity["typeID"] = em.rendering.typeID[i];
        entity["varID"] = em.rendering.varID[i];

        entity["color"] = {em.rendering.col[i].r, em.rendering.col[i].g,
                           em.rendering.col[i].b, em.rendering.col[i].a};

        entity["health"] = em.stats.health[i];
        entity["maxHealth"] = em.stats.maxHealth[i];

        if (!em.vars[i].values.empty()) {
            entity["vars"] = em.vars[i].values;
        }
        if (!em.behs[i].values.empty()) {
            entity["behs"] = em.behs[i].values;
        }

        entitiesArray.push_back(entity);
    }

    save["entities"] = entitiesArray;

    std::ofstream outFile(filename);
    if (!outFile.is_open())
        return false;

    outFile << save.dump(4); // Use 4-space indentation for readability
    TraceLog(LOG_INFO,
             "FILEIO: Level saved successfully to %s. Saved %zu entities.",
             filename.c_str(), count);
    return true;
}

bool LevelManager::Load(const std::string &filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open())
        return false;

    nlohmann::json save;
    try {
        inFile >> save;
    } catch (const nlohmann::json::parse_error &e) {
        TraceLog(LOG_ERROR, "JSON PARSE ERROR in %s: %s", filename.c_str(),
                 e.what());
        return false;
    }

    Clear(); // Wipe current state

    // Iterate safely through the "entities" array
    for (auto &entityJson : save["entities"]) {
        // 1. Extract IDs and basic numbers
        int tID = entityJson.value("typeID", 0);
        int vID = entityJson.value("varID", 0);
        float grav = entityJson.value("gravity", 20.0f);
        float hp = entityJson.value("health", 100.0f);
        float maxHp =
            entityJson.value("maxHealth", hp); // Default max to current hp

        // 2. Extract Raylib Types (safely accessing array indices)
        Vector2 pos = {0, 0};
        auto p = entityJson.value("pos", std::vector<float>{0.0f, 0.0f});
        if (p.size() >= 2) {
            pos.x = p[0];
            pos.y = p[1];
        }

        Vector2 siz = {32, 32};
        auto s = entityJson.value("size", std::vector<float>{32.0f, 32.0f});
        if (s.size() >= 2) {
            siz.x = s[0];
            siz.y = s[1];
        }

        Color col = WHITE;
        auto c =
            entityJson.value("color", std::vector<int>{255, 255, 255, 255});
        if (c.size() >= 4) {
            col.r = c[0];
            col.g = c[1];
            col.b = c[2];
            col.a = c[3];
        }

        // 3. Re-create the entity base
        size_t index = em.AddEntity(tID, vID, pos, siz, grav, col);

        // 4. Restore Stats & Maps (uses .get<> to map JSON object back to
        // std::unordered_map)
        em.stats.health[index] = hp;
        em.stats.maxHealth[index] = maxHp;

        if (entityJson.contains("vars"))
            em.vars[index].values =
                entityJson["vars"]
                    .get<std::unordered_map<std::string, float>>();

        if (entityJson.contains("behs"))
            em.behs[index].values =
                entityJson["behs"]
                    .get<std::unordered_map<std::string, float>>();
    }

    TraceLog(LOG_INFO, "FILEIO: Level [%s] loaded successfully.",
             filename.c_str());
    return true;
}

void LevelManager::Clear() {
    em.physics.pos.clear();
    em.physics.vel.clear();
    em.physics.scale.clear();
    em.physics.siz.clear();
    em.physics.rect.clear();
    em.physics.rectX.clear();
    em.physics.rectY.clear();
    em.physics.gravity.clear();
    em.physics.active.clear();
    em.physics.initialized.clear();
    em.physics.collide.clear();
    em.physics.grounded.clear();
    em.physics.walled.clear();

    em.rendering.varID.clear();
    em.rendering.typeID.clear();
    em.rendering.col.clear();
    em.rendering.rotation.clear();
    em.rendering.texDraw.clear();
    em.rendering.frameNum.clear();
    em.rendering.rowIndex.clear();
    em.rendering.frameSpd.clear();
    em.rendering.frameMin.clear();
    em.rendering.frameMax.clear();

    em.stats.health.clear();
    em.stats.maxHealth.clear();

    em.vars.clear();
    em.behs.clear();
}
