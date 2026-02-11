#include "Engine.h"

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
// check if two boxes are colliding
int Engine::checkCollision(PhysObject& obj1, PhysObject& obj2) {
	// If any of the following conditions are FALSE, then there is no way that the two boxes should be colliding.
	if ((obj1.pos.x < (obj2.pos.x + obj2.size.x)) && // Is LEFT	  side of obj1 past RIGHT  side of obj2?
		((obj1.pos.x + obj1.size.x) > obj2.pos.x) && // Is RIGHT  side of obj1 past LEFT   side of obj2?
		(obj1.pos.y < (obj2.pos.y + obj2.size.y)) && // Is TOP    side of obj1 past BOTTOM side of obj2?
		((obj1.pos.y + obj1.size.y) > obj2.pos.y))   // Is BOTTOM side of obj1 past TOP    side of obj2?
		// If so, then they're colliding.
	{
		return true;
	}
	return false;
}
// direction -> normal vectors
glm::vec2 CARDINALS[] = {
		glm::vec2(0.0f, -1.0f),	// up
		glm::vec2(1.0f, 0.0f),	// right
		glm::vec2(0.0f, 1.0f),	// down
		glm::vec2(-1.0f, 0.0f)	// left
};
// vertices of a box
enum VERT {
	TL,
	TR,
	BR,
	BL
};
// get the vertex of obj1 that is currently inside obj2
VERT getCornerClippinginobj1(const PhysObject& obj1, const PhysObject& obj2) {
	if (obj2.pos.x > obj1.pos.x && obj2.pos.y > obj1.pos.y)
		return BR;
	if (obj1.pos.x + obj1.size.x > obj2.pos.x + obj2.size.x) {
		if (obj1.pos.y < obj2.pos.y)
			return BL;
		else return TL;
	}
	return TR;
}
// get the side of obj1 that is clipping into obj2 the *least*. used to prevent boxes jumping around when clipping
Engine::Direction Engine::getBestSideClipMatching(PhysObject& obj1, PhysObject& obj2) {
	VERT obj1Corner = getCornerClippinginobj1(obj1, obj2);
	switch (obj1Corner) {
	case TL:
		if (obj2.pos.x + obj2.size.x - obj1.pos.x < obj2.pos.y + obj2.size.y - obj1.pos.y)
			return LEFT;
		else return UP;
	case TR:
		if (obj1.pos.x + obj1.size.x - obj2.pos.x < obj2.pos.y + obj2.size.y - obj1.pos.y)
			return RIGHT;
		else return UP;
	case BR:
		if (obj1.pos.x + obj1.size.x - obj2.pos.x < obj1.pos.y + obj1.size.y - obj2.pos.y)
			return RIGHT;
		else return DOWN;
	case BL:
		if (obj2.pos.x + obj2.size.x - obj1.pos.x < obj1.pos.y + obj1.size.y - obj2.pos.y)
			return LEFT;
		else return DOWN;
	}
}
// collision resolution
void Engine::collide(PhysObject& obj1, PhysObject& obj2) {
	// the normal of the collision side we're working with
	Direction bestMatch = getBestSideClipMatching(obj1, obj2);
	// actual normal
	glm::vec2 n = CARDINALS[bestMatch];

	
	// The equations i used also took mass into consideration, but that is yet to be implemented so they're just gonna be 1.0 for now
	float m1 = 1.0f;
	float m2 = 1.0f;
	// used for easier readability
	glm::vec2& v1 = obj1.velocity;
	glm::vec2& v2 = obj2.velocity;
	// i found this equations on wikipedia i dont remember how they worked
	float E = 1.0f;
	float J = (-(1+E) * glm::dot(v1-v2, n)) /
		(1 / m1 + 1 / m2);
	// The calculated final velocities
	glm::vec2 v1_f = v1 + (J / m1) * n;
	glm::vec2 v2_f = v2 - (J / m2) * n;
	// obj1 and obj2 are going to be clipping into eachother, so fix that
	clipBoxes(obj1, obj2, bestMatch);
	// update velocities
	obj1.velocity = v1_f;
	obj2.velocity = v2_f;
}
// reset collision check flags
void Engine::resetCollisionCheckFlags() {
	for (PhysObject& obj : objects) {
		obj.collisionChecked2 = false;
	}
}
// get objects out of each other
void Engine::clipBoxes(PhysObject& obj1, PhysObject& obj2, Direction normDir) {
	// which direction is obj1 clipping into obj2 in?
	// EPSILON is a small float which is added to the calculated offset, so that they're a small distance apart (avoids precision errors)
	switch (normDir) {
	case RIGHT:
		obj1.pos.x = (obj2.pos.x - obj1.size.x) - EPSILON;
		break;
	case LEFT:
		obj1.pos.x = obj2.pos.x + obj2.size.x + EPSILON;
		break;
	case UP:
		obj1.pos.y = obj2.pos.y + obj2.size.y + EPSILON;
		break;
	case DOWN:
		obj1.pos.y = obj2.pos.y - obj1.size.y - EPSILON;
	}

}
// update positions and velocities
void Engine::updateSim(float dt)
{
	// Iterate through each object
	for (int i = 0; i < objects.size(); i++) {
		PhysObject& obj = objects[i]; // the one we're working with
		// Iterate against everything else
		for (int j = 0; j < objects.size(); j++) {
			if (j == i) continue;	// dont check the same one
			PhysObject& obj2 = objects[j];
			// if they're colliding and if it hasn't already been checked
			if (checkCollision(obj, obj2) && !obj2.collisionChecked2) {
				collide(obj, obj2);
				obj.collisionChecked2 = true;
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
	// we're done checking collisions so flags are reset
	resetCollisionCheckFlags();
}
