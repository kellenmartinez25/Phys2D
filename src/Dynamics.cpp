#include "Dynamics.h"
#include "Collision.h"
#include "MathUtils.h"
#include "DebugTools.h"
#include <limits>

float angMomentum(PhysObject& obj, glm::dvec2 r) {
	return Cross(r, (obj.mass) * obj.getPointVelocity(r));
}

void applyForce(PhysObject& obj, glm::dvec2 force, glm::dvec2 fPos, double dt) {
	double I = (1.0f / 12.0f) * obj.mass * (obj.size.x * obj.size.x + obj.size.y * obj.size.y);
	double torque = Cross(fPos, force);
	obj.angAccleration = torque * dt;
}

void collide(PhysObject& objA, PhysObject& objB, ContactPoint& cp, glm::dvec2 normal) {


	DebugDraw::get().drawPoint(cp.pos, glm::vec3(1.0f, 0.0f, 0.0f));

	// relative velocity between B and A
	glm::dvec2 v_BA = objB.velocity - objA.velocity;

	double e = 1.0;
	// check our normal velocity and other things, to make it not extreme when they are very small.
	double vRelNormal = glm::dot(v_BA, normal);
	if (glm::abs(vRelNormal) < 0.05)
		e = 0.05;
	if (glm::dot(v_BA, normal) > 0.0) {
		//return;
	}

	// j: impulse
	double j = (-(1 + e)) * vRelNormal;
	// vector from A/B to collision point P
	glm::dvec2 r_AP = cp.pos - objA.center();
	glm::dvec2 r_BP = cp.pos - objB.center();

	// save the dotprods of A and Norm
	double r_A_crossN = Cross(r_AP, normal);
	double r_B_crossN = Cross(r_BP, normal);

	// calculate the lower denominator of the impulse j
	j = j / ((objA.invMass + objB.invMass) +
		(r_A_crossN * r_A_crossN * objA.invInertia) +
		(r_B_crossN * r_B_crossN * objB.invInertia));
	glm::dvec2 J = j * normal;
	applyImpulse(objA, objB, { cp.pos, r_AP, r_BP }, J);
}

void collide2D(PhysObject& objA, PhysObject& objB, Manifold& manifold) {
	
	for (int i = 0; i < manifold.count; i++) {
		ContactPoint& cp = manifold.points[i];
		glm::dvec2 impulse = cp.normImpulse * manifold.normal;
		applyImpulse(objA, objB, cp, impulse);
		collide(objA, objB, cp, manifold.normal);
	}

}

void applyImpulse(PhysObject& objA, PhysObject& objB, ContactPoint point, const glm::dvec2 J) {
	// apply impulse to linear velocity
	objA.velocity = objA.velocity - J * objA.invMass;
	objB.velocity = objB.velocity + J * objB.invMass;
	// apply impulse to angular momentum
	objA.angularVelocity = objA.angularVelocity - objA.invInertia * Cross(point.rA, J);
	objB.angularVelocity = objB.angularVelocity + objB.invInertia * Cross(point.rB, J);
}