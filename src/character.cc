#include "include/character.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include <cmath>
#include <raylib.h>
#include <raymath.h>

Vector2 inputDirection{0, 0};

void CharacterSystem(EntityManager &em, size_t i) {

    if (em.rendering.typeID[i] != EntityTys::TYCHARACTER)
        return;
    auto &v = em.vars[i];

    if (!em.physics.initialized[i]) {
        v.values["ACEL"] = 20.0f;
        v.values["DCEL"] = 50.0f;
        v.values["MAX_SPEED"] = 500.0f;
        v.values["GRAV_N"] = 18.0f;
        v.values["GRAV_M"] = 40.0f;
        v.values["GRAV"] = 18.0f;
        v.values["JUMP_VAR"] = -750.0f;
        v.values["DASH_VAR"] = 750.0f;

        v.values["CAN_JUMP"] = 0.0f;
        v.values["HAS_DASHED"] = 0.0f;

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

    if (em.physics.grounded[i]) {
        v["CAN_JUMP"] = true;
        v["CAN_DASH"] = true;
        v["HAS_JUMPED"] = false;
        v["HAS_DASHED"] = false;
        v["HAS_WALL_JUMPED"] = false;
    } else {
        v["CAN_JUMP"] = false;
    }

    if (em.physics.walled[i]) {
        v["CAN_WALL_JUMP"] = true;
    } else {
        v["CAN_WALL_JUMP"] = false;
    }

    inputDirection = {0, 0};
    if (IsKeyDown(KEY_MOVE_LEFT))
        inputDirection.x = -1.0f;
    if (IsKeyDown(KEY_MOVE_RIGHT))
        inputDirection.x = 1.0f;
    if (IsKeyDown(KEY_MOVE_UP))
        inputDirection.y = -1.0f;
    if (IsKeyDown(KEY_MOVE_DOWN))
        inputDirection.y = 1.0f;

    if (Vector2Length(inputDirection) > 0) {
        inputDirection = Vector2Normalize(inputDirection);
    }

    em.physics.gravity[i] = v["GRAV"];
    if (em.physics.grounded[i]) {
        v["GRAV"] = v["GRAV_N"];
    } else if (v["GRAV"] <= v["GRAV_M"]) {
        v["GRAV"] += 0.5f;
    }

    float &velX = em.physics.vel[i].x;
    if (inputDirection.x != 0) {
        float projectedSpeed = velX * inputDirection.x;
        int sgnX = (velX > 0) - (velX < 0);

        float currentAccel =
            (sgnX == (int)inputDirection.x) ? v["ACEL"] : (v["ACEL"] * 5.0f);

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

    CharacterJump(em, i);
    CharacterDash(em, i);
}

void CharacterJump(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;

    if (IsKeyPressed(KEY_JUMP) && v["CAN_JUMP"]) {
        em.physics.vel[i].y = v["JUMP_VAR"];
        v["CAN_JUMP"] = 0.0f;
        v["HAS_JUMPED"] = 1.0f;
    } else if (IsKeyReleased(KEY_JUMP) &&
               em.physics.vel[i].y <= v["JUMP_VAR"] / 5.0f &&
               v["CAN_JUMP"] < 0.5f) {
        em.physics.vel[i].y = v["JUMP_VAR"] / 5.0f;
    }

    if (IsKeyPressed(KEY_JUMP) && v["CAN_WALL_JUMP"]) {
        em.physics.vel[i].x = (v["JUMP_VAR"] * 1.0f) * inputDirection.x;
        em.physics.vel[i].y = v["JUMP_VAR"];

        v["CAN_WALL_JUMP"] = false;
        v["HAS_WALL_JUMPED"] = true;
    } else if (IsKeyReleased(KEY_JUMP) && v["CAN_WALL_JUMP"] &&
               !v["HAS_WALL_JUMPED"]) {
        em.physics.vel[i].x = (v["JUMP_VAR"] * 1.0f) * inputDirection.x;
        em.physics.vel[i].y = v["JUMP_VAR"] * 1.3f;

        v["CAN_WALL_JUMP"] = false;
        v["HAS_WALL_JUMPED"] = true;
    }
}

void CharacterDash(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;

    if (IsKeyPressed(KEY_DASH) && v["CAN_DASH"]) {
        em.physics.vel[i].x = v["DASH_VAR"] * inputDirection.x;
        em.physics.vel[i].y = v["DASH_VAR"] * inputDirection.y;

        v["CAN_DASH"] = false;
        v["HAS_DASHED"] = true;
    }
}
