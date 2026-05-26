#pragma once
#include <glm/glm.hpp>

inline double Cross(glm::dvec2 v1, glm::dvec2 v2) {
	return v1.x * v2.y - v1.y * v2.x;
}
inline glm::dvec2 Cross(glm::dvec2 v, double z) {
	return glm::dvec2(v.y * z, -v.x * z);
}
inline glm::dvec2 Perp(glm::dvec2 vec) {
	return glm::dvec2(-vec.y, vec.x);
}
inline bool doubleEquals(double a, double b, double E = 0.00001f) {
	double r = a - b;
	r = (r < 0.0f) ? -r : r;
	return r < E;
}