#pragma once

#include <raylib.h>
#include <string>
#include <unordered_map>

// 1. Fixed syntax: Removed "!" from MUS_CHASE
enum GameTextures { TEX_MAIN, TEX_DEF, TEX_COUNT };
enum GameFonts { FON_MAIN, FON_DEF, FON_COUNT };
enum GameSfx { SFX_ADDENT, SFX_REMOVENT, SFX_EXPLODE, SFX_DEF, SFX_COUNT };
enum GameMusic { MUS_CHASE, MUS_DEF, MUS_COUNT };

struct AssetManager {
    Texture2D textures[TEX_COUNT];
    // --- Fonts ---
    Font fonts[FON_COUNT];

    // --- SFX ---
    Sound sfxs[SFX_COUNT];

    // --- Music ---
    Music music[MUS_COUNT];
    int fadingMusicIndex = -1;
    float fadeTimer = 0.0f;
    float fadeDuration = 0.0f;
    float startVolume = 1.0f;

    Font customFont;

    // Cache to map file paths to loaded textures
    std::unordered_map<std::string, Texture2D> pathCache;

    void LoadTextures() {
        // Create a 1x1 white pixel for the default texture
        Image whitePixel = GenImageColor(1, 1, WHITE);
        textures[TEX_DEF] = LoadTextureFromImage(whitePixel);
        UnloadImage(whitePixel);

        for (int i = 0; i < TEX_COUNT; i++) {
            if (textures[i].id == 0 && i != TEX_MAIN) {
                TraceLog(LOG_WARNING,
                         "Texture index %d initialized as empty/default.", i);
            }
        }
    }

    Texture2D GetTextureByPath(const std::string &path) {
        if (path.empty())
            return textures[TEX_DEF];

        if (pathCache.find(path) != pathCache.end()) {
            return pathCache[path];
        }

        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id == 0) {
            TraceLog(LOG_ERROR, "Failed to load texture: %s", path.c_str());
            return textures[TEX_DEF];
        }

        pathCache[path] = tex;
        return tex;
    }

    bool IsSoundReady(Sound sound) {
        return ((sound.frameCount > 0) &&        // Must have audio frames
                (sound.stream.buffer != NULL) && // Buffer must be allocated
                (sound.stream.sampleRate > 0) && // Valid sample rate
                (sound.stream.sampleSize > 0) && // Valid bit depth
                (sound.stream.channels > 0));    // At least one audio channel
    }

    void LoadSfx() {
        // Example: Manual assignment for specific SFX
        sfxs[SFX_ADDENT] = LoadSound("assets/sfx/add.ogg");
        sfxs[SFX_REMOVENT] = LoadSound("assets/sfx/remove.ogg");
        sfxs[SFX_EXPLODE] = LoadSound("assets/sfx/explode.ogg");

        // Optional: Verification loop
        for (int i = 0; i < SFX_COUNT; i++) {
            if (!IsSoundReady(sfxs[i])) {
                TraceLog(LOG_WARNING,
                         "SFX index %d not ready or failed to load.", i);
            }
        }
    }

    void PlaySfx(GameSfx sfxIndex, float volume = 1.0f) {
        if (sfxIndex < SFX_COUNT && sfxs[sfxIndex].frameCount > 0) {
            SetSoundVolume(sfxs[sfxIndex], volume); // Optional: Adjust volume
            PlaySound(sfxs[sfxIndex]);
        }
    }

    void StopAllSfx() {
        for (int i = 0; i < SFX_COUNT; i++) {
            StopSound(sfxs[i]);
        }
    }

    bool IsMusicReady(Music music) {
        return ((music.stream.buffer != NULL) && // Stream buffer must exist
                (music.frameCount > 0) &&        // Total frames must be known
                (music.stream.sampleRate > 0));
    }

    void LoadMusic() {
        music[MUS_CHASE] = LoadMusicStream("assets/music/chase_theme.ogg");

        for (int i = 0; i < MUS_COUNT; i++) {
            if (!IsMusicReady(music[i])) {
                TraceLog(LOG_WARNING,
                         "Music index %d not ready or failed to load.", i);
            }
        }
    }

    void UpdateMusic() {
        for (int i = 0; i < MUS_COUNT; i++) {
            if (IsMusicStreamPlaying(music[i])) {
                UpdateMusicStream(music[i]);
            }
        }
    }

    void UpdateMusicFading() {
        if (fadingMusicIndex != -1) {
            fadeTimer -= GetFrameTime();

            if (fadeTimer <= 0.0f) {
                StopMusicStream(music[fadingMusicIndex]);
                SetMusicVolume(music[fadingMusicIndex],
                               1.0f); // Reset for next play
                fadingMusicIndex = -1;
            } else {
                float newVolume = (fadeTimer / fadeDuration) * startVolume;
                SetMusicVolume(music[fadingMusicIndex], newVolume);
            }
        }
    }

    void PlayMus(GameMusic musicIndex, float volume = 1.0f, bool loop = true) {
        if (musicIndex < MUS_COUNT &&
            music[musicIndex].stream.buffer != nullptr) {
            SetMusicVolume(music[musicIndex], volume); //
            music[musicIndex].looping = loop;
            PlayMusicStream(music[musicIndex]);
        }
    }

    void FadeOutMusic(GameMusic musicIndex, float duration) {
        if (musicIndex < MUS_COUNT && IsMusicStreamPlaying(music[musicIndex])) {
            fadingMusicIndex = musicIndex;
            fadeDuration = duration;
            fadeTimer = duration;
            startVolume = 1.0f;
        }
    }

    void StopAllMusic() {
        for (int i = 0; i < MUS_COUNT; i++) {
            if (IsMusicStreamPlaying(music[i])) {
                StopMusicStream(music[i]);
            }
        }
    }

    void StopAllAudio() {
        StopAllMusic();
        StopAllSfx();
    }

    void UnloadAssets() {
        // Unload textures
        for (int i = 0; i < TEX_COUNT; i++) {
            if (textures[i].id > 0)
                UnloadTexture(textures[i]);
        }
        for (auto &[path, tex] : pathCache) {
            if (tex.id > 0)
                UnloadTexture(tex);
        }
        pathCache.clear();

        // Unload SFX
        for (int i = 0; i < SFX_COUNT; i++) {
            if (sfxs[i].frameCount > 0)
                UnloadSound(sfxs[i]);
        }

        // 2. Fixed: Use UnloadMusicStream for Music types
        for (int i = 0; i < MUS_COUNT; i++) {
            if (music[i].stream.buffer != nullptr)
                UnloadMusicStream(music[i]);
        }

        // Unload Font
        if (customFont.texture.id > 0)
            UnloadFont(customFont);
    }

    void Startup() {
        if (IsAudioDeviceReady()) {
            SetMasterVolume(1.0f); // Ensure master volume isn't 0
            LoadTextures();
            LoadSfx();
            LoadMusic();
        } else {
            TraceLog(LOG_ERROR, "CRITICAL: Audio device failed to initialize!");
        }
    }

    void Cleanup() {
        StopAllAudio();
        UnloadAssets();
    }
};

extern AssetManager am;

