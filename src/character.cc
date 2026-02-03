#include "include/character.h"
#include "include/constants.h"
#include "include/data.h"
#include "include/entities.h"
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
        v.set("JUMP_VAR", -750.0f);
        v.set("DASH_VAR", 750.0f);

        v.set("TRICK_TYPE", 1);
        v.set("TRICK_METER", 100.0f);

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

    CharacterMovement(em, i);
}

void CharacterDrawing(EntityManager &em, size_t i) {
	if (em.rendering.typeID[i] != EntityRegistry["CHARACTER"])
        return;
    auto &v = em.vars[i];
    
	const Rectangle &r = {em.physics.pos[i].x, em.physics.pos[i].y - 25, v.get("TRICK_METER"), 10};
    
    Texture2D pixelTex = am.textures[TEX_DEF];
	DrawTexturePro(pixelTex, {0, 0, 1, 1}, r, {0, 0}, 0.0f,
					em.rendering.col[i]);
}

void CharacterMovement(EntityManager &em, size_t i) {
    auto &v = em.vars[i];
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

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
    if (v.get("DASH_DURATION") <= 0) {
        em.physics.gravity[i] = v.get("GRAV");
        if (em.physics.grounded[i])
            v.set("GRAV", v.get("GRAV_N"));
        else if (v.get("GRAV") <= v.get("GRAV_M"))
            v.add("GRAV", v.get("GRAV_A"));
    } else {
        em.physics.gravity[i] = 0;
    }

    // 2. Wall Slide Logic
    bool isWalled = em.physics.walled[i];
    if (isWalled && !em.physics.grounded[i] && velY > 0) {
        velY *= 0.7f;
    }

    v.sub("LOCK_TIME", dt);

    // Horizontal Movement (Skipped if dashing for "Locked" dash feel)
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
    auto &v = em.vars[i].values;
    float dt = GetFrameTime();
    float &velX = em.physics.vel[i].x;
    float &velY = em.physics.vel[i].y;

    // Update Timers
    if (em.physics.grounded[i]) {
        v["COYOTE_TIME"] = v["COYOTE_MAX"];
        v["HAS_WALL_JUMPED"] = false;
    } else {
        v["COYOTE_TIME"] -= dt;
    }

    if (IsKeyPressed(KEY_JUMP))
        v["JUMP_BUFFER"] = v["JUMP_BUFFER_MAX"];
    else
        v["JUMP_BUFFER"] -= dt;

    //  Standard Jump
    if (v["JUMP_BUFFER"] > 0 && v["COYOTE_TIME"] > 0) {
        velY = v["JUMP_VAR"];
        v["COYOTE_TIME"] = 0;
        v["JUMP_BUFFER"] = 0;
    }

    // Trigger Jump
    if (v["JUMP_BUFFER"] > 0 &&
        (v["COYOTE_TIME"] > 0 || em.physics.grounded[i])) {

        // --- SUPER JUMP LOGIC ---
        if (v["DASH_DURATION"] > 0) {
            // Preserve horizontal dash speed but allow vertical jump
            velY = v["JUMP_VAR"];
            velX *= 1.5f;
            v["DASH_DURATION"] = 0;
        } else {
            // Normal Jump
            velY = v["JUMP_VAR"];
        }

        v["COYOTE_TIME"] = 0;
        v["JUMP_BUFFER"] = 0;
    }

    // IMPROVED WALL JUMP
    if (v["JUMP_BUFFER"] > 0 && em.physics.walled[i] &&
        !em.physics.grounded[i]) {
        float wallSide = (velX > 0 || inputDirection.x > 0) ? 1.0f : -1.0f;

        velX = -wallSide * std::abs(v["MAX_SPEED"] * 1.5f);
        velY = v["JUMP_VAR"] * 0.9f;

        v["JUMP_BUFFER"] = 0;
        v["LOCK_TIME"] = 0.1f;
        v["HAS_WALL_JUMPED"] = true;
    }

    if (IsKeyReleased(KEY_JUMP) && velY < 0) {
        velY *= 0.5f;
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

	v.add("TRICK_METER", dt);
    
    DrawText("Hello", em.physics.pos[i].x, em.physics.pos[i].y, 12, BLACK);

    if (IsKeyPressed(KEY_TRICK_A) && v.get("TRICK_METER") > 0.0f) {
        // Trick Type 0
        if (v.get("TRICK_TYPE") == 0) {
            velY = v.get("JUMP_VAR");
            v.set("COYOTE_TIME", 0.0f);
            v.set("JUMP_BUFFER", 0.0f);

            v.sub("TRICK_METER", 10.0f);
        }

        // Trick Type 1
        if (v.get("TRICK_TYPE") == 1) {
            velY = v.get("JUMP_VAR") * std::abs(velX) / 120.0f;
            v.set("COYOTE_TIME", 0.0f);
            v.set("JUMP_BUFFER", 0.0f);

            v.sub("TRICK_METER", 10.0f);
        }
    }
}
