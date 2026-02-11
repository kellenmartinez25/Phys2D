#include "PhysObject.h"

PhysObject::PhysObject()
{
}
// create new physobject
PhysObject::PhysObject(glm::vec2 pos, glm::vec2 size, glm::vec2 velocity, glm::vec3 color) :
	pos(pos),
	size(size),
	velocity(velocity),
	rotation(0.0f),	// rotation isnt used yet so 0 degs by default
	color(color)
{
}
// used if we want to init the object but provide the details later
void PhysObject::init(glm::vec2 pos, glm::vec2 size, glm::vec2 velocity, glm::vec3 color) {
	this->pos = pos;
	this->size = size;
	this->velocity = velocity;
	this->rotation = 0.0f;
}
// update position via small timestep, using eulers method (i think)
void PhysObject::update(float dt) {
	pos += dt * velocity;
}
// use a provided renderer object to draw the PhysObject
void PhysObject::render(Renderer* renderer) const {
	renderer->drawSprite(
		pos,
		size,
		rotation,
		color
	);
}
// get the coords of the center of the object
glm::vec2 PhysObject::center()
{
	return glm::vec2(pos.x + 0.5f*size.x, pos.y + 0.5f * size.y);
}
