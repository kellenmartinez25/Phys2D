#include "Collision.h"
#include "MathUtils.h"
#include <limits>
#include <optional>

typedef std::vector<glm::dvec2> Vec2DCollection;
typedef glm::dvec2 Axis;



// Struct to generate, store, and compare SAT projections.
struct ProjectionResult {
	double min;
	double max;
	ProjectionResult(double min, double max) : min(min), max(max) {}
	ProjectionResult(Vec2DCollection& vertices, Axis& axis);
	double getOverlap(ProjectionResult& p2) const;
	bool isOverlapping(ProjectionResult& p2) const;

};

// Defines the vector for an Edge on a PhysObject.
// This should follow the standard CCW winding for the box:
//       <---
//	    -----
//	|  |     |  ^
//  v  |     |  |
//      -----
//       --->
// Meaning that the direction of the Edge (start -> end) should follow this pattern.
struct Edge {
	glm::dvec2 max;
	glm::dvec2 start;
	glm::dvec2 end;
};

// Stores the Minimum Translation Vector (MTV) result.
struct MTV {
	double minOverlap;	// The overlap on this smallest axis
	Axis smallest;	// The axis with the smallest overlap
};

std::optional<MTV> getMTV(PhysObject& obj1, PhysObject& obj2);
std::vector<Axis> getAxes(std::vector<glm::dvec2>& objVertices);
bool isColliding(PhysObject& obj1, PhysObject& obj2);
Edge bestEdge(const PhysObject& obj1, const Axis& sepNormal);

double ProjectionResult::getOverlap(ProjectionResult& p2) const {
	return glm::min(max, p2.max) - glm::max(min, p2.min);
}
bool ProjectionResult::isOverlapping(ProjectionResult& p2) const {
	const double EPSILON = 1e-5;
	return min <= p2.max + EPSILON && p2.min <= max + EPSILON;
}
// Apply a projection from a collection of vertices (from the corners of the box) onto an axis
ProjectionResult::ProjectionResult(Vec2DCollection& vertices, Axis& axis) {
	min = glm::dot(axis, vertices[0]);
	max = min;
	for (int i = 1; i < vertices.size(); i++) {
		double p = glm::dot(axis, vertices[i]);
		if (p < min)
			min = p;
		else if (p > max)
			max = p;
	}
	ProjectionResult proj { min, max };
}
// Get the Axes (normals) of each of the edges, from the vertices of an object.
// The associated axis would be the edge resulting from going forward in the winding pattern to the next vertex.
// So, using the index of the top-right vertex, the next vertex in the winding would be the top-left vertex,
// therefore this edge would be the top edge (pointing left), and its normal would be pointing out of the box (pointing up)
std::vector<Axis> getAxes(std::vector<glm::dvec2>& objVertices) {
	std::vector<Axis> axes(4);
	for (int i = 0; i < 4; i++) {
		glm::dvec2& p1 = objVertices[i];
		glm::dvec2& p2 = objVertices[i + 1 == 4 ? 0 : i + 1];
		glm::dvec2 edge = p2 - p1;
		glm::dvec2 perp = glm::dvec2(edge.y, -edge.x);
		glm::dvec2 norm = glm::normalize(perp);
		axes[i] = norm;
	}
	return axes;
}
// check whether two objects are colliding, using separating axis theorem
bool isColliding(PhysObject& obj1, PhysObject& obj2) {
	constexpr double overlap = std::numeric_limits<double>::max();
	std::vector<glm::dvec2> obj1Verts = obj1.getVertices();
	std::vector<glm::dvec2> obj2Verts = obj2.getVertices();
	std::vector<Axis> axes1 = getAxes(obj1Verts);
	std::vector<Axis> axes2 = getAxes(obj2Verts);
	for (int i = 0; i < 4; i++) {
		Axis axis = axes1[i];
		ProjectionResult p1(obj1Verts, axis);
		ProjectionResult p2(obj2Verts, axis);
		if (!p1.isOverlapping(p2))
			return false;
	}
	for (int i = 0; i < 4; i++) {
		Axis axis = axes2[i];
		ProjectionResult p1(obj1Verts, axis);
		ProjectionResult p2(obj2Verts, axis);
		if (!p1.isOverlapping(p2))
			return false;
	}
	return true;
}
// Use separating axis theorem to get the minimum translation vector
// MTV can be used to move obj1 the exact amount to stop from clipping, and is also used to find contact points
std::optional<MTV> getMTV(PhysObject& obj1, PhysObject& obj2) {
	// try to get the smallest overlapping projection
	double overlap = std::numeric_limits<double>::max();
	// the smallest axis found where projections overlap
	Axis smallest;
	// get the vertices of both objects
	std::vector<glm::dvec2> obj1Verts = obj1.getVertices();
	std::vector<glm::dvec2> obj2Verts = obj2.getVertices();
	// then, we obtain the axes (normals of the edges)
	std::vector<Axis> axes1 = getAxes(obj1Verts);
	std::vector<Axis> axes2 = getAxes(obj2Verts);
	// loop through object 1's axes
	for (int i = 0; i < 4; i++) {
		Axis axis = axes1[i];
		// obtain the projection from object 1 and 2 onto the current axis
		ProjectionResult p1(obj1Verts, axis);
		ProjectionResult p2(obj2Verts, axis);
		// if two projections dont overlap with each other, then we can say that they aren't colliding at all (via SAT)
		if (!p1.isOverlapping(p2))
			return std::nullopt;
		else {
			// if they are overlapping, then we get the actual overlap amount
			double o = p1.getOverlap(p2);
			if (o < overlap) {
				// if this is smaller than the current minimum, we update the smallest overlap and its associated axis
				overlap = o;
				smallest = axis;
			}
		}
	}
	// do the exact same loop as before, except instead use the axes from object 2.
	for (int i = 0; i < 4; i++) {
		Axis axis = axes2[i];
		ProjectionResult p1(obj1Verts, axis);
		ProjectionResult p2(obj2Verts, axis);
		if (!p1.isOverlapping(p2))
			return std::nullopt;
		else {
			double o = p1.getOverlap(p2);
			if (o < overlap) {
				overlap = o;
				smallest = axis;
			}
		}
	}
	// we now have the axis with the smallest amount of overlap between the object 1 and 2's projections.
	return MTV(overlap, smallest);
}

