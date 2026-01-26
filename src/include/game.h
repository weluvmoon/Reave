#pragma once

#include "constants.h"
#include "entities.h"
#include "network.h"
#include <fstream>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

extern Camera2D camera;
extern EntityManager entities;

class Game {
  public:
    Game() {
        GameState = GameStates::LEVEL;

        cameraZoom = 1.0f;
        cameraTarg = Vector2{0, 0};
        camera.target = cameraTarg;
        camera.zoom = cameraZoom;

        saveTo = 1, loadTo = 1;

        ETool = ETools::TILE;
        EToolNum = 0, EToolSize = 1;
        removeRectSize = {25, 25},
        removeRect = {0, 0, removeRectSize.x, removeRectSize.y};
    }

    enum GameStates { TITLE, MENU, LEVEL, EDITOR };
    GameStates GameState;

    float cameraZoom, defZoom;
    Vector2 cameraTarg, cameraOffset;

    char saveTo, loadTo;

    enum ETools { TILE, ENEMY, OBJECT, ITEM };
    int ETool = ETools::TILE;
    int EToolNum = 0, EToolSize = 1;

    Vector2 removeRectSize;
    Rectangle removeRect;

    void Init();
    void Update(float dt);
    void Draw();
    void Unload();

    void ManageState();
    void UpdateState(float dt);
    void DrawState();

    void UpdateEntities(float dt);
    void DrawEntities();

    void DrawGrid();
    void EditLevel(float dt);
    void SpawnEntity(int nm, Vector2 tg);
    void RemoveEntity();
};
