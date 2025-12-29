#pragma once

#include "raylib.h"

extern Camera2D camera;

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

    KEY_MOVE_UP = KEY_W,
    KEY_MOVE_DOWN = KEY_S,
    KEY_MOVE_LEFT = KEY_A,
    KEY_MOVE_RIGHT = KEY_D,

    KEY_JUMP = KEY_SPACE,
    KEY_DASH = KEY_LEFT_SHIFT,
};

// In modules/constants.h (or wherever you keep global constants)
const float GRID_SIZE = 25.0f; // Each grid cell is 25x25 units
const int MAP_WIDTH_UNITS = 2500;
const int MAP_HEIGHT_UNITS = 2500;
