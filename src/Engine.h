#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "PhysObject.h"
#include "resource_manager.h"
#include "Renderer.h"

class Engine {
private:
	void collide(PhysObject& obj1, PhysObject& obj2);
	enum Direction {
		UP,
		RIGHT,
		DOWN,
		LEFT
	};
	Direction getBestSideClipMatching(PhysObject& obj1, PhysObject& obj2);
public:
	GLFWwindow* window;
	int WIDTH;
	int HEIGHT;
	float view_x;
	float view_y;
	Renderer* renderer;

	Engine(GLFWwindow* win, Renderer* renderer);
	Engine();
	void addObject(PhysObject* obj);
	int checkCollision(PhysObject& obj1, PhysObject& obj2);
	void updateSim(float dt);
	std::vector<PhysObject> objects;
	void resetCollisionCheckFlags();
	void clipBoxes(PhysObject& obj1, PhysObject& obj2, Direction normDir);

};