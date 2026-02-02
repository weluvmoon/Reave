#include "include/collision.h"
#include <algorithm>
#include <cmath>
#include <raylib.h>
#include <vector>

inline int CollisionSystem::GetGridIndex(float x, float y) {
    if (std::isnan(x) || std::isnan(y))
        return 0;

    [[maybe_unused]] int gx = std::clamp(
        static_cast<int>(x / static_cast<float>(CELL_SIZE)), 0, COLS - 1);
    [[maybe_unused]] int gy = std::clamp(
        static_cast<int>(y / static_cast<float>(CELL_SIZE)), 0, ROWS - 1);

    if (x == 0)
        gx = 0;
    if (x == COLS * CELL_SIZE)
        gx = COLS - 1;

    return (gy * COLS) + gx;
}

void CollisionSystem::ResetTileGrid() { tileGridInitialized = false; }

void CollisionSystem::BuildGrid(EntityManager &em) {
    for (int i = 0; i < COLS * ROWS; ++i) {
        tileGrid[i].clear();
        grid[i].clear();
    }

    for (size_t i = 0; i < em.physics.pos.size(); ++i) {
        if (!em.physics.active[i])
            continue;

        if (em.rendering.typeID[i] == EntityTys::TYTILE) {
            int idx = GetGridIndex(em.physics.pos[i].x, em.physics.pos[i].y);
            tileGrid[idx].push_back(i);
        }
    }

    for (size_t i = 0; i < em.physics.pos.size(); ++i) {
        if (!em.physics.active[i] ||
            em.rendering.typeID[i] == EntityTys::TYTILE)
            continue;

        float centerX = em.physics.pos[i].x + (em.physics.siz[i].x * 0.5f);
        float centerY = em.physics.pos[i].y + (em.physics.siz[i].y * 0.5f);
        grid[GetGridIndex(centerX, centerY)].push_back(i);
    }
}

void CollisionSystem::ResolveAll(EntityManager &em, float dt) {
    if (dt <= 0.0f)
        return;

    BuildGrid(em);

    for (int cellIdx = 0; cellIdx < COLS * ROWS; ++cellIdx) {
        if (grid[cellIdx].empty())
            continue;

        for (size_t i : grid[cellIdx]) {
            this->ResolveCollision(em, i);
        }
    }
}

void CollisionSystem::ResolveCollision(EntityManager &em, size_t i) {
    em.physics.grounded[i] = false;
    em.physics.walled[i] = false;

    em.SyncRect(em, i);
    ResolveAxis(em, i, true);

    em.SyncRect(em, i);
    ResolveAxis(em, i, false);
}

// TODO: Fix sliding resolution
void CollisionSystem::ResolveAxis(EntityManager &em, size_t i, bool isXAxis) {
    Vector2 &pos = em.physics.pos[i];
    Vector2 &vel = em.physics.vel[i];
    Vector2 &siz = em.physics.siz[i];

    em.SyncRect(em, i);
    Rectangle sensor = isXAxis ? em.physics.rectX[i] : em.physics.rectY[i];

    int startX = std::clamp((int)floorf(sensor.x / CELL_SIZE), 0, COLS - 1);
    int startY = std::clamp((int)floorf(sensor.y / CELL_SIZE), 0, ROWS - 1);
    int endX = std::clamp((int)floorf((sensor.x + sensor.width) / CELL_SIZE), 0,
                          COLS - 1);
    int endY = std::clamp((int)floorf((sensor.y + sensor.height) / CELL_SIZE),
                          0, ROWS - 1);

    const float slidingFactor = 0.7f;
    const float velocityThreshold = 0.05f;

    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            for (size_t j : tileGrid[y * COLS + x]) {
                if (i == j || em.rendering.typeID[j] != EntityTys::TYTILE)
                    continue;
                if (!em.physics.collide[i] || !em.physics.collide[j])
                    continue;

                Rectangle rJ = em.physics.rect[j];

                if (CheckCollisionRecs(sensor, rJ)) {
                    if (isXAxis) {
                        if (vel.x > 0) {
                            pos.x = rJ.x - siz.x;
                        } else if (vel.x < 0) {
                            pos.x = rJ.x + rJ.width;
                        }

                        if (std::abs(vel.x) < velocityThreshold)
                            vel.x = 0;

                        em.physics.walled[i] = true;
                    } else {
                        if (vel.y > 0) {
                            pos.y = rJ.y - siz.y;
                            em.physics.grounded[i] = true;
                        } else if (vel.y < 0) {
                            pos.y = rJ.y + rJ.height;
                            vel.y = 0;
                        }

                        vel.y *= slidingFactor;
                        if (std::abs(vel.y) < velocityThreshold)
                            vel.y = 0;
                    }

                    em.SyncRect(em, i);
                    sensor =
                        isXAxis ? em.physics.rectX[i] : em.physics.rectY[i];
                }
            }
        }
    }
}

