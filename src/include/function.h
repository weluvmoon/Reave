#pragma once

#include "entities.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cstddef>
#include <math.h> // Required for sinf, cosf, atan2f, sqrtf
#include <vector>

class FunctionManager {
  public:
    void DrawWavyLine(Vector2 start, Vector2 end, float waveHeight,
                      float waveLength, float speed, Color color);
    void DrawWavingSquareLine(Vector2 position, int segments, float size,
                              float waveAmplitude, float waveFrequency,
                              float time, Color color);
    void DrawWavingCircleLine(Vector2 position, int segments, float baseRadius,
                              float waveAmplitude, float waveFrequency,
                              float time, Color color);
};

struct InstanceData {
    Vector2 pos;
    Vector2 size;
    Color color;
    float depth;
};

struct InstanceBatch {
    std::vector<InstanceData> instances;
    Mesh quad;
    Material material;
    bool initialized = false;

    void Init();
    void Clear();
    void Submit(const InstanceData &inst);
    void Flush();
};

extern FunctionManager fM;
extern InstanceBatch iB;
