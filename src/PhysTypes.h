#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "PhysObject.h"
// Represents a contact point between two objects
struct ContactPoint {
	// pos in world space
	glm::dvec2 pos;
	// vector from center of A -> pos
	glm::dvec2 rA;
	// vec from center of B -> pos
	glm::dvec2 rB;
	// amount of penetration
	double penetration;
	
	double effectiveMass;

	double normImpulse = 0.0;
};
// Represents a contact manifold; a collection of 0-2 contact points.
// 0 points means no contact, 1 means a single point, 2 means between 2 edges.
struct Manifold {
public:
	// Normal of the reference face on the collision. should be A->B
	glm::dvec2 normal;
	// May contain 1-2 contact points.
	ContactPoint points[2];
	// # of contact points.
	int count = 0;

	// reference
	PhysObject* A;
	// incident
	PhysObject* B;

};