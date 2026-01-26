#include "include/function.h"
#include "include/entities.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <cstddef>
#include <math.h> // Required for sinf, cosf, atan2f, sqrtf
#include <vector>

void FunctionManager::DrawWavyLine(Vector2 start, Vector2 end, float waveHeight,
                                   float waveLength, float speed, Color color) {
    float distance = Vector2Distance(start, end);
    float angle = atan2(end.y - start.y, end.x - start.x);

    int segments = (int)(distance / 10.0f);

    Vector2 previousPoint = start;
    float time = GetTime();

    for (int i = 1; i <= segments; i++) {
        float fraction = (float)i / (float)segments;
        Vector2 currentPoint = Vector2Lerp(start, end, fraction);

        float offset =
            sinf((fraction * distance / waveLength) + (time * speed)) *
            waveHeight;

        Vector2 offsetDir = {cosf(angle + M_PI / 2.0f),
                             sinf(angle + M_PI / 2.0f)};

        currentPoint.x += offsetDir.x * offset;
        currentPoint.y += offsetDir.y * offset;

        DrawLineV(previousPoint, currentPoint, color);

        previousPoint = currentPoint;
    }
}

void FunctionManager::DrawWavingSquareLine(Vector2 position, int segments,
                                           float size, float waveAmplitude,
                                           float waveFrequency, float time,
                                           Color color) {
    if (segments % 4 != 0)
        segments += (4 - (segments % 4));

    std::vector<Vector2> points(segments + 1);
    float sideLen = size;
    int segsPerSide = segments / 4;

    for (int i = 0; i <= segments; i++) {
        Vector2 basePos;
        Vector2 normal;
        float sideProgress = (float)(i % segsPerSide) / segsPerSide;
        int side = i / segsPerSide;

        if (side == 0) {
            basePos = {position.x - sideLen / 2 + sideProgress * sideLen,
                       position.y - sideLen / 2};
            normal = {0, -1};
        } else if (side == 1) {
            basePos = {position.x + sideLen / 2,
                       position.y - sideLen / 2 + sideProgress * sideLen};
            normal = {1, 0};
        } else if (side == 2) {
            basePos = {position.x + sideLen / 2 - sideProgress * sideLen,
                       position.y + sideLen / 2};
            normal = {0, 1};
        } else {
            basePos = {position.x - sideLen / 2,
                       position.y + sideLen / 2 - sideProgress * sideLen};
            normal = {-1, 0};
        }

        float angle = (float)i / segments * 2.0f * PI;
        float wave = waveAmplitude * sinf(waveFrequency * angle - time);

        points[i].x = basePos.x + normal.x * wave;
        points[i].y = basePos.y + normal.y * wave;
    }
    points[segments] = points[0];
    DrawLineStrip(points.data(), segments + 1, color);
}

void FunctionManager::DrawWavingCircleLine(Vector2 position, int segments,
                                           float baseRadius,
                                           float waveAmplitude,
                                           float waveFrequency, float time,
                                           Color color) {
    std::vector<Vector2> points(segments + 1);

    for (int i = 0; i <= segments; i++) {
        float angle = (float)i / segments * 2.0f * PI;

        float currentRadius =
            baseRadius + waveAmplitude * sinf(waveFrequency * angle - time);

        points[i].x = position.x + currentRadius * cosf(angle);
        points[i].y = position.y + currentRadius * sinf(angle);
    }

    DrawLineStrip(points.data(), segments + 1, color);
}

void InstanceBatch::Init() {
    if (initialized)
        return;

    quad = GenMeshPlane(1.0f, 1.0f, 1, 1);
    material = LoadMaterialDefault();

    material.shader = LoadShader(0, TextFormat("assets/shaders/instance.fs"));

    initialized = true;
}

void InstanceBatch::Clear() { instances.clear(); }

void InstanceBatch::Submit(const InstanceData &inst) {
    instances.push_back(inst);
}

void InstanceBatch::Flush() {
    if (instances.empty())
        return;

    std::sort(instances.begin(), instances.end(),
              [](const InstanceData &a, const InstanceData &b) {
                  if (a.depth != b.depth)
                      return a.depth < b.depth;
                  return a.pos.y < b.pos.y;
              });
    std::vector<Matrix> transforms(instances.size());
    std::vector<Color> colors(instances.size());

    for (size_t i = 0; i < instances.size(); i++) {
        transforms[i] = MatrixMultiply(
            MatrixScale(instances[i].size.x, instances[i].size.y, 1.0f),
            MatrixTranslate(instances[i].pos.x, instances[i].pos.y, 0.0f));
        colors[i] = instances[i].color;
    }

    UpdateMeshBuffer(quad, 4, transforms.data(),
                     transforms.size() * sizeof(Matrix), 0);

    rlEnableShader(material.shader.id);
    DrawMeshInstanced(quad, material, transforms.data(), transforms.size());
    rlDisableShader();
}

extern FunctionManager fM;
extern InstanceBatch iB;
