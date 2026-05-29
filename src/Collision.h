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
	int overlap(Projection& p2) const {
		return min < p2.max && p2.min < max;
	}
	double getOverlap(Projection& p2) const {
		return glm::min(max, p2.max) - glm::max(min, p2.min);
	}
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
