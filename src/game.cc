#include "include/game.h"
#include "include/assets.h"
#include "include/collision.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include "include/level.h"
#include "include/mod.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cstdio>

int clientSock;
struct sockaddr_in serverAddr;

std::string statusMessage = "";
float messageTimer = 0.0f;

void Game::Init() {
    // am.PlayMus(MUS_CHASE);
}

void Game::Update(float dt) {
    if (dt <= 0.0f)
        return;
    UpdateState(dt);
    ManageState();
}

void Game::Draw() { DrawState(); }

void Game::Unload() { lm.Clear(); }

void Game::ManageState() {
    switch (GameState) {
    case TITLE:
        if (IsKeyPressed(KEY_ENTER))
            GameState = LEVEL;
        break;
    case LEVEL:
        if (IsKeyPressed(KEY_ENTER))
            GameState = EDITOR;
        break;
    case EDITOR:
        if (IsKeyPressed(KEY_ENTER))
            GameState = LEVEL;
        break;
    default:
        break;
    }
}

void Game::UpdateState(float dt) {
    switch (GameState) {
    case LEVEL:
        em.UpdateAll(dt);
        EntitySystem(em);
        cS.ResolveAll(em, dt);

        cameraOffset = {GetScreenWidth() / 1.5f, GetScreenHeight() / 1.5f};
        cameraZoom = 0.75f;
        for (size_t i = 0; i < em.rendering.typeID.size(); ++i) {
            if (em.rendering.typeID[i] == EntityRegistry["CHARACTER"])
                cameraTarg = {em.physics.pos[i].x - cameraOffset.x,
                              em.physics.pos[i].y - cameraOffset.y};
        }

        camera.zoom = cameraZoom;
        camera.target = cameraTarg;

        break;
    case EDITOR:
        EntitySystem(em);
        EditLevel(dt);

        camera.zoom = cameraZoom;
        camera.target = cameraTarg;

        if (IsKeyPressed(KEY_SAVE)) {
            if (lm.Save("bin/content/level/level-1.json")) {
                statusMessage = "Level Saved!";
            } else {
                statusMessage = "Save Failed!";
            }
            messageTimer = 3.0f; // Show for 3 seconds
            cS.ResetTileGrid();
        } else if (IsKeyPressed(KEY_LOAD)) {
            if (lm.Load("bin/content/level/level-1.json")) {
                statusMessage = "Level Loaded!";
            } else {
                statusMessage = "Load Failed (File Not Found)!";
            }
            messageTimer = 3.0f;
            cS.ResetTileGrid();
        }

        if (messageTimer > 0)
            messageTimer -= GetFrameTime();

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
    case MENU: // Handle the missing case
        // Draw menu logic here
        break;
    case LEVEL:
        BeginMode2D(camera);

        em.DrawAll(camera);
        EntityDrawing(em);

        DrawCircle(50, 50, 50, BLACK);
        EndMode2D();
        break;
    case EDITOR:
        BeginMode2D(camera);

        em.DrawAll(camera);
        EntityDrawing(em);

        DrawGrid();
        DrawRectangleLinesEx(removeRect, 2.0f, BLACK);
        EndMode2D();

        DrawText(TextFormat("Etool: %d", ETool), 10, 30, 20, BLACK);
        DrawText(TextFormat("EtoolNum: %d", EToolNum), 10, 50, 20, BLACK);
        DrawText(TextFormat("EtoolSize: %d", EToolSize), 10, 70, 20, BLACK);

        int entityCount = em.GetActiveCount();
        DrawText(TextFormat("Entities: %d", entityCount), 10, 90, 20, GREEN);

        Vector2 messageLoc =
            Vector2{GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f};
        if (messageTimer > 0) {
            DrawText(statusMessage.c_str(), messageLoc.x, messageLoc.y, 20,
                     DARKGRAY);
        }
        break;
    }
}

void Game::DrawGrid() {
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera);
    Vector2 bottomRight = GetScreenToWorld2D(
        {(float)GetScreenWidth(), (float)GetScreenHeight()}, camera);

    float startX = floor(topLeft.x / GRID_SIZE) * GRID_SIZE;
    float startY = floor(topLeft.y / GRID_SIZE) * GRID_SIZE;
    float endX = ceil(bottomRight.x / GRID_SIZE) * GRID_SIZE;
    float endY = ceil(bottomRight.y / GRID_SIZE) * GRID_SIZE;

    rlBegin(RL_LINES);
    rlColor4ub(160, 160, 160, 255);
    for (float x = startX; x <= endX; x += GRID_SIZE) {
        rlVertex2f(x, startY);
        rlVertex2f(x, endY);
    }
    for (float y = startY; y <= endY; y += GRID_SIZE) {
        rlVertex2f(startX, y);
        rlVertex2f(endX, y);
    }
    rlEnd();
}

