#pragma once

#include "entities.h"
#include <cstddef>
#include <raylib.h>
#include <unordered_map>
#include <vector>

struct CollisionResult {
    bool hit = false;
    int typeID = -1;
    Vector2 pos = {0, 0};
    Vector2 siz = {0, 0};
};

class CollisionSystem {
  public:
    CollisionSystem() {
        head.assign(COLS * ROWS, -1);
        next.assign(7500, -1);
    }
    static const int CELL_SIZE = 32;
    static const int COLS = 128; // Adjust based on your world size
    static const int ROWS = 128;

    void ResolveAll(EntityManager &em, float dt);
    void ResetTileGrid();

    void ResolveAxis(EntityManager &em, size_t i, bool isXAxis);
    CollisionResult CheckCollisionsInternal(EntityManager &em, size_t i,
                                            const Rectangle &sensorRect);
    CollisionResult CheckCollisions(EntityManager &em, size_t i);
    CollisionResult CheckCollisionsX(EntityManager &em, size_t i);
    CollisionResult CheckCollisionsY(EntityManager &em, size_t i);

    bool LineIntersectsRect(Vector2 a, Vector2 b, Rectangle r);

  private:
    std::vector<int> head;
    std::vector<int> next;
    std::vector<size_t> grid[COLS * ROWS];
    std::vector<size_t> tileGrid[COLS * ROWS];
    bool tileGridInitialized = false;

    inline int GetGridIndex(float x, float y);
    void BuildGrid(EntityManager &em);
    void ResolveCollision(EntityManager &em, size_t i);
};
