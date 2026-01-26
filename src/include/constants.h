#pragma once

#include "collision.h"
#include "entities.h"
#include "raylib.h"
#include <cstdint>
#include <string>
#include <vector>

extern Camera2D camera;
extern class CollisionSystem cS;

enum KEYS {
    KEY_PGAME = KEY_M,
    KEY_PLEVEL = KEY_N,

    KEY_PLACE = KEY_E,
    KEY_REMOVE = KEY_R,

    KEY_ZOOM_IN = KEY_KP_1,
    KEY_ZOOM_OUT = KEY_KP_2,

    KEY_LAST_TOOL = KEY_T,
    KEY_NEXT_TOOL = KEY_Y,
    KEY_LAST_TOOL_NUM = KEY_G,
    KEY_NEXT_TOOL_NUM = KEY_H,
    KEY_LAST_TOOL_SIZE = KEY_ONE,
    KEY_NEXT_TOOL_SIZE = KEY_TWO,

    KEY_SAVE = KEY_K,
    KEY_LOAD = KEY_L,

    KEY_MOVE_UP = KEY_KP_8,
    KEY_MOVE_DOWN = KEY_KP_5,
    KEY_MOVE_LEFT = KEY_KP_4,
    KEY_MOVE_RIGHT = KEY_KP_6,

    KEY_JUMP = KEY_Z,
    KEY_DASH = KEY_X,
};

const float GRID_SIZE = 32.0f;
const int MAP_WIDTH_UNITS = 2500;
const int MAP_HEIGHT_UNITS = 2500;
