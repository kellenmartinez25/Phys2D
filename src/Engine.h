#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <utility>
#include <unordered_map>
#include "PhysTypes.h"
#include "PhysObject.h"
#include "resource_manager.h"
#include "Renderer.h"

typedef std::pair<PhysObject*, PhysObject*> PhysObjectPair;
struct PairHash
{
	std::size_t operator()(const PhysObjectPair& p) const {
		std::size_t h1 = std::hash<PhysObject*>()(p.first);
		std::size_t h2 = std::hash<PhysObject*>()(p.second);
		return h1 ^ (h2 << 1);
	}
};
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
	// Reset the engine and remove all objects
	void reset();
	// Add a PhysObject to the simulation
	void addObject(PhysObject* obj);
	// Update the simulation with a timestep dt
	void updateSim(double dt);
	std::vector<PhysObject> objects;

private:
	std::unordered_map<PhysObjectPair, Manifold, PairHash> manifoldCache;
};

inline PhysObjectPair MakePair(PhysObject* A, PhysObject* B) {
	return (A < B)
		? PhysObjectPair(A, B)
		: PhysObjectPair(B, A);
}