// get the edge of the object, who's normal most closest lines up to the separation normal.
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
	int prev = (index - 1 + 4) % 4;
	int next = (index + 1) % 4;

	glm::dvec2& vNext = obj1Verts[next];
	glm::dvec2& vPrev = obj1Verts[prev];

	glm::dvec2 edge1 = glm::normalize(v - vPrev);
	glm::dvec2 edge2 = glm::normalize(vNext - v);

	glm::dvec2 norm1 = glm::normalize(Cross(edge1, 1.0));
	glm::dvec2 norm2 = glm::normalize(Cross(edge2, 1.0));

	if (glm::dot(norm1, sepNormal) > glm::dot(norm2, sepNormal)) {
		return Edge{v, vPrev, v};
	}
	return Edge{v, v, vNext};
}

// Returns a line, which is the segment v1 -> v2 that is clipped if it is past the distance o along the normal n
Vec2DCollection clip(glm::dvec2 v1, glm::dvec2 v2, glm::dvec2 n, double o) {
	// The line we return
	Vec2DCollection cp;
	// The 
	double d1 = glm::dot(n, v1) - o;
	double d2 = glm::dot(n, v2) - o;

	if (d1 >= 0.0f) cp.push_back(v1);
	if (d2 >= 0.0f) cp.push_back(v2);

	if (d1 * d2 < 0.0f) {
		glm::dvec2 e = v2 - v1;
		double u = d1 / (d1 - d2);
		e *= u;
		e += v1;
		cp.push_back(e);

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

	Vec2DCollection cp = clip(inc.start, inc.end, refv, o1);
	// if the clipping didnt result in 1+ points, we have failed to generate our points
	if (cp.empty()) {
		out.count = 0;
		return false;
	}
	// next, clip the remaining inc edge by the 2nd vertex of the ref edge
	// since we're clipping in the opposite direction we flip the direction and offset
	double o2 = glm::dot(refv, ref.end);
	cp = clip(cp.at(0), cp.at(1), -refv, -o2);
	// also fail if not 1+ points left
	if (cp.empty()) {
		out.count = 0;
		return false;
	}

	// last clip: clip along the reference into the refNorm
	// get the normal of the reference edge
	glm::dvec2 refNorm = Cross(ref.end - ref.start, 1.0f);
	// remember to normalize it too
	refNorm = glm::normalize(refNorm);

	// get the largest depth our reference edge penetrates
	double max = glm::dot(refNorm, ref.max);
	bool erasedLast = false;


	// finally, we remove any points that are not past this maximum depth
	if (glm::dot(refNorm, cp.at(1)) - max > 0.0) {
		cp.pop_back();
		erasedLast = true;
	}
	if (glm::dot(refNorm, cp.at(0)) - max > 0.0) {
		cp.erase(cp.begin());
	}
	// build manifold
	out.normal = refNorm;
	for (auto v : cp) {
		// the point we're working on
		ContactPoint& pt = out.points[out.count];
		out.points[out.count].pos = v;
		pt.penetration = -glm::dot(refNorm, (v - ref.start));
		out.count++;
	}
	// if we flipped ref and inc then update the manifold's A and B respectively
	if (flip) {
		PhysObject* oldA = out.A;
		out.A = out.B;
		out.B = oldA;
	}

	return true;
}
// Populate a contact manifold with points between obj1 and obj2.
bool genManifold(PhysObject& obj1, PhysObject& obj2, Manifold& out) {
	// set the A and B objects
	out.A = &obj1;
	out.B = &obj2;

	auto mtvOptional = getMTV(obj1, obj2);
	if (!mtvOptional) {
		out.count = 0;
		return false;
	}
	MTV mtv = *mtvOptional;
	glm::dvec2 sepNorm = obj2.center() - obj1.center();
	double isPointingRight = glm::dot(sepNorm, mtv.smallest) / glm::length(sepNorm);
	if (glm::dot(obj1.center() - obj2.center(), mtv.smallest) > 0.0)
		mtv.smallest *= -1.0;
	
	Edge e1 = bestEdge(obj1, mtv.smallest);
	Edge e2 = bestEdge(obj2, -mtv.smallest);
	
	bool state = getClippedPoints(e1, e2, mtv.smallest, out);

	// check if the two generated points are really close to each other, and reduce the manifold if so.
	if (out.count == 2) {
		glm::dvec2 dv = (out.points[0].pos + out.points[1].pos) * 0.5;
		double d = glm::distance(out.points[0].pos, out.points[1].pos);
		// we dynamically change our tolerance depending on the object's size
		double tolerance = 0.01 * std::min(obj1.size.x, obj1.size.y);
		if (d < tolerance) {
			ContactPoint merged;
			merged.pos = dv;
			merged.penetration = std::max(out.points[0].penetration, out.points[1].penetration);
			merged.normImpulse = out.points[0].normImpulse + out.points[1].normImpulse;
			out.points[0] = merged;
			out.count = 1;
		}
	}

	return state;
}