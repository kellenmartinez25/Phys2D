#include "Dynamics.h"
#include "Collision.h"
#include "MathUtils.h"
#include "DebugTools.h"
#include <limits>

inline glm::dvec2 getPointVelocity(PhysObject& obj, glm::dvec2 r) {
	return obj.velocity + Cross(obj.angularVelocity, r);
}

double vBias(double penetration) {
	const double slop = 0.01;
	const double percent = 0.2;
	return std::max(penetration - slop, 0.0) * percent;
}

void resolveContactPoint(Manifold& m, ContactPoint& cp, double dt) {

	// coeff of restitution
	double e = 1.0;

	auto vA = getPointVelocity(*m.A, cp.rA);
	auto vB = getPointVelocity(*m.B, cp.rB);
	// relative velocity, from A (ref) to B (inc)
	auto vRel = vB - vA;
	// relative velocity along the collision normal
	//auto vRelNorm = glm::dot(vRel, m.normal);

	double vRelNorm = glm::dot(m.B->velocity - m.A->velocity, m.normal)
		+ glm::dot(Cross(m.B->angularVelocity, cp.rB), m.normal)
		- glm::dot(Cross(m.A->angularVelocity, cp.rA), m.normal);

	// sleepy
	if (fabs(vRelNorm) < 0.02 && cp.penetration < 0.02)
		vRelNorm = 0.0;
	if (fabs(vRelNorm) < 0.05)
		e = 0.0;

	// if the vRel along the normal is > 0, then the two objects are moving apart (at this contact point)
	// meaning that we shouldnt resolve anything in this case.
	if (vRelNorm > 0.0)
		return;
	auto rA_crossN = Cross(cp.rA, m.normal);
	auto rB_crossN = Cross(cp.rB, m.normal);
	/*double k = m.A->invMass + m.B->invMass +
		(rA_crossN * rA_crossN * m.A->invInertia) +
		(rB_crossN * rB_crossN * m.B->invInertia);*/

	double k = cp.effectiveMass > 0.0 ? (1.0 / cp.effectiveMass) : 0.0;
	const double beta = 0.8, slop = 0.01;
	double bias = beta * std::max(cp.penetration - slop, 0.0) / dt;

	double j = (bias - vRelNorm) / k;
	// accurate impulse solver by getting the *change* in old->new impulse from our manifold
	double oldImpulse = cp.normImpulse;
	// get the accumulated normal impulse
	double newImpulse = glm::max(oldImpulse + j, 0.0);
	// get the accumulated impulse
	cp.normImpulse = newImpulse;

	double jFinal = newImpulse - oldImpulse;
	glm::dvec2 J = jFinal * m.normal;
	applyImpulse(*m.A, *m.B, cp, J);
	double vnAfter = glm::dot(
		(m.B->velocity + Cross(m.B->angularVelocity, cp.rB)) -
		(m.A->velocity + Cross(m.A->angularVelocity, cp.rA)),
		m.normal);
	double vansdf = vnAfter + 1.0;
}

