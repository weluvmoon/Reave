#pragma once
#include "data.h"
#include "raylib.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <vector>

struct EntityManager {
    PhysicsComponent physics;
    RenderComponent rendering;
    StatsComponent stats;

    std::vector<EntityVars> vars;
    std::vector<EntityBehaves> behs;

    std::unordered_map<std::string, EntityConfig> ConfigMap;

    void Reserve(size_t capacity);
    size_t AddEntity(int typeID, int varID, Vector2 pos, Vector2 siz,
                     float gravity, Color col);
    size_t AddEntityJ(std::string typeName, Vector2 pos);
    void LoadConfigs(const std::string &path);

    void UpdateAll(float dt);
    void DrawAll(Camera2D camera);

    void Compact();
    void FastRemove(size_t index);
    int GetActiveCount();

    void SyncRect(EntityManager &e, size_t i);
};

void EntitySystem(EntityManager &em);
void EntityDrawing(EntityManager &em);

extern EntityManager em;
