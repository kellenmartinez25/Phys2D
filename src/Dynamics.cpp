#include "Dynamics.h"
#include "Collision.h"
#include "MathUtils.h"
#include "DebugTools.h"
#include <limits>

void resolveFrictionPoint(Manifold& m, ContactPoint& cp);
void resolveContactPoint(Manifold& m, ContactPoint& cp, double dt);
void resolveContactSurfaceManifold(Manifold& m, double dt);

// Get the velocity of a point r, relative to the obj's center.
// This includes the object's linear and rotational velocity
inline glm::dvec2 getPointVelocity(PhysObject& obj, glm::dvec2 r) {
	// vP = v + (w x r)
	return obj.velocity + Cross(obj.angularVelocity, r);
}

// Calculate the (delta) impulse from a Manifold's contact point (cp).
// Requires the time-step value from the engines update.
void resolveContactPoint(Manifold& m, ContactPoint& cp, double dt) {
	auto vA = getPointVelocity(*m.A, cp.rA);
	auto vB = getPointVelocity(*m.B, cp.rB);
	// relative velocity, from A (ref) to B (inc)
	glm::dvec2 vRel = vB - vA;
	// relative velocity along the collision normal
	double vRelNorm = glm::dot(vRel, m.normal);

	// dont do anything if we are moving slow and penetrating shallow
	if (fabs(vRelNorm) < 0.02 && cp.penetration < 0.02)
		vRelNorm = 0.0;
	// stabilize the resulting impulse by applying a correctional velocity target
	const double beta = 0.05, slop = 0.02, maxBias = 0.2; 
	double bias = beta * std::max(cp.penetration - slop, 0.0) / dt;
	bias = std::min(bias, maxBias);

	// if the vRel along the normal is > 0, then the two objects are moving apart (at this contact point)
	// meaning that we shouldnt resolve anything in this case.
	if (vRelNorm > 0.0 && cp.penetration <= slop)
		return;
	// get K value
	double k = cp.impulseDenominator > 0.0 ? (1.0 / cp.impulseDenominator) : 0.0;
	// target correctional velocity
	double v_target = bias;
	// impulse magnitude
	// this is dependant on -vRelNorm, because the impulse should act in the opposite direction of the
	// direction where objects are moving inside each other
	// we also put in a v_target here, because in cases were vRelNorm is low and yet they're penetrating deeply,
	// we still need to apply a correctional impulse
	double j = (v_target - vRelNorm) * k;
	// get the previous iteration's normal impulse
	double oldImpulse = cp.normImpulse;
	// the accumulated normal impulse. we clamp it to > 0.0, because impulses shouldnt be negative (never pull them together)
	double newImpulse = glm::max(oldImpulse + j, 0.0);
	// set the accumulated normal impulse for the next solver iteration
	cp.normImpulse = newImpulse;
	// compute the change in impulse from this iteration to the previous iteration
	double delta_j = newImpulse - oldImpulse;
	// get the impulse vector along the normal, and apply it to the contact point & objects
	glm::dvec2 J = delta_j * m.normal;
	applyImpulse(*m.A, *m.B, cp, J);
}

