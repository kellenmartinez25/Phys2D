#pragma once

#include "PhysObject.h"
#include "PhysTypes.h"
#include <glm/glm.hpp>
#include <vector>
typedef glm::dvec2 Axis;
typedef std::vector<glm::dvec2> Vec2DCollection;

struct Projection {
	double min;
	double max;
	int overlap(Projection& p2) const;
	double getOverlap(Projection& p2) const;
	Projection(double min, double max) : min(min), max(max) {}
	Projection(Vec2DCollection vertices, Axis axis);
};
struct Edge {
	glm::dvec2 max;
	glm::dvec2 start;
	glm::dvec2 end;
}; 
struct MTV {
	double overlap;
	Axis smallest;
};
MTV* getMTV(PhysObject& obj1, PhysObject& obj2);
std::vector<Axis> getAxes(std::vector<glm::dvec2>& objVertices);
bool isColliding(PhysObject& obj1, PhysObject& obj2);
bool genManifold(PhysObject& obj1, PhysObject& obj2, Manifold& out);
Edge bestEdge(const PhysObject& obj1, const Axis& sepNormal);
