#pragma once
#include <glm/glm.hpp>
#include "PhysObject.h"
#include "PhysTypes.h"
void applyForce(PhysObject& obj, glm::dvec2 force, glm::dvec2 fPos, double dt);

void collide2D(PhysObject& objA, PhysObject& objB, Manifold& manifold);