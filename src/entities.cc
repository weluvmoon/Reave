#include "include/assets.h"
#include "include/behaves.h"
#include "include/character.h"
#include "include/collision.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/enemies.h"
#include "include/entities.h"
#include "include/objects.h"
#include "include/tiles.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

void EntityManager::Reserve(size_t capacity) {
    if (capacity > 1000000)
        return;
    physics.pos.reserve(capacity);
    physics.vel.reserve(capacity);
    physics.siz.reserve(capacity);
    physics.rect.reserve(capacity);
    physics.rectX.reserve(capacity);
    physics.rectY.reserve(capacity);
    physics.mass.reserve(capacity);
    physics.gravity.reserve(capacity);
    physics.active.reserve(capacity);
    physics.initialized.reserve(capacity);
    physics.collide.reserve(capacity);
    physics.grounded.reserve(capacity);
    physics.walled.reserve(capacity);

    rendering.col.reserve(capacity);
    rendering.varID.reserve(capacity);
    rendering.typeID.reserve(capacity);
    rendering.rotation.reserve(capacity);
    rendering.scale.reserve(capacity);
    rendering.texDraw.reserve(capacity);
    rendering.frameNum.reserve(capacity);
    rendering.rowIndex.reserve(capacity);
    rendering.frameSpd.reserve(capacity);
    rendering.frameMin.reserve(capacity);
    rendering.frameMax.reserve(capacity);

    stats.health.reserve(capacity);
    stats.maxHealth.reserve(capacity);

    vars.reserve(capacity);
    behs.reserve(capacity);
}

size_t EntityManager::AddEntity(int typeID, int varID, Vector2 pos, Vector2 siz,
                                float gravity, Color col) {
    physics.pos.push_back(pos);
    physics.vel.push_back({0, 0});
    physics.siz.push_back(siz);
    physics.mass.push_back(1.0f);
    physics.gravity.push_back(gravity);
    physics.active.push_back(true);
    physics.initialized.push_back(false);
    physics.collide.push_back(true);
    physics.grounded.push_back(false);
    physics.walled.push_back(false);
    physics.rect.push_back({pos.x, pos.y, siz.x, siz.y}); // Initialize rect
    physics.rectX.push_back({0, 0, 0, 0});
    physics.rectY.push_back({0, 0, 0, 0});

    rendering.varID.push_back(varID);
    rendering.typeID.push_back(typeID);
    rendering.col.push_back(col);
    rendering.rotation.push_back(0.0f);
    rendering.scale.push_back(0.0f);
    rendering.texDraw.push_back(false);
    rendering.frameNum.push_back(0);
    rendering.rowIndex.push_back(0);
    rendering.frameMin.push_back(0);
    rendering.frameMax.push_back(0);
    rendering.frameSpd.push_back(1.0f);

    stats.health.push_back(100.0f);
    stats.maxHealth.push_back(100.0f);

    vars.push_back(EntityVars());
    behs.push_back(EntityBehaves());

    TraceLog(LOG_INFO, "ADDING ENTITY: [%s] Gravity: %.2f",
             IdToName[typeID].c_str(), gravity);

    return physics.pos.size() - 1;
}

