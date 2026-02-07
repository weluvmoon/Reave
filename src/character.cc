#include "include/character.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
#include "include/function.h"
#include <cmath>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>

Vector2 inputDirection{0, 0};

enum TrickTypes { TRICK_DOUBLE_JUMP, TRICK_TRIPLE_JUMP, TRICK_COUNT };

void CharacterSystem(EntityManager &em, size_t i) {

    if (em.rendering.typeID[i] != EntityRegistry["CHARACTER"])
        return;
    auto &v = em.vars[i];

    if (!em.physics.initialized[i]) {
        v.set("LOCK_TIME", 0.0f);

        v.set("ACEL", 20.0f);
        v.set("DCEL", 50.0f);
        v.set("MAX_SPEED", 500.0f);
        v.set("GRAV", 18.0f);
        v.set("GRAV_A", 1.0f);
        v.set("GRAV_N", 18.0f);
        v.set("GRAV_M", 40.0f);
        v.set("JUMP_VAR", 750.0f);
        v.set("DASH_VAR", 950.0f);

        v.set("TRICK_TYPE", 0);
        v.set("TRICK_METER", 100.0f);
        v.set("TRICK_METER_MAX", 100.0f);

        v.set("CAN_JUMP", false);
        v.set("CAN_WALL_JUMP", false);

        v.set("HAS_WALL_JUMPED", true);
        v.set("HAS_DASHED", false);

        v.set("COYOTE_TIME", 0.0f);
        v.set("COYOTE_MAX", 0.2f);
        v.set("JUMP_BUFFER", 0.0f);
        v.set("JUMP_BUFFER_MAX", 0.1f);
        v.set("DASH_COOLDOWN", 0.0f);
        v.set("DASH_DURATION", 0.0f);

        v.set("SCALE_TWEEN_TIME", 1.0f);
        v.set("SCALE_TWEEN_DURATION", 0.45f);
        v.set("WAS_IN_AIR", 0.0f);

        em.rendering.col[i] = VIOLET;
        em.physics.gravity[i] = v.values["GRAV_N"];
        em.physics.initialized[i] = true;
    }

    CollisionResult col = cS.CheckCollisions(em, em.physics.rect[i], i);

    if (col.hit) {
        if (col.typeID == EntityTys::TYTILE) {
            em.stats.health[i] -= 0.1f;
        }
    }

    CharacterScaleJuice(em, i);
    CharacterMovement(em, i);
}

