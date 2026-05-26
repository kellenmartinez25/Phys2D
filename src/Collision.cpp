#include "Collision.h"
#include "MathUtils.h"
#include <limits>

typedef std::vector<glm::dvec2> Vec2DCollection;


// Apply a projection from a collection of vertices onto an axis
Projection::Projection(Vec2DCollection vertices, Axis axis) {
	min = glm::dot(axis, vertices[0]);
	max = min;
	for (int i = 1; i < vertices.size(); i++) {
		double p = glm::dot(axis, vertices[i]);
		if (p < min)
			min = p;
		else if (p > max)
			max = p;
	}
	Projection proj { min, max };
}
// get the axes of each edge of the object
std::vector<Axis> getAxes(std::vector<glm::dvec2>& objVertices) {
	std::vector<Axis> axes(4);
	for (int i = 0; i < 4; i++) {
		glm::dvec2& p1 = objVertices[i];
		glm::dvec2& p2 = objVertices[i + 1 == 4 ? 0 : i + 1];
		glm::dvec2 edge = p1 - p2;
		glm::dvec2 perp = glm::dvec2(-edge.y, edge.x);
		glm::dvec2 norm = glm::normalize(perp);
		axes[i] = norm;
	}
	return axes;
}
// check whether two objects are colliding, using separating axis theorem
bool isColliding(PhysObject& obj1, PhysObject& obj2) {
	constexpr double overlap = std::numeric_limits<double>::max();
	Axis smallest;
	std::vector<glm::dvec2> obj1Verts = obj1.getVertices();
	std::vector<glm::dvec2> obj2Verts = obj2.getVertices();
	std::vector<Axis> axes1 = getAxes(obj1Verts);
	std::vector<Axis> axes2 = getAxes(obj2Verts);
	for (int i = 0; i < 4; i++) {
		Axis axis = axes1[i];
		Projection p1(obj1Verts, axis);
		Projection p2(obj2Verts, axis);
		if (!p1.overlap(p2))
			return false;
	}
	for (int i = 0; i < 4; i++) {
		Axis axis = axes2[i];
		Projection p1(obj1Verts, axis);
		Projection p2(obj2Verts, axis);
		if (!p1.overlap(p2))
			return false;
	}
	return true;
}
// Use separating axis theorem to get the minimum translation vector
// MTV can be used to move obj1 the exact amount to stop from clipping, and also used to find contact points
MTV* getMTV(PhysObject& obj1, PhysObject& obj2) {
	double overlap = std::numeric_limits<double>::max();
	Axis smallest;
	std::vector<glm::dvec2> obj1Verts = obj1.getVertices();
	std::vector<glm::dvec2> obj2Verts = obj2.getVertices();
	std::vector<Axis> axes1 = getAxes(obj1Verts);
	std::vector<Axis> axes2 = getAxes(obj2Verts);
	for (int i = 0; i < 4; i++) {
		Axis axis = axes1[i];
		Projection p1(obj1Verts, axis);
		Projection p2(obj2Verts, axis);
		if (!p1.overlap(p2))
			return NULL;
		else {
			double o = p1.getOverlap(p2);
			if (o < overlap) {
				overlap = o;
				smallest = axis;
			}
		}
	}
	for (int i = 0; i < 4; i++) {
		Axis axis = axes2[i];
		Projection p1(obj1Verts, axis);
		Projection p2(obj2Verts, axis);
		if (!p1.overlap(p2))
			return NULL;
		else {
			double o = p1.getOverlap(p2);
			if (o < overlap) {
				overlap = o;
				smallest = axis;
			}
		}
	}
	return new MTV(overlap, smallest);
}

// get the edge of the object, who's normal most closest lines up to the separation normal
Edge bestEdge(const PhysObject& obj1, const Axis& sepNormal) {
	std::vector<glm::dvec2> obj1Verts = obj1.getVertices();

	int c = obj1Verts.size();
	int index = 0;
	double max = glm::dot(sepNormal, obj1Verts[0]);
	for (int i = 0; i < c; i++) {
		double proj = glm::dot(obj1Verts[i], sepNormal);
		if (proj > max) {
			max = proj;
			index = i;
		}
	}
	glm::dvec2& v = obj1Verts[index];
	glm::dvec2& v1 = obj1Verts[index + 1 == 4 ? 0 : index + 1];
	glm::dvec2& v0 = obj1Verts[index - 1 == -1 ? 3 : index - 1];

	glm::dvec2 l = v - v1;
	glm::dvec2 r = v - v0;

	l = glm::normalize(l);
	r = glm::normalize(r);
	
	if (glm::dot(r, sepNormal) <= glm::dot(l, sepNormal)) {
		return Edge(v, v0, v);
	}
	else
		return Edge(v, v, v1);
}