size_t EntityManager::AddEntityJ(std::string typeName, Vector2 pos) {
    auto it = ConfigMap.find(typeName);
    if (it == ConfigMap.end()) {
        TraceLog(LOG_ERROR, "Type [%s] not found!", typeName.c_str());
        return 0;
    }

    EntityConfig &cfg = it->second;

    // --- PHYSICS (Must match PhysicsComponent struct exactly) ---
    physics.pos.push_back(pos);
    physics.vel.push_back({0, 0});
    physics.siz.push_back(cfg.size);
    physics.rect.push_back({pos.x, pos.y, cfg.size.x, cfg.size.y});
    physics.rectX.push_back({0, 0, 0, 0});
    physics.rectY.push_back({0, 0, 0, 0});
    physics.mass.push_back(1.0f);
    physics.gravity.push_back(cfg.gravity);
    physics.active.push_back(true);
    physics.initialized.push_back(false);
    physics.collide.push_back(cfg.canCollide);
    physics.grounded.push_back(false);
    physics.walled.push_back(false);

    // --- RENDERING (Must match RenderComponent struct exactly) ---
    rendering.varID.push_back(cfg.vID);
    rendering.typeID.push_back(cfg.tID);
    rendering.col.push_back(cfg.color);

    rendering.rotation.push_back(0.0f);
    rendering.scale.push_back(cfg.scale);
    rendering.texDraw.push_back(cfg.texDraw);
    rendering.frameNum.push_back(cfg.frameNum);
    rendering.rowIndex.push_back(cfg.rowIndex);
    rendering.frameMin.push_back(cfg.frameMin);
    rendering.frameMax.push_back(cfg.frameMax);
    rendering.frameSpd.push_back(cfg.frameSpeed);

    // --- STATS & VARS ---
    stats.health.push_back(cfg.health);
    stats.maxHealth.push_back(cfg.health);

    EntityVars newVars;
    for (auto const &[key, val] : cfg.customVars) {
        newVars.values[key] = val;
    }
    vars.push_back(newVars);

    EntityBehaves newBehs;
    for (auto const &[key, val] : cfg.customBehs) {
        newBehs.values[key] = val;
    }
    behs.push_back(newBehs);

    TraceLog(LOG_INFO, "ADDING ENTITY: [%s] Gravity: %.2f", typeName.c_str(),
             cfg.gravity);
    return physics.pos.size() - 1;
}

void EntityManager::LoadConfigs(const std::string &path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        TraceLog(LOG_ERROR, "FILEIO: Could not open config at [%s]",
                 path.c_str());
        return;
    }

    try {
        nlohmann::json data;
        file >> data;

        if (data.is_null() || data.empty()) {
            TraceLog(LOG_ERROR, "FILEIO: Config file is empty or invalid! [%s]",
                     path.c_str());
            return;
        }

        // 1. Reset state
        EntityRegistry.clear();
        IdToName.clear();
        ConfigMap.clear();

        // 2. Optional: Pre-load the Registry if entity_types exists
        if (data.contains("entity_types") && data["entity_types"].is_array()) {
            for (const auto &item : data["entity_types"]) {
                std::string name = item.value("name", "UNKNOWN");
                int id = item.value("id", 0);
                EntityRegistry[name] = id;
                IdToName[id] = name;
            }
        }

        // 3. Process every object in the JSON
        for (auto &[name, configData] : data.items()) {
            if (name == "entity_types" || !configData.is_object())
                continue;

            EntityConfig cfg;
            cfg.from_json(
                configData); // This already fills customVars and customBehs

            if (EntityRegistry.find(name) == EntityRegistry.end()) {
                EntityRegistry[name] = cfg.tID;
                IdToName[cfg.tID] = name;
                TraceLog(LOG_INFO, "FILEIO: Self-registered [%s] with ID %d",
                         name.c_str(), cfg.tID);
            } else {
                cfg.tID = EntityRegistry[name];
            }

            ConfigMap[name] = cfg;

            // Parse customVars safely
            if (configData.contains("customVars") &&
                configData["customVars"].is_object()) {
                for (auto &[key, value] : configData["customVars"].items()) {
                    if (value.is_number()) {
                        cfg.customVars[key] = value.get<float>();
                    }
                }
            }

            if (configData.contains("behaviors") &&
                configData["behaviors"].is_object()) {
                for (auto &[key, value] : configData["behaviors"].items()) {
                    if (value.is_number()) {
                        cfg.customBehs[key] = value.get<float>();
                    }
                }
            }

            ConfigMap[name] = cfg;
        }

        LoadTypes();

        for (auto const &[name, cfg] : ConfigMap) {
            TraceLog(LOG_INFO, "Loaded: [%s] ID: %d Gravity: %.2f",
                     name.c_str(), cfg.tID, cfg.gravity);
        }

        TraceLog(LOG_INFO, "FILEIO: Successfully loaded %zu entities from [%s]",
                 ConfigMap.size(), path.c_str());

    } catch (const nlohmann::json::parse_error &e) {
        TraceLog(LOG_ERROR, "JSON PARSE ERROR in %s: %s", path.c_str(),
                 e.what());
    } catch (const std::exception &e) {
        TraceLog(LOG_ERROR, "GENERAL ERROR loading %s: %s", path.c_str(),
                 e.what());
    }
}

