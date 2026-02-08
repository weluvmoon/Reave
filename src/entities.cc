#include "include/entities.h"
#include "include/assets.h"
#include "include/behaves.h"
#include "include/character.h"
#include "include/collision.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/enemies.h"
#include "include/objects.h"
#include "include/tiles.h"
#include "raylib.h"
#include "raymath.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

void EntityManager::Reserve(size_t capacity) {
    if (capacity > 1000000)
        return;

    physics.Reserve(capacity);
    rendering.Reserve(capacity);
    stats.Reserve(capacity);

    vars.reserve(capacity);
    behs.reserve(capacity);
}

size_t EntityManager::AddEntity(int typeID, int varID, Vector2 pos, Vector2 siz,
                                float gravity, Color col) {
    physics.pos.push_back(pos);
    physics.vel.push_back({0, 0});
    physics.siz.push_back(siz);
    physics.scale.push_back({1.0f, 1.0f});
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
    physics.scale.push_back({1.0f, 1.0f});
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

    Texture2D pixelTex = am.textures[TEX_DEF];

    for (size_t i = 0; i < physics.pos.size(); ++i) {
        if (!physics.active[i])
            continue;
        const Rectangle &r = physics.rect[i];

        if (CheckCollisionRecs(r, view)) {
            float sX = physics.scale[i].x;
            float sY = physics.scale[i].y;
            float rot = rendering.rotation[i];
            Color col = rendering.col[i];
            Vector2 vel = physics.vel[i];
            float speed = Vector2Length(vel);

            Rectangle dest = {r.x + r.width / 2.0f, r.y + r.height,
                              r.width * sX, r.height * sY};
            Vector2 origin = {(r.width * sX) / 2.0f, r.height * sY};

            // --- 2. MASSIVE NEEDLE AFTERIMAGES ---
            // Increased speed threshold for "Power Trails"
            if (speed > 400.0f || sY > 1.3f) {
                Vector2 moveDir = Vector2Normalize(vel);
                float travelAngle = fabsf(moveDir.x);

                for (int j = 1; j <= 5;
                     j++) { // Increased to 5 ghosts for massive length
                    // Spacing: Pushes ghosts further apart at high speed
                    float spacing = (speed * 0.025f) * (float)j;

                    // Base Decay (Shrinks the ghost overall as it gets older)
                    float baseDecay = 1.0f - (j * 0.12f);
                    // Needle Thinning (Aggressively thins the perpendicular
                    // axis)
                    float needleThin = 1.0f - (j * 0.22f);
                    // Massive Expansion (Stretches the parallel axis to look
                    // "Bigger/Longer")
                    float massiveStretch = 1.0f + (j * 0.45f);

                    if (baseDecay < 0.05f)
                        baseDecay = 0.05f;
                    if (needleThin < 0.02f)
                        needleThin = 0.02f;

                    float ghostW = r.width * sX * baseDecay;
                    float ghostH = r.height * sY * baseDecay;

                    // Apply Axis-Aware Needle Sharpening
                    if (travelAngle > 0.5f) {     // Horizontal Movement
                        ghostW *= massiveStretch; // Stretch Width (Longer)
                        ghostH *= needleThin;     // Thin Height (Sharper)
                    } else {                      // Vertical Movement
                        ghostH *= massiveStretch; // Stretch Height (Longer)
                        ghostW *= needleThin;     // Thin Width (Sharper)
                    }

                    Rectangle tDest = {dest.x - (moveDir.x * spacing),
                                       dest.y - (moveDir.y * spacing), ghostW,
                                       ghostH};
                    Vector2 tOrigin = {ghostW / 2.0f, ghostH};

                    // Fade out slower so the long trail is clearly visible
                    DrawTexturePro(pixelTex, {0, 0, 1, 1}, tDest, tOrigin, rot,
                                   Fade(col, 0.45f / j));
                }
            }

            // --- 3. Main Entity ---
            DrawTexturePro(pixelTex, {0, 0, 1, 1}, dest, origin, rot, col);

            // --- 4. Heavy Landing & Screen Shake ---
            if (sY < 0.99f && physics.grounded[i]) {
                if (vars[i].get("AIR_TIME") > 1.0f) {
                    vars[i].set("AIR_TIME", 0.0f);
                }
                // Impact Ring
                DrawCircleV({dest.x, dest.y}, r.width * (1.0f - sY) * 3.0f,
                            Fade(BLACK, 0.4f));
            }
        }
    }
}

void EntityManager::FastRemove(size_t index) {
    physics.Remove(index);
    rendering.Remove(index);
    stats.Remove(index);
}

int EntityManager::GetActiveCount() {
    int count = 0;
    for (bool isActive : physics.active)
        if (isActive)
            count++;
    return count;
}

void EntityManager::SyncRect(EntityManager &em, size_t i) {
    // The main bounding box
    em.physics.rect[i] = {em.physics.pos[i].x, em.physics.pos[i].y,
                          em.physics.siz[i].x, em.physics.siz[i].y};

    // Horizontal Probe (Center line, height of 2px)
    em.physics.rectX[i] = {em.physics.pos[i].x,
                           em.physics.pos[i].y + (em.physics.siz[i].y * 0.5f) -
                               1.0f,
                           em.physics.siz[i].x, 2.0f};

    // Vertical Probe (Center line, width of 2px)
    em.physics.rectY[i] = {em.physics.pos[i].x + (em.physics.siz[i].x * 0.5f) -
                               1.0f,
                           em.physics.pos[i].y, 2.0f, em.physics.siz[i].y};
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