// clip 2 points
Vec2DCollection* clip(glm::dvec2 v1, glm::dvec2 v2, glm::dvec2 n, double o) {
	Vec2DCollection* cp = new Vec2DCollection();
	double d1 = glm::dot(n, v1) - o;
	double d2 = glm::dot(n, v2) - o;

	if (d1 >= 0.0f) cp->push_back(v1);
	if (d2 >= 0.0f) cp->push_back(v2);

	if (d1 * d2 < 0.0f) {
		glm::dvec2 e = v2 - v1;
		double u = d1 / (d1 - d2);
		e *= u;
		e += v1;
		cp->push_back(e);

	}
	return cp;
}



// Generate a contact manifold between two edges and an MTV.
// May contain 0-2 points.
bool getClippedPoints(Edge& e1, Edge& e2, glm::dvec2 sepNorm, Manifold& out) {
	// helper vectors for easier readibility
	glm::dvec2 e10 = e1.end - e1.start;
	glm::dvec2 e20 = e2.end - e2.start;
	glm::dvec2 n = sepNorm;
	
	bool flip = false;
	Edge ref = e1;
	Edge inc = e2;
	glm::dvec2 refv = e10;
	if (glm::abs(glm::dot(e10, n)) > glm::abs(glm::dot(e20, n))) {
		ref = e2;
		inc = e1;
		refv = e20;
		flip = true;
	}

	// normalize the reference vector
	refv = glm::normalize(refv);

	double o1 = glm::dot(refv, ref.start);
	// clip the inc edge by the first vertex of the ref edge

	Vec2DCollection* cp = clip(inc.start, inc.end, refv, o1);
	// if the clipping didnt result in 2 points, we have failed to generate our points
	if (cp->size() < 2) {
		out.count = 0;
		return false;
	}
	// next, clip the remaining inc edge by the 2nd vertex of the ref edge
	// since we're clipping in the opposite direction we flip the direction and offset
	double o2 = glm::dot(refv, ref.end);
	cp = clip(cp->at(0), cp->at(1), -refv, -o2);
	// also fail if not 2 points left
	if (cp->size() < 2) {
		out.count = 0;
		return false;
	}

	// last clip: clip along the reference into the refNorm
	// get the normal of the reference edge
	glm::dvec2 refNorm = Cross(ref.end - ref.start, -1.0f);
	// remember to normalize it too
	refNorm = glm::normalize(refNorm);

	// get the largest depth our reference edge penetrates
	double max = glm::dot(refNorm, ref.max);
	bool erasedLast = false;


	// finally, we remove any points that are not past this maximum depth
	if (glm::dot(refNorm, cp->at(1)) - max < 0.0) {
		cp->pop_back();
		erasedLast = true;
	}
	if (glm::dot(refNorm, cp->at(0)) - max < 0.0) {
		cp->erase(cp->begin());
	}
	// build manifold
	out.normal = refNorm;
	for (auto v : *cp) {
		// the point we're working on
		ContactPoint& pt = out.points[out.count];
		out.points[out.count].pos = v;
		pt.penetration =  glm::dot(refNorm, v) - glm::dot(refNorm, ref.start);
		out.count++;
	}


	return true;
}
// Populate a contact manifold with points between obj1 and obj2.
bool genManifold(PhysObject& obj1, PhysObject& obj2, Manifold& out) {
	MTV* mtv = getMTV(obj1, obj2);
	glm::dvec2 sepNorm = obj2.center() - obj1.center();
	double isPointingRight = glm::dot(sepNorm, mtv->smallest) / glm::length(sepNorm);
	if (glm::dot(obj1.center() - obj2.center(), mtv->smallest) < 0.0)
		mtv->smallest *= -1.0;
	
	Edge e1 = bestEdge(obj1, -mtv->smallest);
	Edge e2 = bestEdge(obj2, mtv->smallest);
	
	return getClippedPoints(e1, e2, mtv->smallest, out);
}