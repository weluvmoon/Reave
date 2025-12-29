#pragma once

#include "constants.h"
#include "entity.h"
#include "raylib.h"
#include <asm-generic/errno.h>
#include <fstream>
#include <string>
#include <vector>

extern Camera2D camera;

class Game {
  public:
    Game() {
        GameState = GameStates::LEVEL;

        cameraZoom = 1.0f;
        cameraTarg = Vector2{0, 0};
        camera.target = cameraTarg;
        camera.zoom = cameraZoom;

        saveTo = 1, loadTo = 1;
        ETool = ETools::TILE, EToolNum = 1.0f;

        removeRectSize = {25, 25},
        removeRect = {0, 0, removeRectSize.x, removeRectSize.y};
    }

    enum GameStates { TITLE, MENU, LEVEL, EDITOR };
    GameStates GameState;

    float cameraZoom, defZoom;
    Vector2 cameraTarg;

    float saveTo, loadTo;

    enum ETools { TILE, ITEM, OBJECT, ENEMY };
    ETools ETool;
    float EToolNum, EToolSize;

    Vector2 removeRectSize;
    Rectangle removeRect;

    void Update(float dt);
    void Draw();

    void ManageState();
    void UpdateState(float dt);
    void DrawState();

    void UpdateEntities(float dt);
    void DrawEntities();

    void DrawGrid();
    void EditLevel();
    void SpawnEntity();
    void RemoveEntity();
};