// try to resolve a contact surface's impulses using rigid body constrait solving
void resolveContactSurfaceManifold(Manifold& m) {
	PhysObject& A = *m.A;
	PhysObject& B = *m.B;
	glm::dvec2& n = m.normal;
	if (m.count != 2) return;

	// K = J * M^-1 * J^T
	// 
	// Kl = -b
	// K = 2x2 effective mass (already calculated in pre-step)
	// b = velocity error vector (bias)
	// each element K_ij = effective mass between contact i and j
	
	// im just going to define each vec/mat element manually
	auto& c0 = m.points[0];
	auto& c1 = m.points[1];
	
	glm::dvec2& rA0 = c0.rA;
	glm::dvec2& rB0 = c0.rB;
	glm::dvec2& rA1 = c1.rA;
	glm::dvec2& rB1 = c1.rB;

	auto v0 = getPointVelocity(B, rB0) - getPointVelocity(A, rA0);
	auto v1 = getPointVelocity(B, rB1) - getPointVelocity(A, rA1);

	double b0 = glm::dot(v0, n);
	double b1 = glm::dot(v1, n);

	// construct the matrix
	double K00, K11, K01, K10;

	// K = m_A^-1 + m_B^-1 + (rA x n)^2 * I_A^-1 + (rB x n)^2 * I_B^-1

	K00 = A.invMass + B.invMass +
		Cross(rA0, n) * Cross(rA0, n) * A.invInertia +
		Cross(rB0, n) * Cross(rB0, n) * B.invInertia;
	K11 = A.invMass + B.invMass +
		Cross(rA1, n) * Cross(rA1, n) * A.invInertia +
		Cross(rB1, n) * Cross(rB1, n) * B.invInertia;
	K01 = A.invMass + B.invMass +
		Cross(rA0, n) * Cross(rA1, n) * A.invInertia +
		Cross(rB0, n) * Cross(rB1, n) * B.invInertia;
	K10 = K01;

	// get the det
	double det = K00 * K11 - K01 * K10;
	if (fabs(det) < 1e-8) return;
	
	double invDet = 1.0 / det;

	// calculate the j vector
	double j0, j1;
	
	j0 = invDet * (K11 * (-b0) - K01 * (-b1));
	j1 = invDet * (-K10 * (-b0) + K00 * (-b1));

	// apply the solved impulses
	applyImpulse(A, B, c0, j0 * n);
	applyImpulse(A, B, c1, j1 * n);


}

// Resolve two clipping bodies in a Manifold using Baumgarte stabilization
void correctPos(Manifold& manifold) {
	const double slop = 0.01;
	const double percent = 0.80;
	
	// find the contact point with maximum penetration; this is the one we will be correcting from
	double maxPenetration = 0.0;
	for (int i = 0; i < manifold.count; i++) {
		maxPenetration = std::max(maxPenetration, manifold.points[i].penetration);
	}
	double correctMag = std::max(maxPenetration - slop, 0.0) * percent;
	glm::dvec2 correction = correctMag * manifold.normal;
	double totalInvMass = manifold.A->invMass + manifold.B->invMass;

	// total m^-1 might be 0.0 if both objects are static
	if (totalInvMass == 0.0)
		return;
	// apply to both objects
	manifold.A->pos -= correction * (manifold.A->invMass / totalInvMass);
	manifold.B->pos += correction * (manifold.B->invMass / totalInvMass);
}

 void collide2D(Manifold& manifold, double dt) {
	// get the average point of collision
	if (manifold.count == 2)
		resolveContactSurfaceManifold(manifold);
	else
		resolveContactPoint(manifold, manifold.points[0], dt);
}

void applyImpulse(PhysObject& A, PhysObject& B, ContactPoint& point,  glm::dvec2 J) {
	// apply impulse to linear velocity
	A.velocity -= J * A.invMass;
	B.velocity += J * B.invMass;
	// apply impulse to angular momentum
	A.angularVelocity -= A.invInertia * Cross(point.rA, J);
	B.angularVelocity += B.invInertia * Cross(point.rB, J);
}

void preStep(Manifold& manifold) {
	for (int i = 0; i < manifold.count; i++) {
		auto& cp = manifold.points[i];

		// recalc rA and rB
		cp.rA = cp.pos - manifold.A->center();
		cp.rB = cp.pos - manifold.B->center();


		// save the dotprods of A and Norm
		double r_A_crossN = Cross(cp.rA, manifold.normal);
		double r_B_crossN = Cross(cp.rB, manifold.normal);

		// calculate the effective mass
		double K = ((manifold.A->invMass + manifold.B->invMass) +
			(r_A_crossN * r_A_crossN * manifold.A->invInertia) +
			(r_B_crossN * r_B_crossN * manifold.B->invInertia));
		// effective mass is essentially the denominator of j
		// also check for any div by 0 cases
		cp.effectiveMass = (K > 0.0) ? (1.0 / K) : 0.0;

		glm::dvec2 J = cp.normImpulse * manifold.normal;

		applyImpulse(*manifold.A, *manifold.B, cp, J);
	}
}
