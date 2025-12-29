#include "module/entity.h"
#include "raylib.h"
#include "raymath.h"
#include <iostream>

#include "module/entities.h"

int Entity::s_instanceCount = 0;

void Entity::Update(float) {}
void Entity::Draw() { DrawRect(); }

void Entity::UpdateVariables(float dt) {
    vel.y += grav;
    pos += vel * dt;
    rect = {pos.x, pos.y, siz.x, siz.y};

    bool onAnySurface = false;
}

void Entity::CheckCollisions() {
    if (removed)
        return;
}

void Entity::ResolveCollisionsX(Entity &other) {
    if (vel.x > 0 && rect.x < other.rect.x) {
        vel.x = 0;
        pos.x = other.rect.x - rect.width;
    } else if (vel.x < 0 && rect.x > other.rect.x) {
        vel.x = 0;
        pos.x = other.rect.x + other.rect.width;
    }
}

void Entity::ResolveCollisionsY(Entity &other) {
    if (vel.y > 0 && rect.y < other.rect.y) {
        vel.y = 0;
        pos.y = other.rect.y - rect.height;
    } else if (vel.y < 0 && rect.y > other.rect.y) {
        vel.y = 0;
        pos.y = other.rect.y + other.rect.height;
    }
}

bool Entity::ChecKCol(Entity *a, Entity *b) {
    if (!a || !b || a->removed || b->removed) {
        return false;
    }
    return (CheckCollisionRecs(a->rect, b->rect));
}

void Entity::MoveTo(float dt, Vector2 targ, float ac, float max) {
    Vector2 dir = Vector2Normalize(Vector2Subtract(targ, pos));
    vel = Vector2Lerp(vel, dir * max * dt, ac * dt);
}
void Entity::MoveAway(float dt, Vector2 targ, float ac, float max) {
    Vector2 dir = Vector2Normalize(Vector2Subtract(targ, pos));
    vel = Vector2Lerp(vel, dir * -max * dt, ac * dt);
}

void Entity::UpdateFrame(float min, float max, float ac, float row) {
    frameNum += GetFrameTime() * ac;

    if (frameNum >= max) {
        frameNum = min;
    }

    rowNum = row;
}

void Entity::DrawSpriteRow(Texture2D texture, int frameX, int rowIndex,
                           int columnSize, int rowSize, Vector2 position,
                           float scale, float rotation, bool flipX) {
    // 1. Use the provided frame dimensions directly
    float frameWidth = (float)columnSize;
    float frameHeight = (float)rowSize;

    // 2. Define the source area in the texture
    // sourceRec.x/y is the top-left corner of the frame in the spritesheet
    Rectangle sourceRec = {
        (float)frameX * frameWidth, (float)rowIndex * frameHeight,
        flipX ? -frameWidth
              : frameWidth, // Negative width flips the image horizontally
        frameHeight};

    Vector2 drawPos = {position.x, position.y};
    Rectangle destRec = {drawPos.x, drawPos.y, frameWidth * scale,
                         frameHeight * scale};

    Vector2 origin = {(destRec.width / 2.0f), (destRec.height / 2.0f)};

    DrawTexturePro(texture, sourceRec, destRec, origin, rotation, WHITE);
}

void Entity::DrawShadow() {}

void Entity::DrawRect() {
    Vector2 drawpos = {pos.x, pos.y};
    DrawRectangleV(drawpos, siz, col);
}

// Health
void Entity::ManageHealth() {
    if (health <= 0)
        removed = true;
}
void Entity::SetHealth(float amnt) { health = amnt; }
void Entity::IncreaseHealth(float amnt) { health += amnt; }
void Entity::DecreaseHealth(float amnt) { health -= amnt; }
void Entity::SetHealthMax(float amnt) { healthMax = amnt; }
void Entity::IncreaseHealthMax(float amnt) { healthMax += amnt; }
void Entity::DecreaseHealthMax(float amnt) { healthMax -= amnt; }

int Entity::getInstanceCount() { return s_instanceCount; }
