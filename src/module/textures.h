#pragma once

#include "raylib.h"

class TextureManager {
  public:
    Texture2D sheetText;

    void LoadTextures() {
        sheetText = LoadTexture("assets/textures/sheet.png");
    }
};

extern TextureManager textures;
