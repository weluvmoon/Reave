#pragma once

#include "entity.h"
#include <memory>
#include <string>
#include <vector>

struct EntityManager {
    std::vector<std::unique_ptr<Entity>> objects;

    // -------- Factory (DECLARATION ONLY) --------
    Entity *EntityFactory(int typeID, int id, Vector2 pos);

    // -------- Entity management --------
    template <typename T, typename... Args> T *AddEntity(Args &&...args) {
        // Create the unique_ptr and get the raw pointer before moving it into
        // the container
        auto entity = std::make_unique<T>(std::forward<Args>(args)...);
        T *raw = entity.get();

        objects.emplace_back(std::move(entity));
        return raw;
    }

    void UpdateAll(float dt);
    void CleanupRemoved();
    void ClearAll();

    void SaveLevel(const std::string &filename);
    void LoadLevel(const std::string &filename);
};

extern EntityManager entities;