void CharacterDrawing(EntityManager &em, size_t i) {
    if (em.rendering.typeID[i] != EntityRegistry["CHARACTER"])
        return;
    auto &v = em.vars[i];
    Texture2D pixelTex = am.textures[TEX_DEF];
    float dt = GetFrameTime();

    float fTime = v.get("FLASH_TIME");
    Color mainCol = em.rendering.col[i];
    if (fTime > 0) {
        mainCol = WHITE;
        v.sub("FLASH_TIME", dt);
    }

    float sX = em.physics.scale[i].x;
    float sY = em.physics.scale[i].y;
    float rot = em.rendering.rotation[i];
    Vector2 vel = em.physics.vel[i];
    float speed = Vector2Length(vel);
    float maxSpeed = v.get("MAX_SPEED");

    Rectangle dest = {em.physics.pos[i].x + em.physics.siz[i].x / 2.0f,
                      em.physics.pos[i].y + em.physics.siz[i].y,
                      em.physics.siz[i].x * sX, em.physics.siz[i].y * sY};
    Vector2 origin = {(em.physics.siz[i].x * sX) / 2.0f,
                      em.physics.siz[i].y * sY};

    if (fTime <= 0 && speed > maxSpeed) {
        float intensity = (speed - maxSpeed) / maxSpeed;
        float jitterX = (float)GetRandomValue(-100, 100) * 0.06f * intensity;
        float jitterY = (float)GetRandomValue(-100, 100) * 0.06f * intensity;

        DrawTexturePro(
            pixelTex, {0, 0, 1, 1},
            {dest.x + jitterX, dest.y + jitterY, dest.width, dest.height},
            origin, rot, Fade(RED, 0.5f));

        DrawTexturePro(
            pixelTex, {0, 0, 1, 1},
            {dest.x - jitterX, dest.y - jitterY, dest.width, dest.height},
            origin, rot, Fade(BLUE, 0.5f));
    }

    DrawTexturePro(pixelTex, {0, 0, 1, 1}, dest, origin, rot, mainCol);

    // --- HUD Elements ---
    const Rectangle &healthRectY = {em.physics.pos[i].x - 15.0f,
                                    em.physics.pos[i].y -
                                        (em.stats.health[i] / 4.0f) +
                                        (em.physics.siz[i].y / 2.0f),
                                    5.0f, em.stats.health[i] / 2.0f};
    const Rectangle &healthRectOutlineY = {healthRectY.x - 1.0f,
                                           healthRectY.y - 1.0f, 7.0f,
                                           healthRectY.height + 2.0f};

    DrawTexturePro(pixelTex, {0, 0, 1, 1}, healthRectOutlineY, {0, 0}, 0.0f,
                   BLACK);
    DrawTexturePro(pixelTex, {0, 0, 1, 1}, healthRectY, {0, 0}, 0.0f, RED);

    // --- Trick Meter ---
    const Rectangle &trickRectY = {em.physics.pos[i].x - 25.0f,
                                   em.physics.pos[i].y -
                                       (v.get("TRICK_METER") / 4.0f) +
                                       (em.physics.siz[i].y / 2.0f),
                                   5.0f, v.get("TRICK_METER") / 2.0f};
    const Rectangle &trickRectOutlineY = {trickRectY.x - 1.0f,
                                          trickRectY.y - 1.0f, 7.0f,
                                          trickRectY.height + 2.0f};

    DrawTexturePro(pixelTex, {0, 0, 1, 1}, trickRectOutlineY, {0, 0}, 0.0f,
                   BLACK);
    DrawTexturePro(pixelTex, {0, 0, 1, 1}, trickRectY, {0, 0}, 0.0f,
                   em.rendering.col[i]);
}

void CharacterScaleJuice(EntityManager &em, size_t i) {
    auto &v = em.vars[i];
    auto &scale = em.physics.scale[i];
    auto &rotation = em.rendering.rotation[i];
    auto &vel = em.physics.vel[i];
    float dt = GetFrameTime();

    bool isGrounded = v.get("IS_GROUNDED") > 0.5f;
    bool isWalled = em.physics.walled[i];
    float tweenTime = v.get("SCALE_TWEEN_TIME");
    float duration = v.get("SCALE_TWEEN_DURATION");

    if (isGrounded) {
        if (v.get("WAS_IN_AIR") > 0.5f) {
            float airTime = v.get("AIR_TIME");

            v.set("SCALE_TWEEN_TIME", 0.0f);
            v.set("SCALE_TWEEN_DURATION", 0.45f);

            float squashIntensity = (airTime > 1.0f) ? 0.45f : 0.65f;
            v.set("SCALE_START_VAL", squashIntensity);

            v.set("WAS_IN_AIR", 0.0f);
            v.set("AIR_TIME", 0.0f);
            v.set("IS_SPINNING", 0.0f);
        }
    } else {
        v.set("WAS_IN_AIR", 1.0f);
        v.add("AIR_TIME", dt);

        if (isWalled && v.get("WAS_ON_WALL") < 0.5f) {
            v.set("SCALE_TWEEN_TIME", 0.0f);
            v.set("SCALE_TWEEN_DURATION", 0.35f);
            v.set("SCALE_START_VAL", 1.4f);
            v.set("WAS_ON_WALL", 1.0f);
        }
    }

    if (!isWalled)
        v.set("WAS_ON_WALL", 0.0f);

    if (tweenTime < duration) {
        v.add("SCALE_TWEEN_TIME", dt);
        float t = fminf(v.get("SCALE_TWEEN_TIME"), duration);
        float startS = v.get("SCALE_START_VAL");

        scale.y = FunctionManager::Ease::OutElastic(t, startS, 1.0f - startS,
                                                    duration);

        if (v.get("IS_SPINNING") > 0.5f) {
            rotation =
                FunctionManager::Ease::OutQuad(t, 0.0f, 720.0f, duration);
        } else if (startS > 1.1f) {
            float shakeAmp =
                FunctionManager::Ease::OutQuad(t, 15.0f, -15.0f, duration);
            rotation = sinf(t * 25.0f) * shakeAmp;
        }

        if (t >= duration)
            v.set("IS_SPINNING", 0.0f);
    } else {
        float velocityStretch = fabsf(vel.y) * 0.0003f;
        scale.y = Lerp(scale.y, 1.0f + velocityStretch, dt * 10.0f);
        rotation = Lerp(rotation, 0.0f, dt * 10.0f);
    }

    if (scale.y != 0)
        scale.x = 1.0f / scale.y;
}

