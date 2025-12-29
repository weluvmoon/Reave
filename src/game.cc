#include "module/game.h"
#include "module/constants.h"
#include "module/entities.h"
#include "module/entity.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

Camera2D camera;
EntityManager entities;

void Game::Update(float dt) {
    entities.CleanupRemoved();
    UpdateState(dt);
    ManageState();
}

void Game::Draw() { DrawState(); }

void Game::ManageState() {
    switch (GameState) {
    case TITLE:

        if (IsKeyPressed(KEY_ENTER)) {
            GameState = LEVEL;
        }
        break;
    case LEVEL:

        if (IsKeyPressed(KEY_ENTER)) {
            GameState = EDITOR;
        }
        break;
    case EDITOR:

        if (IsKeyPressed(KEY_ENTER)) {
            GameState = LEVEL;
        }
        break;
    default:
        break;
    }
}
void Game::UpdateState(float dt) {
    switch (GameState) {
    case TITLE:
        break;
    case LEVEL:
        entities.UpdateAll(dt);
        break;
    case EDITOR:
        break;
    default:
        break;
    }
}
void Game::DrawState() {
    switch (GameState) {
    case TITLE:
        DrawText("hello", 75, 75, 15, BLACK);

        break;
    case LEVEL:
        BeginMode2D(camera);
        entities.DrawAll();

        DrawCircle(50, 50, 50, BLACK);

        EndMode2D();

        DrawText("hello", 75, 75, 15, BLUE);
        break;
    case EDITOR:
        BeginMode2D(camera);
        entities.DrawAll();

        DrawGrid();
        EndMode2D();

        DrawText("hello", 75, 75, 15, GREEN);
        break;
    default:
        break;
    }
}

void Game::DrawGrid() {
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    int startX = floor(topLeft.x / GRID_SIZE) * GRID_SIZE;
    int startY = floor(topLeft.y / GRID_SIZE) * GRID_SIZE;
    int endX = ceil(bottomRight.x / GRID_SIZE) *
               GRID_SIZE; // Ensure grid covers full screen
    int endY = ceil(bottomRight.y / GRID_SIZE) * GRID_SIZE;

    // OPTIMIZATION: Manual Line Batching
    // Instead of hundreds of DrawLine calls, we send all vertices to the GPU at
    // once.
    rlBegin(RL_LINES);
    rlColor4ub(160, 160, 160, 255); // GRAY
    for (int x = startX; x <= endX; x += GRID_SIZE) {
        rlVertex2f((float)x, (float)startY);
        rlVertex2f((float)x, (float)endY);
    }
    for (int y = startY; y <= endY; y += GRID_SIZE) {
        rlVertex2f((float)startX, (float)y);
        rlVertex2f((float)endX, (float)y);
    }
    rlEnd();
}

void Game::EditLevel() {}

void Game::SpawnEntity() {}
