#include "Engine.h"
#include "Collision.h"
#include "Dynamics.h"
constexpr float EPSILON = 0.5f;


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

// update positions and velocities
void Engine::updateSim(double dt)
{
	// Iterate through each object
	for (int i = 0; i < objects.size(); i++) {
		PhysObject& obj = objects[i]; // the one we're working with

		// Iterate against everything else
		for (int j = i + 1; j < objects.size(); j++) {
			if (j == i) continue;	// dont check the same one
			PhysObject& obj2 = objects[j];
			// if they're colliding and if it hasn't already been checked

			if (isColliding(obj, obj2)) {

				//std::cout << "Collision!" << std::endl;
				MTV* mt = getMTV(obj, obj2);
				glm::dvec2 sepNorm = obj2.center() - obj.center();
				double isPointingRight = glm::dot(sepNorm, mt->smallest) / glm::length(sepNorm);
				if (isPointingRight < 0.0)
					mt->smallest *= -1.0;
				Edge e1 = bestEdge(obj, mt->smallest);
				Edge e2 = bestEdge(obj2, -mt->smallest);
				ClippedPoints* cp = getClippedPoints(e1, e2, mt->smallest);
				



				collide2D(obj, obj2, cp, mt->smallest);
				// get them out of each other
				obj.pos = obj.pos - (mt->overlap - .01 > 0 ? mt->overlap - .01 : 0) * .2 * mt->smallest;
				delete cp;
				delete mt;

			}
		}
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
		// update position
		obj.update(dt);
		// render to display
		obj.render(renderer);
	}
}
