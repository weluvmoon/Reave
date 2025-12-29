#include "include/enemy.h"
#include "include/entities.h"
#include "include/entity.h"

void Enemy::Update(float dt) {
    Entity::Update(dt);
    UpdateAI();
}

void Enemy::Draw() { Entity::Draw(); }

void Enemy::UpdateAI() {}
