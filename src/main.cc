#include "include/assets.h"
#include "include/constants.h"
#include "include/game.h"
#include "include/entities.h"
#include "raylib.h"

Camera2D camera;
Game game;
AssetManager am;
EntityManager em;
CollisionSystem cS;

int main() {
    const int screenWidth = 640;
    const int screenHeight = 450;
    const int FrameCap = 60;

    InitWindow(screenWidth, screenHeight, "Beta");
    SetAudioStreamBufferSizeDefault(8192);
    InitAudioDevice();
    SetTargetFPS(FrameCap);

    for (int i = 0; i < 10; i++) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Initializing Game...", 200, 200, 20, WHITE);
        EndDrawing();
    }

    am.Startup();
    game.Init();

    while (!WindowShouldClose()) {
        am.UpdateMusic();
        am.UpdateMusicFading();
        game.Update(GetFrameTime());

        BeginDrawing();

        ClearBackground(RAYWHITE);

        game.Draw();

        DrawFPS(10, 10);

        EndDrawing();
    }

    am.Cleanup();
    game.Unload();

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

