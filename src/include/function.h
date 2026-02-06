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

    // --- Penner Easing Functions ---
    // t = current time, b = start value, c = change in value, d = duration
    struct Ease {
        static float InQuad(float t, float b, float c, float d) {
            t /= d;
            return c * t * t + b;
        }

        static float OutQuad(float t, float b, float c, float d) {
            t /= d;
            return -c * t * (t - 2) + b;
        }

        static float InOutQuad(float t, float b, float c, float d) {
            t /= d / 2;
            if (t < 1)
                return c / 2 * t * t + b;
            t--;
            return -c / 2 * (t * (t - 2) - 1) + b;
        }

        static float OutElastic(float t, float b, float c, float d) {
            if (t == 0)
                return b;
            if ((t /= d) == 1)
                return b + c;
            float p = d * .3f;
            float s = p / 4;
            return (c * powf(2, -10 * t) * sinf((t * d - s) * (2 * PI) / p) +
                    c + b);
        }
    };
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
