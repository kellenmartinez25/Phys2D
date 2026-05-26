#pragma once
#include <glm/glm.hpp>
#include <vector>

// Represents a contact point between two objects
struct ContactPoint {
	// pos in world space
	glm::dvec2 pos;
	// amount of penetration
	double penetration;
	// vector from center of A -> pos
	glm::dvec2 rA;
	// vec from center of B -> pos
	glm::dvec2 rB;
	
	double normImpulse = 0.0;
	double tangentImpulse = 0.0;
};
// Represents a contact manifold; a collection of 0-2 contact points.
// 0 points means no contact, 1 means a single point, 2 means between 2 edges.
struct Manifold {
public:
	// Normal of the reference face on the collision.
	glm::dvec2 normal;
	ContactPoint points[2];
	int count;
	// Adds a new vector to the Manifold
	// Behaves the same as std::vector.push_back()
	// Returns whether v was successfully added.
	bool add(glm::dvec2 v);
	glm::dvec2 get(int index);
};