void Game::EditLevel(float dt) {
    float zoomS = 0.1f;
    float moveS = 400.0f; // Snappier for 2025
    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
    float wheel = GetMouseWheelMove();

    // Snap to grid
    Vector2 snapped = {
        floor(mousePos.x / GRID_SIZE) * GRID_SIZE + GRID_SIZE / 2.0f,
        floor(mousePos.y / GRID_SIZE) * GRID_SIZE + GRID_SIZE / 2.0f};

    cameraZoom = std::clamp(cameraZoom + wheel * zoomS, 0.2f, 5.0f);

    // Camera Controls
    if (IsKeyDown(KEY_MOVE_UP))
        cameraTarg.y -= moveS * dt;
    if (IsKeyDown(KEY_MOVE_DOWN))
        cameraTarg.y += moveS * dt;
    if (IsKeyDown(KEY_MOVE_LEFT))
        cameraTarg.x -= moveS * dt;
    if (IsKeyDown(KEY_MOVE_RIGHT))
        cameraTarg.x += moveS * dt;

    // Tool logic
    removeRectSize = {(float)EToolSize * GRID_SIZE * 0.9f,
                      (float)EToolSize * GRID_SIZE * 0.9f};
    removeRect = {snapped.x - removeRectSize.x / 2,
                  snapped.y - removeRectSize.y / 2, removeRectSize.x,
                  removeRectSize.y};

    // Tool Selection (Consolidated)
    if (IsKeyPressed(KEY_NEXT_TOOL))
        ETool++;
    if (IsKeyPressed(KEY_LAST_TOOL))
        ETool--;
    if (IsKeyPressed(KEY_NEXT_TOOL_NUM))
        EToolNum = std::min(EToolNum + 1, 2000);
    if (IsKeyPressed(KEY_LAST_TOOL_NUM))
        EToolNum = std::max(EToolNum - 1, 0);
    if (IsKeyPressed(KEY_NEXT_TOOL_SIZE))
        EToolSize = std::min(EToolSize + 1, 10);
    if (IsKeyPressed(KEY_LAST_TOOL_SIZE))
        EToolSize = std::max(EToolSize - 1, 1);

    if (IsKeyPressed(KEY_Z))
        EToolNum = 0;
    else if (IsKeyPressed(KEY_X))
        EToolNum = EntityTys::TYTILE;
    else if (IsKeyPressed(KEY_C))
        EToolNum = EntityTys::TYHITBOX;
    else if (IsKeyPressed(KEY_V))
        EToolNum = EntityTys::TYWALKER;

    if (IsKeyDown(KEY_R)) {
        RemoveEntity();
        cS.ResetTileGrid();
    }
    if (IsKeyDown(KEY_PLACE)) {
        SpawnEntity(EToolNum, snapped);
        cS.ResetTileGrid();
    }

    if (IsKeyPressed(KEY_F5)) {
        em.LoadConfigs("assets/entities.json");
        TraceLog(LOG_INFO, "Configs reloaded successfully!");
    }
}

void Game::SpawnEntity(int nm, Vector2 tg) {
    Vector2 spawnPos = {tg.x - GRID_SIZE / 2, tg.y - GRID_SIZE / 2};

    for (size_t i = 0; i < em.physics.pos.size(); ++i) {
        if (em.physics.active[i] && em.physics.pos[i].x == spawnPos.x &&
            em.physics.pos[i].y == spawnPos.y)
            return;
    }

    if (IdToName.count(nm)) {
        am.PlaySfx(SFX_ADDENT);
        em.AddEntityJ(IdToName[nm], spawnPos);
    }
}

void Game::RemoveEntity() {
    for (size_t i = 0; i < em.physics.pos.size();) {
        if (!em.physics.active[i]) {
            i++;
            continue;
        }

        if (CheckCollisionRecs(em.physics.rect[i], removeRect)) {
            am.PlaySfx(SFX_REMOVENT);
            em.FastRemove(i);
        } else {
            i++;
        }
    }
}