CollisionResult
CollisionSystem::CheckCollisionsInternal(EntityManager &em, size_t i,
                                         const Rectangle &sensorRect) {
    const float invCell = 1.0f / CELL_SIZE;
    int startX = std::max(0, (int)(sensorRect.x * invCell));
    int startY = std::max(0, (int)(sensorRect.y * invCell));
    int endX =
        std::min(COLS - 1, (int)((sensorRect.x + sensorRect.width) * invCell));
    int endY =
        std::min(ROWS - 1, (int)((sensorRect.y + sensorRect.height) * invCell));

    for (int y = startY; y <= endY; ++y) {
        int rowOffset = y * COLS;
        for (int x = startX; x <= endX; ++x) {
            const auto &cell = tileGrid[rowOffset + x];
            for (size_t j : cell) {
                if (i == j)
                    continue;

                if (CheckCollisionRecs(sensorRect, em.physics.rect[j])) {
                    return {true, em.rendering.typeID[j], em.physics.pos[j],
                            em.physics.siz[j]};
                }
            }
        }
    }
    return {false, 0, {0, 0}, {0, 0}};
}

CollisionResult CollisionSystem::CheckCollisions(EntityManager &em,
                                                 Rectangle &sensorRect,
                                                 size_t i) {
    return CheckCollisionsInternal(em, i, sensorRect);
}

CollisionResult CollisionSystem::CheckCollisionsX(EntityManager &em, size_t i) {
    return CheckCollisionsInternal(em, i, em.physics.rectX[i]);
}

CollisionResult CollisionSystem::CheckCollisionsY(EntityManager &em, size_t i) {
    return CheckCollisionsInternal(em, i, em.physics.rectY[i]);
}

bool CollisionSystem::LineIntersectsRect(Vector2 a, Vector2 b, Rectangle r) {
    float minX = (a.x < b.x) ? a.x : b.x;
    float maxX = (a.x > b.x) ? a.x : b.x;
    float minY = (a.y < b.y) ? a.y : b.y;
    float maxY = (a.y > b.y) ? a.y : b.y;

    if (maxX < r.x || minX > r.x + r.width || maxY < r.y ||
        minY > r.y + r.height) {
        return false;
    }

    if (CheckCollisionPointRec(a, r) || CheckCollisionPointRec(b, r)) {
        return true;
    }

    Vector2 tl = {r.x, r.y};
    Vector2 tr = {r.x + r.width, r.y};
    Vector2 bl = {r.x, r.y + r.height};
    Vector2 br = {r.x + r.width, r.y + r.height};

    return CheckCollisionLines(a, b, tl, tr, nullptr) ||
           CheckCollisionLines(a, b, tr, br, nullptr) ||
           CheckCollisionLines(a, b, br, bl, nullptr) ||
           CheckCollisionLines(a, b, bl, tl, nullptr);
}
