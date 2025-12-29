#pragma once

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h> // Required for sinf, cosf, atan2f, sqrtf
#include <vector>
#include <algorithm>

class FunctionManager{
    public:
        static bool LineIntersectsRect(Vector2 a, Vector2 b, Rectangle r) {
            Vector2 tl = { r.x, r.y };
            Vector2 tr = { r.x + r.width, r.y };
            Vector2 bl = { r.x, r.y + r.height };
            Vector2 br = { r.x + r.width, r.y + r.height };
        
            return
                CheckCollisionLines(a, b, tl, tr, nullptr) ||
                CheckCollisionLines(a, b, tr, br, nullptr) ||
                CheckCollisionLines(a, b, br, bl, nullptr) ||
                CheckCollisionLines(a, b, bl, tl, nullptr) ||
                CheckCollisionPointRec(a, r) ||
                CheckCollisionPointRec(b, r);
        }
               
        void DrawWavyLine(Vector2 start, Vector2 end, float waveHeight, float waveLength, float speed, Color color){
            float distance = Vector2Distance(start, end);
            // Determine the angle of the straight line (in radians)
            float angle = atan2(end.y - start.y, end.x - start.x);
        
            // Number of segments to draw the wave smoothly
            int segments = (int)(distance / 10.0f); // Adjust segment length for smoothness vs performance
        
            Vector2 previousPoint = start;
            float time = GetTime();
        
            for (int i = 1; i <= segments; i++)
            {
                float fraction = (float)i / (float)segments;
                // Interpolate along the straight line
                Vector2 currentPoint = Vector2Lerp(start, end, fraction);
        
                // Calculate sine wave offset
                // The time variable makes the wave move
                float offset = sinf((fraction * distance / waveLength) + (time * speed)) * waveHeight;
        
                // Calculate perpendicular offset direction (angle + 90 degrees)
                Vector2 offsetDir = { cosf(angle + M_PI/2.0f), sinf(angle + M_PI/2.0f) };
                
                // Apply offset
                currentPoint.x += offsetDir.x * offset;
                currentPoint.y += offsetDir.y * offset;
        
                // Draw the segment
                DrawLineV(previousPoint, currentPoint, color);
        
                previousPoint = currentPoint;
            }
        }


        void DrawWavingSquareLine(Vector2 position, int segments, float size, float waveAmplitude, float waveFrequency, float time, Color color) {
            if (segments % 4 != 0) segments += (4 - (segments % 4));
            
            std::vector<Vector2> points(segments + 1);
            float sideLen = size; 
            int segsPerSide = segments / 4;
        
            for (int i = 0; i <= segments; i++) {
                Vector2 basePos;
                Vector2 normal; // The direction the wave pushes out
                float sideProgress = (float)(i % segsPerSide) / segsPerSide;
                int side = i / segsPerSide;
        
                if (side == 0) { // Top side (Left to Right)
                    basePos = { position.x - sideLen/2 + sideProgress * sideLen, position.y - sideLen/2 };
                    normal = { 0, -1 };
                } else if (side == 1) { // Right side (Top to Bottom)
                    basePos = { position.x + sideLen/2, position.y - sideLen/2 + sideProgress * sideLen };
                    normal = { 1, 0 };
                } else if (side == 2) { // Bottom side (Right to Left)
                    basePos = { position.x + sideLen/2 - sideProgress * sideLen, position.y + sideLen/2 };
                    normal = { 0, 1 };
                } else { // Left side (Bottom to Top)
                    basePos = { position.x - sideLen/2, position.y + sideLen/2 - sideProgress * sideLen };
                    normal = { -1, 0 };
                }
        
                float angle = (float)i / segments * 2.0f * PI;
                float wave = waveAmplitude * sinf(waveFrequency * angle - time);
                
                points[i].x = basePos.x + normal.x * wave;
                points[i].y = basePos.y + normal.y * wave;
            }
            points[segments] = points[0];
            DrawLineStrip(points.data(), segments + 1, color);
        }

        void DrawWavingCircleLine(Vector2 position, int segments, float baseRadius, float waveAmplitude, float waveFrequency, float time, Color color) {
            // We need segments + 1 points to close the loop (last point = first point)
            std::vector<Vector2> points(segments + 1);
        
            for (int i = 0; i <= segments; i++) {
                // Calculate angle from 0 to 2*PI
                float angle = (float)i / segments * 2.0f * PI; 
        
                // Offset the wave by 'time' to make it ripple around the circle
                float currentRadius = baseRadius + waveAmplitude * sinf(waveFrequency * angle - time);
        
                points[i].x = position.x + currentRadius * cosf(angle);
                points[i].y = position.y + currentRadius * sinf(angle);
            }
        
            // Draw all segments in one efficient batch
            DrawLineStrip(points.data(), segments + 1, color);
        }          
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

    void Init() {
        if (initialized) return;
    
        quad = GenMeshPlane(1.0f, 1.0f, 1, 1);
        material = LoadMaterialDefault();
    
        // Enable instancing
        material.shader = LoadShader(
            0,
            TextFormat("assets/shaders/instance.fs")
        );
    
        initialized = true;
    }
    
    void Clear() {
        instances.clear();
    }
    
    void Submit(const InstanceData& inst) {
        instances.push_back(inst);
    }
    
    void Flush() {
        if (instances.empty()) return;
    
        // DEPTH SORT (z + y)
        std::sort(instances.begin(), instances.end(),
            [](const InstanceData& a, const InstanceData& b) {
                if (a.depth != b.depth)
                    return a.depth < b.depth;
                return a.pos.y < b.pos.y;
            });
    
        // Upload instance buffer
        std::vector<Matrix> transforms(instances.size());
        std::vector<Color> colors(instances.size());
    
        for (size_t i = 0; i < instances.size(); i++) {
            transforms[i] = MatrixMultiply(
                MatrixScale(instances[i].size.x, instances[i].size.y, 1.0f),
                MatrixTranslate(instances[i].pos.x, instances[i].pos.y, 0.0f)
            );
            colors[i] = instances[i].color;
        }
    
        UpdateMeshBuffer(quad, 4, transforms.data(),
                         transforms.size() * sizeof(Matrix), 0);
    
        rlEnableShader(material.shader.id);
        DrawMeshInstanced(quad, material,
                          transforms.data(),
                          transforms.size());
        rlDisableShader();
    }
};

extern FunctionManager functions;
extern InstanceBatch instanceBatch;
