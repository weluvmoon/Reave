#include "include/character.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include <cmath>
#include <raylib.h>
#include <raymath.h>

Vector2 inputDirection{0, 0};

void CharacterSystem(EntityManager &em, size_t i) {

    if (em.rendering.typeID[i] != EntityRegistry["CHARACTER"])
        return;
    auto &v = em.vars[i];

    if (!em.physics.initialized[i]) {
        v.values["LOCK_TIME"] = 0.0f;

        v.values["ACEL"] = 20.0f;
        v.values["DCEL"] = 50.0f;
        v.values["MAX_SPEED"] = 500.0f;
        v.values["GRAV"] = 18.0f;
        v.values["GRAV_A"] = 1.0f;
        v.values["GRAV_N"] = 18.0f;
        v.values["GRAV_M"] = 40.0f;
        v.values["JUMP_VAR"] = -750.0f;
        v.values["DASH_VAR"] = 750.0f;

        v.values["CAN_JUMP"] = false;
        v.values["CAN_WALL_JUMP"] = false;

        v.values["HAS_WALL_JUMPED"] = true;
        v.values["HAS_DASHED"] = false;

        v.values["COYOTE_TIME"] = 0.0f; // Current timer
        v.values["COYOTE_MAX"] = 0.2f;
        v.values["JUMP_BUFFER"] = 0.0f; // Current timer
        v.values["JUMP_BUFFER_MAX"] = 0.1f;
        v.values["DASH_COOLDOWN"] = 0.0f;
        v.values["DASH_DURATION"] = 0.0f;

        em.rendering.col[i] = VIOLET;
        em.physics.gravity[i] = v.values["GRAV_N"];
        em.physics.initialized[i] = true;
    }

    CollisionResult col = cS.CheckCollisions(em, i);

    if (col.hit) {
        if (col.typeID == EntityTys::TYTILE) {
            em.stats.health[i] -= 0.1f;
        }
    }

    CharacterMovement(em, i);
}

void CharacterMovement(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;
    float dt = GetFrameTime();

    // Standard input and gravity logic...
    inputDirection = {0, 0};
    if (IsKeyDown(KEY_MOVE_LEFT))
        inputDirection.x = -1.0f;
    if (IsKeyDown(KEY_MOVE_RIGHT))
        inputDirection.x = 1.0f;
    if (IsKeyDown(KEY_MOVE_UP))
        inputDirection.y = -1.0f;
    if (IsKeyDown(KEY_MOVE_DOWN))
        inputDirection.y = 1.0f;

    if (Vector2Length(inputDirection) > 0)
        inputDirection = Vector2Normalize(inputDirection);

    // Apply gravity ONLY if not dashing
    if (v["DASH_DURATION"] <= 0) {
        em.physics.gravity[i] = v["GRAV"];
        if (em.physics.grounded[i])
            v["GRAV"] = v["GRAV_N"];
        else if (v["GRAV"] <= v["GRAV_M"])
            v["GRAV"] += v["GRAV_A"];
    } else {
        em.physics.gravity[i] = 0; // Freeze gravity during dash
    }

    v["LOCK_TIME"] -= dt;

    // Horizontal Movement (Skipped if dashing for "Locked" dash feel)
    if (v["LOCK_TIME"] <= 0) {
        float &velX = em.physics.vel[i].x;
        if (inputDirection.x != 0) {
            float projectedSpeed = velX * inputDirection.x;
            int sgnX = (velX > 0) - (velX < 0);

            float currentAccel = (sgnX == (int)inputDirection.x)
                                     ? v["ACEL"]
                                     : (v["ACEL"] * 5.0f);

            if (projectedSpeed < v["MAX_SPEED"]) {
                velX += currentAccel * inputDirection.x;
                if (velX * inputDirection.x > v["MAX_SPEED"]) {
                    velX = v["MAX_SPEED"] * inputDirection.x;
                }
            }
        } else {
            velX *= 0.8f;
            if (std::abs(velX) < 0.01f)
                velX = 0.0f;
        }
    }

    CharacterJump(em, i);
    CharacterDash(em, i);
}

void CharacterJump(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;
    float dt = GetFrameTime();

    if (em.physics.grounded[i]) {
        v["COYOTE_TIME"] = v["COYOTE_MAX"];

        v["HAS_WALL_JUMPED"] = false;
    } else {
        v["COYOTE_TIME"] -= dt; // Counts down when in the air
    }

    if (em.physics.walled[i]) {
        v["CAN_WALL_JUMP"] = true;
    } else {
        v["CAN_WALL_JUMP"] = false;
    }

    // --- JUMP BUFFER LOGIC ---
    if (IsKeyPressed(KEY_JUMP)) {
        v["JUMP_BUFFER"] = v["JUMP_BUFFER_MAX"];
    } else {
        v["JUMP_BUFFER"] -= dt;
    }

    // --- Standard Jump (with Coyote and Buffer) ---
    if (v["JUMP_BUFFER"] > 0 && v["COYOTE_TIME"] > 0) {
        em.physics.vel[i].y = v["JUMP_VAR"];
        v["COYOTE_TIME"] = 0;
        v["JUMP_BUFFER"] = 0;
    } else if (IsKeyReleased(KEY_JUMP) &&
               em.physics.vel[i].y <= v["JUMP_VAR"] / 5.0f &&
               v["CAN_JUMP"] < 0.5f) {
        em.physics.vel[i].y = v["JUMP_VAR"] / 5.0f;
    }

    // --- Wall Jump (Directional Inference) ---
    if (IsKeyPressed(KEY_JUMP) && v["CAN_WALL_JUMP"] &&
        !em.physics.grounded[i]) {

        float pushDir = (inputDirection.x != 0) ? -inputDirection.x : 0;
        if (pushDir == 0) {
            pushDir = (em.physics.vel[i].x > 0) ? -1.0f : 1.0f;
        }

        em.physics.vel[i].x = pushDir * std::abs(v["JUMP_VAR"]);
        em.physics.vel[i].y = v["JUMP_VAR"];

        v["JUMP_BUFFER"] = 0;
        v["HAS_WALL_JUMPED"] = true;

        v["LOCK_TIME"] = 0.01f;
    } else if (IsKeyReleased(KEY_JUMP) && v["CAN_WALL_JUMP"] &&
               !v["HAS_WALL_JUMPED"]) {
        em.physics.vel[i].x = (v["JUMP_VAR"] * 1.0f) * inputDirection.x;
        em.physics.vel[i].y = v["JUMP_VAR"] * 1.3f;

        v["JUMP_BUFFER"] = 0;
        v["CAN_WALL_JUMP"] = false;
        v["HAS_WALL_JUMPED"] = true;

        v["LOCK_TIME"] = 0.01f;
    }
}

void CharacterDash(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;
    float dt = GetFrameTime();

    if (em.physics.grounded[i]) {
        v["CAN_DASH"] = true;
        v["HAS_DASHED"] = false;
    }

    v["DASH_DURATION"] -= dt;

    if (IsKeyPressed(KEY_DASH) && v["CAN_DASH"]) {
        Vector2 dashDir = inputDirection;
        if (Vector2Length(dashDir) == 0)
            dashDir.x = 1.0f;

        em.physics.vel[i].x = dashDir.x * v["DASH_VAR"];
        em.physics.vel[i].y = dashDir.y * v["DASH_VAR"];

        v["CAN_DASH"] = false;
        v["HAS_DASHED"] = true;

        v["LOCK_TIME"] = 0.15f;
        v["DASH_DURATION"] = 0.15f;
    }
}
