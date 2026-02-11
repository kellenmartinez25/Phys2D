#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Renderer.h"
class PhysObject {
public:
	glm::vec2 pos;
	glm::vec2 size;
	float rotation;
	glm::vec2 velocity;
	glm::vec3 color;
	PhysObject();
	PhysObject(glm::vec2 pos, glm::vec2 size, glm::vec2 velocity = glm::vec2(0.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
	void init(glm::vec2 pos, glm::vec2 size, glm::vec2 velocity = glm::vec2(0.0f, 0.0f), glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f));
	void update(float dt);
	void render(Renderer* renderer) const;
	glm::vec2 center();
	int collisionState = false;
	int collisionChecked2 = false;

};

class Box2D : public PhysObject {

};