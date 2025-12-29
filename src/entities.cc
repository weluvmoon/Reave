#include <algorithm>
#include <fstream>
#include <memory>
#include <raylib.h>

#include "module/entities.h"

// --------------------------------------------------
// Factory
// --------------------------------------------------
Entity *EntityManager::EntityFactory(int typeID, int id, Vector2 pos) {
    switch (typeID) {
    case 1:
        // return AddEntity<Solid>(id, pos, Vector2{25,25}, BLACK);
    case 2:
    // return AddEntity<Character>(id, pos, BLACK);
    case 3:
    // return AddEntity<Walker>(id, pos, Vector2{32,32}, BLUE);
    case 4:
    // return AddEntity<Shooter>(id, pos, Vector2{32,32}, RED);
    default:
        return nullptr;
    }
}

void EntityManager::UpdateAll(float dt) {
    for (auto &e : objects) {
        if (!e || e->removed)
            continue;
        e->Update(dt);
    }
}
void EntityManager::DrawAll() {
    for (auto &e : objects) {
        if (!e || e->removed)
            continue;
        e->Draw();
    }
}

void EntityManager::CleanupRemoved() {
    objects.erase(std::remove_if(objects.begin(), objects.end(),
                                 [](const std::unique_ptr<Entity> &e) {
                                     return !e || e->removed;
                                 }),
                  objects.end());
}

void EntityManager::ClearAll() { objects.clear(); }

void EntityManager::SaveLevel(const std::string &filename) {
    std::ofstream os(filename, std::ios::binary);
    if (!os)
        return;

    size_t count = objects.size();
    os.write(reinterpret_cast<char *>(&count), sizeof(count));

    for (auto &obj : objects) {
        int typeID = static_cast<int>(obj->getType());
        os.write(reinterpret_cast<char *>(&typeID), sizeof(typeID));

        int id = obj->getID();
        os.write(reinterpret_cast<char *>(&id), sizeof(id));
        os.write(reinterpret_cast<char *>(&obj->pos.x), sizeof(float));
        os.write(reinterpret_cast<char *>(&obj->pos.y), sizeof(float));
        os.write(reinterpret_cast<char *>(&obj->siz.x), sizeof(float));
        os.write(reinterpret_cast<char *>(&obj->siz.y), sizeof(float));

        // Optional: save extra data per entity
        obj->Save(os);
    }
}

// --------------------------------------------------
// Load
// --------------------------------------------------
void EntityManager::LoadLevel(const std::string &filename) {
    std::ifstream is(filename, std::ios::binary);
    if (!is)
        return;

    ClearAll();

    uint64_t count; // Use fixed width
    is.read(reinterpret_cast<char *>(&count), sizeof(count));

    for (uint64_t i = 0; i < count; ++i) {
        int32_t typeID, id;
        is.read(reinterpret_cast<char *>(&typeID), sizeof(typeID));
        is.read(reinterpret_cast<char *>(&id), sizeof(id));

        Vector2 pos;
        is.read(reinterpret_cast<char *>(&pos.x), sizeof(float));
        is.read(reinterpret_cast<char *>(&pos.y), sizeof(float));
        Vector2 siz;
        is.read(reinterpret_cast<char *>(&siz.x), sizeof(float));
        is.read(reinterpret_cast<char *>(&siz.y), sizeof(float));

        Entity *obj = EntityFactory(typeID, id, pos);
        if (obj) {
            obj->Load(is);
        }
    }
}
