#include "Engine.h"

typedef glm::dvec2 vec2;
using glm::radians;
inline double mass(glm::dvec2 size) {
	return size.x * size.y;
}
void floorBorder(Engine* engine) {
	engine->addObject(new PhysObject(
		glm::dvec2(0.0, 0.0),
		glm::dvec2(16.0, 1.0),
		0.0,
		true)
	);
}

// Default scenario, blank
void scenario0(Engine* engine) {
	floorBorder(engine);
}
// spinny fan
void scenario1(Engine* engine) {
	floorBorder(engine);
	PhysObject* fan = new PhysObject();
	fan->pos = { 2.0, 3.0 };
	fan->size = { 4.0, 0.2 };
	fan->angularVelocity = glm::radians(30.0);
	fan->isStatic = true;
	fan->calcInverses();
	engine->addObject(fan);
}

// some stacking
void scenario2(Engine* engine) {
	floorBorder(engine);
	engine->addObject(new PhysObject(
		vec2{ 2.0, 1.0 },
		vec2{ 1.0 },
		0.0,
		false
	));
	engine->addObject(new PhysObject(
		vec2{ 1.8, 2.5 },
		vec2{ 1.0 },
		radians(-89.0),
		vec2{0.2,0.0}
	));
	engine->addObject(new PhysObject(
		vec2{ 2.25, 3.9 },
		vec2{ 1.0 },
		radians(15.0),
		vec2{ -0.3,0.0 }
	));
	engine->addObject(new PhysObject(
		vec2{ 4.0, 1.0 },
		vec2{ 1.0 },
		0.0,
		false
	));
}
// balance
void scenario3(Engine* engine) {
	floorBorder(engine);
	PhysObject* left = new PhysObject(
		vec2{ 2.0, 1.0 },
		vec2{ 1.0 },
		0.0,
		true
	);
	PhysObject* right = new PhysObject(
		vec2{ 4.0, 1.0 },
		vec2{ 1.0 },
		0.0,
		true
	);
	PhysObject* bevel = new PhysObject(
		vec2{ 3.0, 2.5 },
		vec2{ 1.0 },
		radians(47.0),
		false
	);

	engine->addObject(left);
	engine->addObject(right);
	engine->addObject(bevel);
}