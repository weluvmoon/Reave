#pragma once

#include "entity.h"
#include "raylib.h"

class Enemy : public Entity {
  public:
    Enemy(int id, Vector2 pos, Vector2 vel) : Entity(id, pos, vel) {}

    float acel, dcel;

    void Update(float dt) override;
    void Draw() override;

    void UpdateAI();
};
