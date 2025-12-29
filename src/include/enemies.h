#pragma once

#include "enemy.h"
#include <raylib.h>

class Walker : public Enemy {
  public:
    Walker(int id, Vector2 pos, Vector2 vel) : Enemy(id, pos, vel) {
        this->siz = Vector2{12, 15};
        this->acel = 75.0f;
        this->dcel = 145.0f;
    }

    float jumpV;

    void Jump();
    void Dash(float s, Vector2 tg), Evade(float s, Vector2 tg);
};