void EntityManager::UpdateAll(float dt) {
    for (size_t i = 0; i < physics.pos.size(); ++i) {
        if (!physics.active[i] || rendering.typeID[i] == EntityTys::TYTILE)
            continue;

        if (!physics.grounded[i]) {
            physics.vel[i].y += physics.gravity[i];
        } else if (physics.vel[i].y > 0.0f) {
            physics.vel[i].y = 0.0f;
        }
        physics.grounded[i] = false;

        physics.pos[i].x += physics.vel[i].x * dt;
        cS.ResolveAxis(*this, i, true);

        physics.pos[i].y += physics.vel[i].y * dt;
        cS.ResolveAxis(*this, i, false);
        SyncRect(*this, i);

        stats.health[i] = std::clamp(stats.health[i], 0.0f, stats.maxHealth[i]);

        if (stats.health[i] <= 0.0f) {
            this->FastRemove(i);
        }
    }
}

void EntityManager::DrawAll(Camera2D camera) {
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);
    Rectangle view = {topLeft.x, topLeft.y, bottomRight.x - topLeft.x,
                      bottomRight.y - topLeft.y};

    for (size_t i = 0; i < physics.pos.size(); ++i) {
        if (!physics.active[i])
            continue;

        const Rectangle &r = physics.rect[i];

        // Frustum Culling Check
        if (r.x + r.width > view.x && r.x < view.x + view.width &&
            r.y + r.height > view.y && r.y < view.y + view.height) {

            Texture2D pixelTex = am.textures[TEX_DEF];
            DrawTexturePro(pixelTex, {0, 0, 1, 1}, r, {0, 0}, 0.0f,
                           rendering.col[i]);
        }
    }
}

void EntityManager::FastRemove(size_t index) {
    size_t last = physics.pos.size() - 1;

    // Only swap if the element we are removing is NOT the last one
    if (index < last) {
        physics.pos[index] = physics.pos[last];
        physics.vel[index] = physics.vel[last];
        physics.siz[index] = physics.siz[last];
        physics.rect[index] = physics.rect[last];
        physics.rectX[index] = physics.rectX[last];
        physics.rectY[index] = physics.rectY[last];
        physics.gravity[index] = physics.gravity[last];
        physics.active[index] = physics.active[last];
        physics.initialized[index] = physics.initialized[last];
        physics.collide[index] = physics.collide[last];
        physics.grounded[index] = physics.grounded[last];
        physics.walled[index] = physics.walled[last];

        rendering.varID[index] = rendering.varID[last];
        rendering.typeID[index] = rendering.typeID[last];
        rendering.col[index] = rendering.col[last];
        rendering.rotation[index] = rendering.rotation[last];
        rendering.scale[index] = rendering.scale[last];
        rendering.texDraw[index] = rendering.texDraw[last];
        rendering.frameNum[index] = rendering.frameNum[last];
        rendering.rowIndex[index] = rendering.rowIndex[last];
        rendering.frameSpd[index] = rendering.frameSpd[last];
        rendering.frameMin[index] = rendering.frameMin[last];
        rendering.frameMax[index] = rendering.frameMax[last];

        stats.health[index] = stats.health[last];
        stats.maxHealth[index] = stats.maxHealth[last];

        // Ensure variables are swapped to the new index
        vars[index] = vars[last];
        behs[index] = behs[last];
    }

    // Ridiculous Speed: Pop all vectors to keep them perfectly aligned
    physics.pos.pop_back();
    physics.vel.pop_back();
    physics.siz.pop_back();
    physics.rect.pop_back();
    physics.rectX.pop_back();
    physics.rectY.pop_back();
    physics.gravity.pop_back();
    physics.active.pop_back();
    physics.initialized.pop_back();
    physics.collide.pop_back();
    physics.grounded.pop_back();
    physics.walled.pop_back();

    rendering.varID.pop_back();
    rendering.typeID.pop_back();
    rendering.col.pop_back();
    rendering.rotation.pop_back();
    rendering.scale.pop_back();
    rendering.texDraw.pop_back();
    rendering.frameNum.pop_back();
    rendering.rowIndex.pop_back();
    rendering.frameSpd.pop_back();
    rendering.frameMin.pop_back();
    rendering.frameMax.pop_back();

    stats.health.pop_back();
    stats.maxHealth.pop_back();

    vars.pop_back();
    behs.pop_back();
}