void CharacterMovement(EntityManager &em, size_t i) {
    auto &v = em.vars[i];
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

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

    if (v.get("DASH_DURATION") <= 0) {
        em.physics.gravity[i] = v.get("GRAV");
        if (em.physics.grounded[i])
            v.set("GRAV", v.get("GRAV_N"));
        else if (v.get("GRAV") <= v.get("GRAV_M"))
            v.add("GRAV", v.get("GRAV_A"));
    } else {
        em.physics.gravity[i] = 0;
    }

    bool isWalled = em.physics.walled[i];
    if (isWalled && !em.physics.grounded[i] && velY > 0) {
        velY *= 0.7f;
    }

    v.sub("LOCK_TIME", dt);

    if (v.get("LOCK_TIME") <= 0) {
        if (inputDirection.x != 0) {
            float projectedSpeed = velX * inputDirection.x;
            int sgnX = (velX > 0) - (velX < 0);

            float currentAccel = (sgnX == (int)inputDirection.x)
                                     ? v.get("ACEL")
                                     : (v.values["ACEL"] * 5.0f);

            if (projectedSpeed < v.get("MAX_SPEED")) {
                velX += currentAccel * inputDirection.x;
                if (velX * inputDirection.x > v.get("MAX_SPEED")) {
                    velX = v.get("MAX_SPEED") * inputDirection.x;
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
    CharacterTricks(em, i);
}

void CharacterJump(EntityManager &em, size_t i) {
    auto &v = em.vars[i];
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

    if (em.physics.grounded[i]) {
        v.set("COYOTE_TIME", v.get("COYOTE_MAX"));
        v.set("HAS_WALL_JUMPED", 0.0f);
    } else {
        v.sub("COYOTE_TIME", dt);
    }

    if (IsKeyPressed(KEY_JUMP))
        v.set("JUMP_BUFFER", v.get("JUMP_BUFFER_MAX"));
    else
        v.sub("JUMP_BUFFER", dt);

    if (v.get("JUMP_BUFFER") > 0 && em.physics.walled[i] &&
        !em.physics.grounded[i]) {
        float kickDir = (inputDirection.x != 0) ? -inputDirection.x
                                                : (velX > 0 ? -1.0f : 1.0f);

        velX = kickDir * v.get("MAX_SPEED");
        velY = -v.get("JUMP_VAR") * 0.9f;

        v.set("SCALE_TWEEN_TIME", 0.0f);
        v.set("SCALE_TWEEN_DURATION", 0.4f);
        v.set("SCALE_START_VAL", 1.4f);
        v.set("WALL_KICK_TIME", 0.0f);
        v.set("WALL_KICK_SIDE", (velX > 0) ? 1.0f : -1.0f);

        v.set("JUMP_BUFFER", 0);
        v.set("LOCK_TIME", 0.15f);
        v.set("HAS_WALL_JUMPED", 1.0f);

        v.set("SCALE_TWEEN_TIME", 0.0f);
        v.set("SCALE_START_VAL", 1.4f);

        return;
    }

    if (v.get("JUMP_BUFFER") > 0 && v.get("COYOTE_TIME") > 0) {
        if (v.get("DASH_DURATION") > 0) {
            velY = -v.get("DASH_VAR");
            velX *= 1.5f;
            v.set("DASH_DURATION", 0);
        } else {
            velY = -v.get("JUMP_VAR");
        }

        v.set("SCALE_TWEEN_TIME", 0.0f);
        v.set("SCALE_TWEEN_DURATION", 0.35f);
        v.set("SCALE_START_VAL", 1.3f);

        v.set("COYOTE_TIME", 0);
        v.set("JUMP_BUFFER", 0);
    }

    if (IsKeyReleased(KEY_JUMP) && velY < 0) {
        velY = 0.0f;
    }
}

void CharacterDash(EntityManager &em, size_t i) {
    auto &v = em.vars[i].values;
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

    if (em.physics.grounded[i]) {
        v["CAN_DASH"] = true;
        v["HAS_DASHED"] = false;
    }

    v["DASH_DURATION"] -= dt;

    if (IsKeyPressed(KEY_DASH) && v["CAN_DASH"]) {
        Vector2 dashDir = inputDirection;

        if (Vector2Length(dashDir) == 0)
            dashDir.x = (em.physics.vel[i].x >= 0) ? 1.0f : -1.0f;

        velX = dashDir.x * v["DASH_VAR"];
        velY = dashDir.y * v["DASH_VAR"];

        v["CAN_DASH"] = false;
        v["HAS_DASHED"] = true;
        v["DASH_DURATION"] = 0.15f;
        v["LOCK_TIME"] = 0.15f;
    }
}

void CharacterTricks(EntityManager &em, size_t i) {
    auto &v = em.vars[i];
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

    if (v.get("TRICK_METER") < 0.0f)
        v.set("TRICK_METER", 0.0f);
    else if (v.get("TRICK_METER") < v.get("TRICK_METER_MAX"))
        v.add("TRICK_METER", (dt * 10.0f));

    float t = v.get("SCALE_TWEEN_TIME");
    float dur = v.get("SCALE_TWEEN_DURATION");
    float startVel = v.get("TRICK_START_VEL");

    if (t < dur && startVel != 0.0f) {
        if (v.get("TRICK_TYPE") == 0) {
            velY = FunctionManager::Ease::OutQuad(t, startVel, -startVel, dur);
        } else if (v.get("TRICK_TYPE") == 1) {
            float dir = v.get("TRICK_DIR");
            velX = FunctionManager::Ease::OutQuad(t, startVel * dir,
                                                  -(startVel * dir), dur);
            velY = 0;
        }
    }

    if (IsKeyPressed(KEY_TRICK_A) && v.get("TRICK_METER") >= 10.0f) {
        v.set("SCALE_TWEEN_TIME", 0.0f);
        v.set("SCALE_TWEEN_DURATION", 0.5f);
        v.sub("TRICK_METER", 25.0f);
        v.set("FLASH_TIME", 0.1f);

        if (v.get("TRICK_TYPE") == 0) {
            v.set("SCALE_START_VAL", 1.6f);
            velY = -v.get("JUMP_VAR");
        } else if (v.get("TRICK_TYPE") == 1) {
            v.set("TRICK_START_VEL", v.get("DASH_VAR") * 1.5f);
            v.set("TRICK_DIR", (velX >= 0) ? 1.0f : -1.0f);
            v.set("SCALE_START_VAL", 0.5f);

            v.set("IS_SPINNING", 1.0f);
        }
    }
}
