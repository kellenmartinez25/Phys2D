#include "PhysObject.h"
#include <limits>

constexpr double ALPHA_DECAY = 0.01;

PhysObject::PhysObject() :
	pos(0.0f),
	size(1.0f),
	angle(0.0f),
	velocity(0.0f),
	color(1.0f),
	force(0.0f),
	mass(1.0f)
{
}
// create new physobject
PhysObject::PhysObject(glm::dvec2 pos, glm::dvec2 size, glm::dvec2 velocity, glm::vec3 color) :
	pos(pos),
	size(size),
	velocity(velocity),
	angle(0.0f),
	color(color),
	mass(1.0f),
	force(0.0f)

{
}

PhysObject::PhysObject(glm::dvec2 pos, glm::dvec2 size, double mass, double rotation, glm::dvec2 velocity) :
	pos(pos),
	size(size),
	velocity(velocity),
	angle(rotation),
	color(1.0f),
	mass(mass),
	force(0.0f)
{
}

PhysObject::PhysObject(glm::dvec2 pos, glm::dvec2 size, double rotation, bool isStatic) :
	PhysObject(pos, size, std::numeric_limits<double>::max(), rotation)
{
	this->isStatic = isStatic;
}

// used if we want to init the object but provide the details later
void PhysObject::init(glm::dvec2 pos, glm::dvec2 size, glm::dvec2 velocity, glm::vec3 color) {
	this->pos = pos;
	this->size = size;
	this->velocity = velocity;
	this->angle = 0.0f;
}
// update position via small timestep, using eulers method (i think)
void PhysObject::update(double dt) {
	if (!isStatic) {
		// get force vector
		force = glm::dvec2(0.0, -100.0);
		// update velocity from force:
		// F = m * a -> a = F/m
		// v1 = v0 + a * dt -> v1 = v0 + (F/m) * dt
		velocity += (dt * (1.0 / mass)) * force;
		// update our position from velocity & timestep
		pos += dt * velocity;
		// Update angular velocity too, from ang accel
		angularVelocity += dt * angAccleration;
		// angle position is updated from w
		angle += dt * angularVelocity;

		// decay
		angularVelocity *= 0.995;
		
	}
}
// use a provided renderer object to draw the PhysObject
void PhysObject::render(Renderer* renderer) const {
	renderer->drawSprite(
		pos,
		size,
		angle,
		color
	);
}
// Get the positions of each vertex on the box
std::vector<glm::dvec2> PhysObject::getVertices() const {
	// build the transformation matrix from our pos/angle/size
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(size.x * 0.5f, size.y * 0.5f, 0.0f));
	model = glm::translate(model, glm::vec3(pos, 0.0f));
	model = glm::rotate(model, glm::radians(static_cast<float>(angle)), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	

	/*model = glm::translate(model, glm::vec3(pos, 0.0f));
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, -glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));*/

	// the base vertex matrix that we will put in
	std::vector<glm::dvec2> vertices = {
		glm::dvec2(-0.5f, -0.5f),	// BL
		glm::dvec2(0.5f, -0.5f),		// BR
		glm::dvec2(0.5f, 0.5f),		// TR
		glm::dvec2(-0.5f, 0.5f)		// TL
	};
	// fill with the transformed vectors
	for (int i = 0; i < 4; i++) {
		vertices[i] = glm::dvec2(model * glm::vec4(vertices[i], 0.0f, 1.0f));
	}
	// return the collection of vectors
	return vertices;
}
// get the velocity of a certain point on the PhysObject
// r = position relative to the center of the object
glm::dvec2 PhysObject::getPointVelocity(glm::dvec2 r) {	
	return glm::radians(angularVelocity) * glm::dvec2(-r.y, r.x) + velocity;
}