void EntityManager::ClearAll() {
    physics.pos.clear();
    physics.vel.clear();
    physics.siz.clear();
    physics.rect.clear();
    physics.rectX.clear();
    physics.rectY.clear();
    physics.gravity.clear();
    physics.active.clear();
    physics.initialized.clear();
    physics.collide.clear();
    physics.grounded.clear();
    physics.walled.clear();

    rendering.varID.clear();
    rendering.typeID.clear();
    rendering.col.clear();
    rendering.rotation.clear();
    rendering.scale.clear();
    rendering.texDraw.clear();
    rendering.frameNum.clear();
    rendering.rowIndex.clear();
    rendering.frameSpd.clear();
    rendering.frameMin.clear();
    rendering.frameMax.clear();

    stats.health.clear();
    stats.maxHealth.clear();

    vars.clear();
    behs.clear();
}

int EntityManager::GetActiveCount() {
    int count = 0;
    for (bool isActive : physics.active)
        if (isActive)
            count++;
    return count;
}

void EntityManager::SyncRect(EntityManager &em, size_t i) {
    em.physics.rect[i] = {em.physics.pos[i].x, em.physics.pos[i].y,
                          em.physics.siz[i].x, em.physics.siz[i].y};

    em.physics.rectX[i] = {em.physics.pos[i].x,
                           em.physics.pos[i].y + em.physics.siz[i].y * 0.5f -
                               1.0f,
                           em.physics.siz[i].x, 2.0f};

    em.physics.rectY[i] = {em.physics.pos[i].x + em.physics.siz[i].x * 0.5f -
                               1.0f,
                           em.physics.pos[i].y, 2.0f, em.physics.siz[i].y};
}

bool EntityManager::SaveLevel(const std::string &filename) {
    nlohmann::json save;
    nlohmann::json entitiesArray = nlohmann::json::array();
    size_t count = physics.pos.size();

    for (size_t i = 0; i < count; ++i) {
        nlohmann::json entity;

        entity["pos"] = {physics.pos[i].x, physics.pos[i].y};
        entity["size"] = {physics.siz[i].x, physics.siz[i].y};

        entity["gravity"] = physics.gravity[i];

        entity["typeID"] = rendering.typeID[i];
        entity["varID"] = rendering.varID[i];

        entity["color"] = {rendering.col[i].r, rendering.col[i].g,
                           rendering.col[i].b, rendering.col[i].a};

        entity["health"] = stats.health[i];
        entity["maxHealth"] = stats.maxHealth[i];

        if (!vars[i].values.empty()) {
            entity["vars"] = vars[i].values;
        }
        if (!behs[i].values.empty()) {
            entity["behs"] = behs[i].values;
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

bool EntityManager::LoadLevel(const std::string &filename) {
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

    ClearAll(); // Wipe current state

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
        size_t index = AddEntity(tID, vID, pos, siz, grav, col);

        // 4. Restore Stats & Maps (uses .get<> to map JSON object back to
        // std::unordered_map)
        stats.health[index] = hp;
        stats.maxHealth[index] = maxHp;

        if (entityJson.contains("vars"))
            vars[index].values =
                entityJson["vars"]
                    .get<std::unordered_map<std::string, float>>();

        if (entityJson.contains("behs"))
            behs[index].values =
                entityJson["behs"]
                    .get<std::unordered_map<std::string, float>>();
    }

    TraceLog(LOG_INFO, "FILEIO: Level [%s] loaded successfully.",
             filename.c_str());
    return true;
}

// Management Functions
void EntitySystem(EntityManager &em) {
    for (size_t i = 0; i < em.rendering.typeID.size(); ++i) {
        TileSystem(em, i);
        ObjectSystem(em, i);
        CharacterSystem(em, i);
        EnemySystem(em, i);

        BehaveSystem(em, i);
    }
}

void EntityDrawing(EntityManager &em) {
    for (size_t i = 0; i < em.rendering.typeID.size(); ++i) {
		ObjectDrawing(em, i);
        CharacterDrawing(em, i);
        EnemyDrawing(em, i);
        
        BehaveDrawing(em, i);
    }
}

