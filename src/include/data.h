#pragma once

#include "assets.h"
#include "raylib.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
inline void from_json(const nlohmann::json &j, Vector2 &v) {
    if (j.is_array() && j.size() >= 2) {
        v.x = j.at(0).get<float>();
        v.y = j.at(1).get<float>();
    }
}

inline void from_json(const nlohmann::json &j, Color &c) {
    if (j.is_array() && j.size() >= 4) {
        c.r = (unsigned char)j.at(0).get<int>();
        c.g = (unsigned char)j.at(1).get<int>();
        c.b = (unsigned char)j.at(2).get<int>();
        c.a = (unsigned char)j.at(3).get<int>();
    }
}

inline std::unordered_map<std::string, int> EntityRegistry; // Name -> ID
inline std::unordered_map<int, std::string> IdToName;       // ID -> Name

inline int GetTypeID(
    const std::string &name) { // Use const reference to avoid string copying
    auto it = EntityRegistry.find(name);
    if (it != EntityRegistry.end()) {
        return it->second; // Return the iterator value directly
    }
    return 0;
}

namespace EntityTys {
inline int TYCORE;
inline int TYCHARACTER;
inline int TYWALKER;
inline int TYBOUNCER;
inline int TYSHOOTER;

inline int TYTILE;

inline int TYHITBOX;
inline int TYHURTBOX;
inline int TYBULLET;

inline int TYMOD_START;
} // namespace EntityTys

inline void LoadTypes() {
    EntityTys::TYCORE = GetTypeID("CORE");
    EntityTys::TYCHARACTER = GetTypeID("CHARACTER");
    EntityTys::TYWALKER = GetTypeID("WALKER");
    EntityTys::TYBOUNCER = GetTypeID("BOUNCER");
    EntityTys::TYSHOOTER = GetTypeID("SHOOTER");

    EntityTys::TYTILE = GetTypeID("TILE");

    EntityTys::TYHITBOX = GetTypeID("HITBOX");
    EntityTys::TYHURTBOX = GetTypeID("HURTBOX");
    EntityTys::TYBULLET = GetTypeID("BULLET");

    EntityTys::TYMOD_START = 10000;
}

struct PhysicsComponent {
    std::vector<Vector2> pos;
    std::vector<Vector2> vel;
    std::vector<Vector2> siz;
    std::vector<Vector2> scale;
    std::vector<Rectangle> rect;
    std::vector<Rectangle> rectX;
    std::vector<Rectangle> rectY;
    std::vector<float> mass;
    std::vector<float> gravity;
    std::vector<bool> active;
    std::vector<bool> initialized;
    std::vector<bool> collide;
    std::vector<bool> grounded, walled;
};

struct RenderComponent {
    std::vector<int> varID;
    std::vector<int> typeID;
    std::vector<Color> col;
    std::vector<Texture2D> texture;
    std::vector<float> rotation;

    std::vector<bool> texDraw;
    std::vector<int> frameNum, rowIndex;
    std::vector<int> frameMin, frameMax;
    std::vector<float> frameSpd;
};

struct StatsComponent {
    std::vector<float> health;
    std::vector<float> maxHealth;
};

struct EntityConfig {
    Vector2 size = {32, 32};
    float gravity = 20.0f;
    bool canCollide = true;

    int vID = 0.0f;
    int tID = 0.0f;
    Color color = BLACK;
    Texture2D texture;
    float rotation = 0.0f;
    float scale = 1.0f;

    bool texDraw = false;
    int frameNum = 0.0f, rowIndex = 0.0f;
    int frameMin = 0.0f, frameMax = 0.0f;
    float frameSpeed = 1.0f;

    float health = 100.0f;
    float maxHealth = 100.0f;

    std::map<std::string, float> customVars;
    std::map<std::string, float> customBehs;

    void from_json(const nlohmann::json &j) {
        // Physics
        if (j.contains("size") && j["size"].is_array() &&
            j["size"].size() >= 2) {
            size.x = j["size"][0].get<float>();
            size.y = j["size"][1].get<float>();
        }

        canCollide = j.value("collide", true);

        if (j.contains("gravity") && j["gravity"].is_number()) {
            gravity = j["gravity"].get<float>();
        }

        // Rendering
        tID = j.value("tID", 0);
        vID = j.value("vID", 0);

        if (j.contains("color") && j["color"].is_array() &&
            j["color"].size() >= 4) {
            color.r = j["color"][0].get<unsigned char>();
            color.g = j["color"][1].get<unsigned char>();
            color.b = j["color"][2].get<unsigned char>();
            color.a = j["color"][3].get<unsigned char>();
        }

        texDraw = j.value("texDraw", false);
        if (j.contains("frameNum") && j["frameNum"].is_number()) {
            frameNum = j["frameNum"].get<int>();
        }
        if (j.contains("rowIndex") && j["rowIndex"].is_number()) {
            rowIndex = j["rowIndex"].get<int>();
        }
        if (j.contains("frameMin") && j["frameMin"].is_number()) {
            frameMin = j["frameMin"].get<int>();
        }
        if (j.contains("frameMax") && j["frameMax"].is_number()) {
            frameMax = j["frameMax"].get<int>();
        }

        if (j.contains("frameSpeed") && j["frameSpeed"].is_number()) {
            frameSpeed = j["frameSpeed"].get<float>();
        }

        if (j.contains("frameSpeed") && j["frameSpeed"].is_number()) {
            frameSpeed = j["frameSpeed"].get<float>();
        }

        // Stats
        health = j.value("health", 100.0f);
        maxHealth = j.value("maxHealth",
                            health); // Match max to current if not specified

        // Variables
        if (j.contains("customVars") && j["customVars"].is_object()) {
            for (auto &[key, value] : j["customVars"].items()) {
                if (value.is_number()) {
                    customVars[key] = value.get<float>();
                }
            }
        }

        // Behaviors
        if (j.contains("behaviors") && j["behaviors"].is_object()) {
            for (auto &[key, val] : j["behaviors"].items()) {
                if (val.is_number())
                    customBehs[key] = val.get<float>();
            }
        }
    }
};

struct EntityVars {
    std::unordered_map<std::string, float> values;

    bool has(const std::string &name) {
        return values.find(name) != values.end();
    }

    float get(const std::string &key, float defaultVal = 0.0f) {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : defaultVal;
    }

    void set(const std::string &key, float value) { values[key] = value; }

    void add(const std::string &key, float value) { values[key] += value; }

    void sub(const std::string &key, float value) { values[key] -= value; }

    void mul(const std::string &key, float value) { values[key] *= value; }

    void div(const std::string &key, float value) { values[key] /= value; }
};

struct EntityBehaves {
    std::unordered_map<std::string, float> values;

    bool has(const std::string &name) {
        return values.find(name) != values.end();
    }

    float get(const std::string &key, float defaultVal = 0.0f) {
        auto it = values.find(key);
        return (it != values.end()) ? it->second : defaultVal;
    }

    void set(const std::string &key, float value) { values[key] = value; }
};
