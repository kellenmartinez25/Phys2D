#include "PhysTypes.h"

bool Manifold::add(glm::dvec2 v) {
	switch (count) {
	case 0:
		points[0].pos = v;
		count = 1;
		return true;
	case 1:
		points[1].pos = v;
		count = 2;
		return true;
	case 2:
		return false;
	}
}
glm::dvec2 Manifold::get(int index)
{
	return points[index].pos;
}
