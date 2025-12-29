#pragma once

#include "raylib.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "functions.h"

// Define unique IDs for each entity type
enum class EntityType : int {
    TYBASE,
    TYDUMMY,
    TYSOLID
    // Add other types
};

class Entity {
  public:
    Entity(int id, Vector2 pos, Vector2 siz, Vector2 vel) {
        this->id = id;
        this->pos = pos;
        this->siz = siz;
        this->vel = vel;

        s_instanceCount++;

        // Ground position
        rect = {pos.x, pos.y, siz.x, siz.y};
    }

    ~Entity() { UnloadTexture(texture); }

    bool removed = false, locked = false;
    bool toDraw = false, alwaysDraw = false;
    int id;
    Vector2 pos, vel, siz;
    Rectangle rect;
    Color col;
    Texture2D texture;

    float scale = 1.0f, mass = 10.0f, grav = 1000.0f;
    float health, healthMax;
    float frameNum = 0, rowNum = 0;

    void UpdateFrame(float min, float max, float ac, float row);
    void DrawSpriteRow(Texture2D texture, int frameX, int rowIndex, int columns,
                       int rows, Vector2 position, float scale, float rotation,
                       bool flipX);
    void DrawShadow();
    void DrawRect();

    virtual EntityType getType() const = 0;
    virtual int getID() { return id; }

    virtual void Update(float dt);
    virtual void Draw();

    int GetId(Entity &e);
    void SetId(Entity &e, int id);

    void UpdateVariables(float dt);

    bool ChecKCol(Entity *a, Entity *b);
    virtual void CheckCollisions();
    virtual void ResolveCollisionsX(Entity &otherEnt);
    virtual void ResolveCollisionsY(Entity &otherEnt);

    virtual void MoveTo(float dt, Vector2 targ, float ac, float max);
    virtual void MoveAway(float dt, Vector2 targ, float ac, float max);

    virtual void ManageHealth();
    void SetHealth(float amnt);
    void IncreaseHealth(float amnt);
    void DecreaseHealth(float amnt);
    void SetHealthMax(float amnt);
    void IncreaseHealthMax(float amnt);
    void DecreaseHealthMax(float amnt);

    Rectangle GetRect() const { return {pos.x, pos.y, siz.x, siz.y}; }

    static int getInstanceCount();

    // Virtual function to save object state
    virtual void Save(std::ofstream &os) const {
        // Always write the type ID first in the base class implementation
        unsigned int type = static_cast<unsigned int>(getType());
        os.write(reinterpret_cast<const char *>(&type), sizeof(unsigned int));
        os.write(reinterpret_cast<const char *>(&id), sizeof(unsigned int));
        os.write(reinterpret_cast<const char *>(&pos.x), sizeof(float));
        os.write(reinterpret_cast<const char *>(&pos.y), sizeof(float));
        os.write(reinterpret_cast<const char *>(&siz.x), sizeof(float));
        os.write(reinterpret_cast<const char *>(&siz.y), sizeof(float));
        os.write(reinterpret_cast<const char *>(&col), sizeof(col));
        os.write(reinterpret_cast<const char *>(&health), sizeof(unsigned int));
    }

    // Virtual function to load object state
    virtual void Load(std::ifstream &is) {
        is.read(reinterpret_cast<char *>(&id), sizeof(unsigned int));
        is.read(reinterpret_cast<char *>(&pos.x), sizeof(float));
        is.read(reinterpret_cast<char *>(&pos.y), sizeof(float));
        is.read(reinterpret_cast<char *>(&siz.x), sizeof(float));
        is.read(reinterpret_cast<char *>(&siz), sizeof(float));
        is.read(reinterpret_cast<char *>(&col), sizeof(col));
        is.read(reinterpret_cast<char *>(&health), sizeof(unsigned int));
    }

  private:
    static int s_instanceCount;
};
