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
public:
	GLFWwindow* window;
	int WIDTH;
	int HEIGHT;
	float view_x;
	float view_y;
	Renderer* renderer;
	glm::dvec2 gravityField = glm::dvec2(0.0f, 10.0f);
	Engine(GLFWwindow* win, Renderer* renderer);
	Engine();
	void addObject(PhysObject* obj);
	void updateSim(double dt);
	std::vector<PhysObject> objects;
private:
	void pushIntoScreenBounds(PhysObject& obj);
};