// try to resolve a contact surface's impulses using rigid body constraint solving
void resolveContactSurfaceManifold(Manifold& m, double dt) {
	// helper defines
	PhysObject& A = *m.A;
	PhysObject& B = *m.B;
	glm::dvec2& n = m.normal;
	// this is meant for 2 point manifolds, dont do anything for 1 point ones.
	if (m.count != 2) return;

	// i will solve this system of equations:
	// K * dj = b
	// K = 2x2 impulse-const matrix
	// b = velocity error vector (bias)
	// each element K_ij = how contact j's impulse effect contact i's velocity on the normal
	
	// helper variables because i dont wanna type the full thing out everyt ime
	auto& c0 = m.points[0];
	auto& c1 = m.points[1];
	
	glm::dvec2& rA0 = c0.rA;
	glm::dvec2& rB0 = c0.rB;
	glm::dvec2& rA1 = c1.rA;
	glm::dvec2& rB1 = c1.rB;
	// relative velocity vectors between A and B (for both contacts)
	auto v0 = getPointVelocity(B, rB0) - getPointVelocity(A, rA0);	// vRel A->B for contact 0
	auto v1 = getPointVelocity(B, rB1) - getPointVelocity(A, rA1);	// vRel A->B for contact 1

	// biases/stabilization for both contacts, depending on their penetration
	const double beta = 0.08, slop = 0.02, maxBias = 0.5;
	double bias0 = beta * std::max(c0.penetration - slop, 0.0) / dt;
	double bias1 = beta * std::max(c1.penetration - slop, 0.0) / dt;
	// cap the biases
	bias0 = std::min(bias0, maxBias);
	bias1 = std::min(bias1, maxBias);

	// calculate relative velocities along the contact normal
	double vn0 = glm::dot(v0, n);
	double vn1 = glm::dot(v1, n);
	// bias vector
	// b = [b0]
	//	   [b1]
	double b0 = bias0 - vn0;
	double b1 = bias1 - vn1;
	// pre compute cross products
	double rA0xn = Cross(rA0, n);	// r_A0 x n
	double rB0xn = Cross(rB0, n);
	double rA1xn = Cross(rA1, n);
	double rB1xn = Cross(rB1, n);

	// construct the K matrix
	double K00, K11, K01, K10;
	// For each element of K:
	// K_ij = m_A^-1 + m_B^-1 + ((r_Ai x n) * (r_Aj x n) * I_A^-1) + ((r_Bi x n) * (r_Bj x n) * I_B^-1) 

	K00 = A.invMass + B.invMass +
		rA0xn * rA0xn * A.invInertia +
		rB0xn * rB0xn * B.invInertia;
	K11 = A.invMass + B.invMass +
		rA1xn * rA1xn * A.invInertia +
		rB1xn * rB1xn * B.invInertia;
	K01 = A.invMass + B.invMass +
		rA0xn * rA1xn * A.invInertia +
		rB0xn * rB1xn * B.invInertia;
	K10 = K01;	// K_10 is actually the same as K_01 (due to communative property) so i can use it againe

	// aquire the impulses from the previous frame/iteration and calculate the change
	// this is the same thing we do in the 1D manifold solver (except its way more complicated now, thank you supreme court eunuch ChatGPT)
	double old0 = c0.normImpulse;
	double old1 = c1.normImpulse;
	double new0 = old0;
	double new1 = old1;
	bool solved = false;
	{
		// try to solve K * dj = b
		// spoiler: dj = K^-1 * b
		// so now we can get the impulse changes to correct both of our contact velocities

		// det(K)
		double det = K00 * K11 - K01 * K10;
		// a small determinate (0 or near 00) means that the K matrix is un-invertible and this cannot be solved
		if (fabs(det) > 1e-9) {
			// if we can invert it then continue solving
			// get 1 / det(K)
			double invDet = 1.0 / det;

			// achieve the inverse of the 2x2 matrix:
			// K^-1 = 1/det * [  K11 -K01 ]
			//				  [ -K10  K00 ]
			// to then get dj = K^-1 * b
			
			// solve dj0
			double d0 = invDet * (K11 * b0 - K01 * b1);
			// solve dj1
			double d1 = invDet * (-K10 * b0 + K00 * b1);
			// these might be dj but thats your job to find out
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
	// if at least 1 of the contacts were negative, then it wasn't solved.
	// first, assume that c1 was invalid; disable it
	// and solve only c0.
	if (!solved) {
		// calculate dj_0
		double d0 = 0.0;
		if (K00 > 0.0) {
			d0 = b0 / K00;
		}
		// disable c1
		new1 = 0.0;
		// clamp the accumulated impulse to be >0
		new0 = std::max(old0 + d0, 0.0);
		// mark as solved
		solved = true;
	}

	// if the case was that contact 0 was invalid, then we do the same thing as before but with contact 1 instead
	if (new0 == 0.0) {
		double d1 = 0.0;
		if (K11 > 0.0) {
			d1 = b1 / K11;
		}
		// disable c0
		new0 = 0.0;
		// accumulate impulse
		new1 = std::max(old1 + d1, 0.0);
	}

	// now, with things in expected conditions, we can compute the changes in impulse.
	// because of the multiple iterations, we apply only dj so that we dont keep on doing the same impulse application on every iteration
	double delta_j0 = new0 - old0;
	double delta_j1 = new1 - old1;
	// update our accumulated impulses, for usage by the next iteration and prestep
	c0.normImpulse = new0;
	c1.normImpulse = new1;
	
	// we can calculate our J vectors
	glm::dvec2 J0 = delta_j0 * n;
	glm::dvec2 J1 = delta_j1 * n;

	// apply the solved impulses
	applyImpulse(A, B, c0, J0);
	applyImpulse(A, B, c1, J1);
}


// solves the friction step on a single-point contact (and manifold)
void resolveFrictionPoint(Manifold& m, ContactPoint& cp) {
	// helper vars
	PhysObject& A = *m.A;
	PhysObject& B = *m.B;
	glm::dvec2& n = m.normal;
	glm::dvec2 t = glm::dvec2(-n.y, n.x); // tangent of normal (rotated 90)
	// get relative velocity between A and B
	glm::dvec2 vA = getPointVelocity(A, cp.rA);
	glm::dvec2 vB = getPointVelocity(B, cp.rB);
	glm::dvec2 vRel = vB - vA;
	// calculate relative velocity along the tangent
	double vRelTan = glm::dot(vRel, t);
	// since the tangent impulse should stop/slow down any relative motion along the tangent,
	// the applied tangential impulse should be in the *opposite* direction of vRelTan
	double j_tangent = -vRelTan * cp.tangentImpulseDenominator;
	
	// friction coefficient
	const double mu = 0.1;
	// per coulombs friction:
	// |J_t| <= mu * J_norm
	// so the maximum amount of fricitonal force is limited by mu * normImpulse
	double maxFriction = mu * cp.normImpulse;

	// accumulate the tangent impulse from the previous iteration //

	// get the previous iteration's tan impulse
	double oldTanImpulse = cp.tangentImpulse;
	// clamp the magnitude of the accumulated tan impulse between +-maxFriction (per Coulombs law)
	cp.tangentImpulse =
		glm::clamp(oldTanImpulse + j_tangent,
			-maxFriction,
			maxFriction);
	// get the change in tangent impulse
	double dj_tangent = cp.tangentImpulse - oldTanImpulse;
	// tangent impulse vector along the tangent
	glm::dvec2 J_tangent = dj_tangent * t;
	// apply the tangential impulse vector to the contact point
	applyImpulse(A, B, cp, J_tangent);
}

// Resolve two clipping bodies in a Manifold using Baumgarte stabilization
void correctPos(Manifold& manifold) {
	// stabilization bias consts
	const double slop = 0.02;
	// use lower scale for single contacts due to them being less stable, overcorrection could cause bouncy
	double percent = (manifold.count == 1) ? 0.05 : 0.10;
	
	// find the contact point with maximum penetration; this is the one to use to calc the bias
	double maxPenetration = 0.0;
	for (int i = 0; i < manifold.count; i++) {
		maxPenetration = std::max(maxPenetration, manifold.points[i].penetration);
	}
	// calc the correction amount
	double correctionMag = std::max(maxPenetration - slop, 0.0) * percent;
	// we correct the position along the contact normal
	glm::dvec2 correction = correctionMag * manifold.normal;
	// sum of m^-1 across both bodies
	double totalInvMass = manifold.A->invMass + manifold.B->invMass;

	// total m^-1 might be 0.0 if both objects are static
	if (totalInvMass == 0.0)
		return;
	// apply to both objects
	// static objects have an invMass=0.0; dont worry they wont move during correction
	manifold.A->pos -= correction * (manifold.A->invMass / totalInvMass);
	manifold.B->pos += correction * (manifold.B->invMass / totalInvMass);
}
// Solve a 1-2 Point collision manifold, using the engine's timestep dt.
 
void collide2D(Manifold& manifold, double dt) {
	// we choose which method we use to solve our manifold based on its dimension
	 if (manifold.count == 2) {
		 resolveContactSurfaceManifold(manifold, dt);
		 // solve the friction on both points independantly
		 resolveFrictionPoint(manifold, manifold.points[0]);
		 resolveFrictionPoint(manifold, manifold.points[1]);
	 }
	 else {
		 // solve the normal and tangential impulses sequentially
		 resolveContactPoint(manifold, manifold.points[0], dt);
		 resolveFrictionPoint(manifold, manifold.points[0]);
	 }
}

// Apply an impulse vector J at a ContactPoint, onto objects A and B.
// A should be the reference object, and B should be the incident object.
void applyImpulse(PhysObject& A, PhysObject& B, ContactPoint& point, glm::dvec2 J) {
	// apply impulse to linear velocity
	A.velocity -= J * A.invMass;
	B.velocity += J * B.invMass;
	// apply impulse to angular velocity (omega)
	A.angularVelocity -= A.invInertia * Cross(point.rA, J);
	B.angularVelocity += B.invInertia * Cross(point.rB, J);
}

// dynamics pre-step
// this calculates the needed vectors for the manifold and contact points (that weren't already done in the first generation)
// and also does warm-start things
void preStep(Manifold& manifold) {
	// iterate over each contact point
	for (int i = 0; i < manifold.count; i++) {
		// the point we're working with
		auto& cp = manifold.points[i];
		// calc rA and rB, which is the vector from the center of the object to the contact point
		cp.rA = cp.pos - manifold.A->center(); // A -> cp
		cp.rB = cp.pos - manifold.B->center(); // B -> cp

		// pre-compute r x n
		double r_A_crossN = Cross(cp.rA, manifold.normal);
		double r_B_crossN = Cross(cp.rB, manifold.normal);

		// calculate K
		double K = ((manifold.A->invMass + manifold.B->invMass) +
			(r_A_crossN * r_A_crossN * manifold.A->invInertia) +
			(r_B_crossN * r_B_crossN * manifold.B->invInertia));
		// assign the impulse denominator to 1/K, checking for dividing by 0 cases:
		cp.impulseDenominator = (K > 0.0) ? (1.0 / K) : 0.0;

		// tangent / friction //
		// calculate the tangent normal vector, t
		glm::dvec2 tangent = glm::dvec2(-manifold.normal.y, manifold.normal.x);
		// pre-compute r x t
		double rAxT = Cross(cp.rA, tangent);
		double rBxT = Cross(cp.rB, tangent);
		// calculate K_tangent
		double K_Tangent =
			manifold.A->invMass +
			manifold.B->invMass +
			rAxT * rAxT * manifold.A->invInertia +
			rBxT * rBxT * manifold.B->invInertia;
		// assign inverse of K_tangent, checking for 1/0 cases.
		cp.tangentImpulseDenominator = (K_Tangent > 0.0) ? (1.0 / K_Tangent) : 0.0;

		// warm-start //
		
		// if this is a single-contact manifold, then we shouldn't warm-start.
		if (manifold.count == 1) {
			// reset accumulated impulses from previous frame
			cp.normImpulse = 0.0;
			cp.tangentImpulse = 0.0;
			// move onto next point
			continue;
		}
		// calculate warm start Impulse from the previous frame's normal and tangent
		glm::dvec2 J =
			cp.normImpulse * manifold.normal +
			cp.tangentImpulse * tangent;
		// apply warmstart impulse
		applyImpulse(*manifold.A, *manifold.B, cp, J);
	}
}
