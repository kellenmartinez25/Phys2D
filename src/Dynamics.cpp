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
	// stabilize the resulting impulse
	const double beta = 0.05, slop = 0.02, maxBias = 0.2;
	double bias = beta * std::max(cp.penetration - slop, 0.0) / dt;
	bias = std::min(bias, maxBias);

	// if the vRel along the normal is > 0, then the two objects are moving apart (at this contact point)
	// meaning that we shouldnt resolve anything in this case.
	if (vRelNorm > 0.0 && cp.penetration <= slop)
		return;

	double k = cp.effectiveMass > 0.0 ? (1.0 / cp.effectiveMass) : 0.0;
	// coeff of restitution
	double e = 0.0;
	//if (vRelNorm < -1.0)
		//e = 0.5;
	double bouncy = -e * vRelNorm;
	double v_target = bias + bouncy;
	double j = (v_target - vRelNorm) * cp.effectiveMass;
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
void resolveContactSurfaceManifold(Manifold& m, double dt) {
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
	// velocity vector
	auto v0 = getPointVelocity(B, rB0) - getPointVelocity(A, rA0);
	auto v1 = getPointVelocity(B, rB1) - getPointVelocity(A, rA1);

	// bias, for stabilization
	const double beta = 0.08, slop = 0.02, maxBias = 0.5;
	double bias0 = beta * std::max(c0.penetration - slop, 0.0) / dt;
	double bias1 = beta * std::max(c1.penetration - slop, 0.0) / dt;
	// limit
	bias0 = std::min(bias0, maxBias);
	bias1 = std::min(bias1, maxBias);

	// normalized
	double vn0 = glm::dot(v0, n);
	double vn1 = glm::dot(v1, n);
	// target norm velocity vector
	double b0 = bias0 - vn0;
	double b1 = bias1 - vn1;
	// pre compute cross products
	double rA0xn = Cross(rA0, n);
	double rB0xn = Cross(rB0, n);
	double rA1xn = Cross(rA1, n);
	double rB1xn = Cross(rB1, n);

	// construct the matrix
	double K00, K11, K01, K10;

	// K = m_A^-1 + m_B^-1 + (rA x n)^2 * I_A^-1 + (rB x n)^2 * I_B^-1

	K00 = A.invMass + B.invMass +
		rA0xn * rA0xn * A.invInertia +
		rB0xn * rB0xn * B.invInertia;
	K11 = A.invMass + B.invMass +
		rA1xn * rA1xn * A.invInertia +
		rB1xn * rB1xn * B.invInertia;
	K01 = A.invMass + B.invMass +
		rA0xn * rA1xn * A.invInertia +
		rB0xn * rB1xn * B.invInertia;
	K10 = K01;

	// aquire the impulses from the previous frame/iteration
	// we actually dont really need to solve for the entire impulse from scratch every calculation
	// you should instead solve for a change in J and then only apply the delta, the change in I to the objects
	// this is the same thing we do in the 1D manifold solver (except its way more complicated now, thank you supreme court eunuch ChatGPT)
	double old0 = c0.normImpulse;
	double old1 = c1.normImpulse;
	double new0 = old0;
	double new1 = old1;
	bool solved = false;
	{
		// try to solve K * deltaImpulse = b
		// (deltaImpulse = [d0, d1]
		// so now we can get the impulse changes to correct both of our contact velocities

		// determinate
		double det = K00 * K11 - K01 * K10;
		// is it really small? if not then it should be worth while to use
		if (fabs(det) > 1e-9) {
			double invDet = 1.0 / det;

			// achieve the inverse of the 2x2 matrix:
			// K^-1 = 1/det * [  K11 -K01 ]
			//				  [ -K10  K00 ]
			// so then:
			// deltaI = inv(K) * b
			double d0 = invDet * (K11 * b0 - K01 * b1);
			double d1 = invDet * (-K10 * b0 + K00 * b1);

			double canidate0 = old0 + d0;
			double canidate1 = old1 + d1;
			// these contact impulses that may or may not be used,
			// should be pushing the objects apart, but they shouldnt pull them together!
			// we need to make sure that each of these accumulated impulses stay above >= 0.0
			// if our canidate impulses check out, then this is the valid solution and we get off easy
			if (canidate0 >= 0.0 && canidate1 >= 0.0) {
				new0 = canidate0;
				new1 = canidate1;
				solved = true;
			}
		}
	}
	if (!solved) {
		// we come here when the full 2D manifold solution was invalid,
		// due to at least 1 of the impulses were negative.
		// we have a delightful remedy for this by disabling a contact and solving the other one on its own
		double d0 = 0.0;
		if (K00 > 0.0) {
			d0 = b0 / K00;
		}
		new0 = std::max(old0 + d0, 0.0);
		new1 = 0.0;
		solved = true;
	}

	if (new1 == 0.0) {
		// if contact 0 ends up inactive, then we will try to solve contact 1.
		// this will handle cases where only the 2nd ContactPoint can actually support the collision.
		double d1 = 0.0;
		if (K11 > 0.0) {
			d1 = b1 / K11;
		}
		new0 = 0.0;
		new1 = std::max(old1 + d1, 0.0);
	}

	// now, with things in expected conditions, we can compute the changes in impulse.
	// because of the multiple iterations, we apply only dI so that we dont keep on doing the same impulse application on every iteration
	double delta_I0 = new0 - old0;
	double delta_I1 = new1 - old1;
	// update our accumulated impulses, for usage by the next iteration and prestep
	c0.normImpulse = new0;
	c0.normImpulse = new1;
	
	// we can calculate our J vectors
	glm::dvec2 J0 = delta_I0 * n;
	glm::dvec2 J1 = delta_I1 * n;

	// apply the solved impulses
	applyImpulse(A, B, c0, J0);
	applyImpulse(A, B, c1, J1);


}

// Resolve two clipping bodies in a Manifold using Baumgarte stabilization
void correctPos(Manifold& manifold) {
	const double slop = 0.02;
	const double percent = (manifold.count == 1) ? 0.05 : 0.10;
	
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
	// we choose which method we use to solve our manifold based on its dimension
	if (manifold.count == 2)
		resolveContactSurfaceManifold(manifold, dt);
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
