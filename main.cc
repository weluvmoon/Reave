#include "raylib.h"

#include "src/modules/entities.h"
#include "src/modules/game.h"

int main() {
  const int screenWidth = 640;
  const int screenHeight = 450;
  const int FrameCap = 60;

  InitWindow(screenWidth, screenHeight, "Beta");
  SetTargetFPS(FrameCap);

  // Initialize the container structure

  Game game;
  game.Init();

  while (!WindowShouldClose()) {
    game.Update(GetFrameTime());

    BeginDrawing();

    ClearBackground(RAYWHITE);

    game.Draw();

    // You can add general UI elements or collision text here
    DrawFPS(10, 10);

    EndDrawing();
  }

  game.Unload();

  CloseWindow();
  return 0;
}
