#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Renderer.h"
class PhysObject {
public:
	/// (x, y) position. +x = Right, +y = Up
	glm::dvec2 pos;
	// Size of box
	glm::dvec2 size;
	/// Rotation CCW, radians.
	double angle;
	/// Angular Velocity; Positive = CCW
	double angularVelocity=0.0f;
	double angAccleration = 0.0f;
	/// (x, y) velocity. Direction is same as pos vector.
	glm::dvec2 velocity;
	/// RGB Color
	glm::vec3 color;
	int collisionState = false;
	int collisionChecked2 = false;
	/// Force vector. Directions are same as pos vector.
	glm::dvec2 force;
	/// Mass of entire object.
	double mass;
	/// 1 / mass
	double invMass;
	/// 1 / I; used for dynamics calculations.
	double invInertia;
	/// Whether this object can be moved.
	bool isStatic = false;

	std::vector<glm::dvec2> getVertices() const;

	PhysObject();
	PhysObject(glm::dvec2 pos, glm::dvec2 size, glm::dvec2 velocity = glm::dvec2(0.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
	PhysObject(glm::dvec2 pos, glm::dvec2 size, double mass, double rotation, glm::dvec2 velocity = glm::dvec2(0.0f, 0.0f));
	PhysObject(glm::dvec2 pos, glm::dvec2 size, double rotation, bool isStatic);
	void init(glm::dvec2 pos, glm::dvec2 size, glm::dvec2 velocity = glm::dvec2(0.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
	void update(double dt);
	void preUpdate(double dt);
	glm::dvec2 getPointVelocity(glm::dvec2 r);
	void render(Renderer* renderer) const;
	glm::dvec2 center() {
		return glm::dvec2(pos.x + 0.5f * size.x, pos.y + 0.5f * size.y);
	}
	// set the inverse mass / inertia
	void calcInverses();
};

class Box2D : public PhysObject {

};