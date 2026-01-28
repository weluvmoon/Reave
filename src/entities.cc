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
    rendering.frameNum.reserve(capacity);
    rendering.rowIndex.reserve(capacity);
    rendering.frameSpd.reserve(capacity);
    rendering.frameMin.reserve(capacity);
    rendering.frameMax.reserve(capacity);

    stats.health.reserve(capacity);
    stats.maxHealth.reserve(capacity);

    vars.reserve(capacity);
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
    rendering.frameNum.push_back(0);
    rendering.rowIndex.push_back(0);
    rendering.frameMin.push_back(0);
    rendering.frameMax.push_back(0);
    rendering.frameSpd.push_back(1.0f);

    stats.health.push_back(100.0f);
    stats.maxHealth.push_back(100.0f);

    TraceLog(LOG_INFO, "ADDING ENTITY: [%s] Gravity: %.2f",
             IdToName[typeID].c_str(), gravity);
    vars.push_back(EntityVars());
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
    rendering.frameNum.push_back(0);
    rendering.rowIndex.push_back(0);
    rendering.frameMin.push_back(0);
    rendering.frameMax.push_back(0);
    rendering.frameSpd.push_back(cfg.frameSpeed);

    // --- STATS & VARS ---
    stats.health.push_back(cfg.health);
    stats.maxHealth.push_back(cfg.health);

    EntityVars newVars;
    for (auto const &[key, val] : cfg.customVars) {
        newVars.values[key] = val;
    }
    vars.push_back(newVars);

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
            // Skip the registry key itself or non-config metadata
            if (name == "entity_types" || !configData.is_object())
                continue;

            EntityConfig cfg;
            cfg.from_json(
                configData); // Will pick up "tID" from inside the CORE2 block

            if (EntityRegistry.find(name) == EntityRegistry.end()) {
                EntityRegistry[name] = cfg.tID;
                IdToName[cfg.tID] = name;
                TraceLog(LOG_INFO, "FILEIO: Self-registered [%s] with ID %d",
                         name.c_str(), cfg.tID);
            } else {
                cfg.tID = EntityRegistry[name];
            }

            // Parse customVars safely
            if (configData.contains("customVars") &&
                configData["customVars"].is_object()) {
                for (auto &[key, value] : configData["customVars"].items()) {
                    if (value.is_number()) {
                        cfg.customVars[key] = value.get<float>();
                    }
                }
            }

            ConfigMap[name] = cfg;
        }

        LoadTypes();

        for (auto const &[name, cfg] : ConfigMap) {
            TraceLog(LOG_INFO, "Final Config: [%s] ID: %d Gravity: %.2f",
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
        rendering.frameNum[index] = rendering.frameNum[last];
        rendering.rowIndex[index] = rendering.rowIndex[last];
        rendering.frameSpd[index] = rendering.frameSpd[last];
        rendering.frameMin[index] = rendering.frameMin[last];
        rendering.frameMax[index] = rendering.frameMax[last];

        stats.health[index] = stats.health[last];
        stats.maxHealth[index] = stats.maxHealth[last];

        // Ensure variables are swapped to the new index
        vars[index] = vars[last];
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
    rendering.frameNum.pop_back();
    rendering.rowIndex.pop_back();
    rendering.frameSpd.pop_back();
    rendering.frameMin.pop_back();
    rendering.frameMax.pop_back();

    stats.health.pop_back();
    stats.maxHealth.pop_back();
    vars.pop_back();
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
    rendering.frameNum.clear();
    rendering.rowIndex.clear();
    rendering.frameSpd.clear();
    rendering.frameMin.clear();
    rendering.frameMax.clear();

    stats.health.clear();
    stats.maxHealth.clear();

    vars.clear();
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
    std::ofstream outFile(filename, std::ios::binary);
    if (!outFile)
        return false;

    size_t count = physics.pos.size();
    outFile.write(reinterpret_cast<const char *>(&count), sizeof(size_t));

    for (size_t i = 0; i < count; ++i) {
        // Save only essential "Definition" data
        outFile.write(reinterpret_cast<const char *>(&rendering.typeID[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&rendering.varID[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&physics.pos[i]),
                      sizeof(Vector2));
        outFile.write(reinterpret_cast<const char *>(&physics.siz[i]),
                      sizeof(Vector2));
        outFile.write(reinterpret_cast<const char *>(&physics.gravity[i]),
                      sizeof(float));

        outFile.write(reinterpret_cast<const char *>(&rendering.col[i]),
                      sizeof(Color));
        outFile.write(reinterpret_cast<const char *>(&rendering.frameNum[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&rendering.rowIndex[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&rendering.frameMin[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&rendering.frameMax[i]),
                      sizeof(int));
        outFile.write(reinterpret_cast<const char *>(&rendering.frameSpd[i]),
                      sizeof(float));

        outFile.write(reinterpret_cast<const char *>(&stats.health[i]),
                      sizeof(float));
        outFile.write(reinterpret_cast<const char *>(&stats.maxHealth[i]),
                      sizeof(float));

        size_t varCount = vars[i].values.size();
        outFile.write((char *)&varCount, sizeof(size_t));

        for (auto const &[key, val] : vars[i].values) {
            size_t keyLen = key.size();
            outFile.write((char *)&keyLen, sizeof(size_t));
            outFile.write(key.data(), keyLen);
            outFile.write((char *)&val, sizeof(float));
        }
    }

    outFile.close();
    TraceLog(LOG_INFO, "FILEIO: Level saved successfully. Saved %zu entities.",
             count);
    return true;
}

bool EntityManager::LoadLevel(const std::string &filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile)
        return false;

    ClearAll();

    size_t count;
    inFile.read(reinterpret_cast<char *>(&count), sizeof(size_t));

    // Pre-reserve to avoid multiple reallocations
    Reserve(count);

    for (size_t i = 0; i < count; ++i) {
        // Temporary variables to hold loaded data
        int tID, vID;
        Vector2 pos, siz;
        float grav, hp, maxHp;
        Color col;
        int frameNum, rowIndex, frameMax, frameMin;
        float frameSpd;

        // Read in the EXACT same order as SaveLevel
        inFile.read(reinterpret_cast<char *>(&tID), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&vID), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&pos), sizeof(Vector2));
        inFile.read(reinterpret_cast<char *>(&siz), sizeof(Vector2));
        inFile.read(reinterpret_cast<char *>(&grav), sizeof(float));

        inFile.read(reinterpret_cast<char *>(&col), sizeof(Color));
        inFile.read(reinterpret_cast<char *>(&frameNum), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&rowIndex), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&frameMin), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&frameMax), sizeof(int));
        inFile.read(reinterpret_cast<char *>(&frameSpd), sizeof(float));

        inFile.read(reinterpret_cast<char *>(&hp), sizeof(float));
        inFile.read(reinterpret_cast<char *>(&maxHp), sizeof(float));

        size_t index = AddEntity(tID, vID, pos, siz, grav, col);

        size_t varCount = 0;
        if (!inFile.read((char *)&varCount, sizeof(size_t)))
            break;

        for (size_t v = 0; v < varCount; ++v) {
            size_t keyLen = 0;
            inFile.read((char *)&keyLen, sizeof(size_t));

            // SAFETY CHECK: Prevent the 4.8 exabyte crash
            if (keyLen > 1024) {
                TraceLog(LOG_ERROR,
                         "Invalid Save File: String length too long!");
                return false;
            }

            std::string key(keyLen, ' ');
            inFile.read(&key[0], keyLen); // Read into string buffer

            float val = 0;
            inFile.read((char *)&val, sizeof(float));
            vars[index].values[key] = val;
        }
    }

    inFile.close();
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
