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

void collide2D(PhysObject& objA, PhysObject& objB, Manifold& manifold) {
	// for collision manifolds of side 2 we just resolve it to one point by getting the midpoint
	glm::dvec2 collisionPoint;
	if (manifold.count == 2)
		collisionPoint = (manifold.get(0) + manifold.get(1)) / 2.0;
	else if (manifold.count == 0)
		return;
	else
		collisionPoint = manifold.get(0);


	DebugDraw::get().drawPoint(collisionPoint, glm::vec3(1.0f, 0.0f, 0.0f));

	// relative velocity between B and A
	glm::dvec2 v_BA = objB.velocity - objA.velocity;
	
	double e = 1.0;
	// check our normal velocity and other things, to make it not extreme when they are very small.
	double vRelNormal = glm::dot(v_BA, manifold.normal);
	if (glm::abs(vRelNormal) < 0.05 || objA.angularVelocity < 0.05)
		e = 0.05;
	if (glm::dot(v_BA, manifold.normal) > 0.0) {
		return;
	}
	
	// we can change this later

	
	// did you know? 1 / inf = 0
	// we instead calculate the inverse inertia and mass, for dealing with static objects
	// static objects will have an infinite mass, meaning they will have an inverse mass/inertia of exactly 0.0
	// I^-1 = 12 / (m * (w^2 + h^2))
	double invIntertiaA = (objA.isStatic) ? 0.0 : (12.0) / (objA.mass * (objA.size.x * objA.size.x + objA.size.y * objA.size.y));
	double invIntertiaB = (objB.isStatic) ? 0.0 : (12.0) / (objB.mass * (objB.size.x * objB.size.x + objB.size.y * objB.size.y));

	// same here
	double invMassA = (objA.isStatic) ? 0.0 : 1 / objA.mass;
	double invMassB = (objB.isStatic) ? 0.0 : 1 / objB.mass;

	// j: impulse
	double j = (-(1 + e)) * glm::dot(v_BA, manifold.normal);
	// vector from A/B to collision point P
	glm::dvec2 r_AP = collisionPoint - objA.center();
	glm::dvec2 r_BP = collisionPoint - objB.center();

	// save the dotprods of A and Norm
	double r_A_crossN = Cross(r_AP, manifold.normal);
	double r_B_crossN = Cross(r_BP, manifold.normal);

	// calculate the lower denominator of the impulse j
	j = j / ((invMassA + invMassB) +
		(r_A_crossN * r_A_crossN * invIntertiaA) +
		(r_B_crossN * r_B_crossN * invIntertiaB));
	// captital J is the impulse vector, along the normal
	glm::dvec2 J = j * manifold.normal;
	// apply impulse to linear velocity
	objA.velocity = objA.velocity - J * invMassA;
	objB.velocity = objB.velocity + J * invMassB;
	// apply impulse to angular momentum
	objA.angularVelocity = objA.angularVelocity - invIntertiaA * Cross(r_AP, J);
	objB.angularVelocity = objB.angularVelocity + invIntertiaB * Cross(r_BP, J);

}