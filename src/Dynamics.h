#pragma once
#include <glm/glm.hpp>
#include "PhysObject.h"
#include "Dynamics.h"

void applyForce(PhysObject& obj, glm::dvec2 force, glm::dvec2 fPos, double dt);

void collide2D(PhysObject& obj1, PhysObject& obj2, ClippedPoints* contactPoints, glm::dvec2 sepNorm);