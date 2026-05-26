#include "Engine.h"
#include "Collision.h"
#include "Dynamics.h"
#include "DebugTools.h"
#include <vector>
#include <utility>


Engine::Engine(GLFWwindow* win, Renderer* renderer) :
	WIDTH(800),
	HEIGHT(600),
	view_x(800),
	view_y(600),
	renderer(renderer),
	window(win),
	objects(1)
{

}
Engine::Engine() :
	WIDTH(800),
	HEIGHT(600),
	view_x(800),
	view_y(600)
{}
// add a physobject to the simulation
void Engine::addObject(PhysObject* obj)
{
	objects.push_back(*obj);
}

// make sure an object doesnt go off screen bounds
void Engine::pushIntoScreenBounds(PhysObject& obj) {
	// also keep within screen boundaries
	if (obj.pos.y + obj.size.y > HEIGHT) {
		obj.pos.y = HEIGHT - obj.size.y;
		obj.velocity.y *= -1.0f;
	}
	else if (obj.pos.y <= 0.0f) {
		obj.pos.y = 0.0f;
		obj.velocity.y *= -1.0f;
	}
	if (obj.pos.x + obj.size.x > WIDTH) {
		obj.pos.x = WIDTH - obj.size.x;
		obj.velocity.x *= -1.0f;
	}
	else if (obj.pos.x < 0.0f) {
		obj.pos.x = 0.0f;
		obj.velocity.x *= -1.0f;
	}
}

// update positions and velocities
void Engine::updateSim(double dt) {
	// Iterate through each object
	// generate a pair of each object to test upon:
	std::vector<std::pair<PhysObject&, PhysObject&>> pairs;
	// pair each object with one another
	for (int i = 0; i < objects.size(); i++) {
		for (int j = i + 1; j < objects.size(); j++) {
			pairs.emplace_back(objects[i], objects[j]);
		}
	}
	// the contact manifolds we will generate
	std::vector<Manifold> manifolds;
	// iterate through each pair
	for (auto p : pairs) {
		// the new manifold
		Manifold m;
		// generate the contact manifold, might be 0 objects.
		genManifold(p.first, p.second, m);
		manifolds.push_back(m);
	}

	for (auto& m : manifolds) {
		
		// collide on each manifold
		if (m.count == 0) continue;
		for (int i = 0; i < m.count; i++) {
			DebugDraw::get().drawPoint(m.get(i), glm::vec3(1.0f, 0.0f, 0.0f));
		}

		collide2D(*m.A, *m.B, m);
	}
	// update each object with new values
	for (auto& obj : objects) {
		obj.update(dt);
		pushIntoScreenBounds(obj);
		obj.render(renderer);
	}
	
}
