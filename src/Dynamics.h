#pragma once
#include <glm/glm.hpp>
#include "PhysObject.h"
#include "PhysTypes.h"
void applyForce(PhysObject& obj, glm::dvec2 force, glm::dvec2 fPos, double dt);
void applyImpulse(PhysObject& A, PhysObject& B, ContactPoint& point, glm::dvec2 J);
void collide2D(Manifold& manifold, double dt);
void preStep(Manifold& manifold);
void correctPos(Manifold& manifold);