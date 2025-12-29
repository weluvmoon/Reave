#include "include/enemies.h"
#include "include/entities.h"

void Walker::Jump() {
    vel.x /= 2;
    vel.y = jumpV;
}

void Walker::Dash(float s, Vector2 tg) {
    if (pos.x < tg.x)
        vel.x = +s;
    if (pos.x > tg.x)
        vel.x = -s;
}

void Walker::Evade(float s, Vector2 tg) {
    if (pos.x < tg.x)
        vel.x = -s;
    if (pos.x > tg.x)
        vel.x = +s;
}
