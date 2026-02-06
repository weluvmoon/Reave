#include "entities.h"
#include <cmath>
#include <cstddef>

void CharacterSystem(EntityManager &em, size_t i);
void CharacterDrawing(EntityManager &em, size_t i);

void CharacterScaleJuice(EntityManager &em, size_t i);

void CharacterStates(EntityManager &em, size_t i);
void CharacterMovement(EntityManager &em, size_t i);

void CharacterJump(EntityManager &em, size_t i);
void CharacterDash(EntityManager &em, size_t i);
void CharacterTricks(EntityManager &em, size_t i);
