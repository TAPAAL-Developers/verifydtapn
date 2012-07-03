/*
 * TPlibMPP.cpp
 *
 *  Created on: Jun 29, 2012
 *      Author: ravn
 */

#include "TPlibMPP.hpp"

namespace VerifyTAPN {
	void TPlibMPP::InitZero(int numberOfTokens) {
		matrix_t *initGen = matrix_alloc(1, 1 + numberOfTokens);
		for (unsigned int i = 0; i <= numberOfTokens; i++) {
			matrix_set(initGen, 0, i, 0.);
		}
		poly = of_gen(1 + numberOfTokens, initGen);
		matrix_free(initGen);
		set_canonical(POLY, 0);
	}

	void TPlibMPP::FreeClock(int clock) {
#if DEBUG_PRINT
		std::cout << "TPlibMPP::FreeClock" << std::endl;
		std::cout << "clock " << clock << std::endl;
#endif
		int clockArray[1] = { clock };
		semi_project_with(poly, 0, clockArray, 1);
	}

	/*
	 * constrains the polyhedron such that lb <= clock <= ub
	 */
	void TPlibMPP::ConstrainClock(int clock, int ub, int lb) {
		if (lb > 0 || (ub != INT_MAX && ub != INF)) {
			unsigned int numberOfCons = 0;
			bool isUBreal = false;
			bool isLBreal = false;
			if (ub != INT_MAX && ub != INF) {
				numberOfCons++;
				isUBreal = true;
			}
			if (lb > 0) {
				numberOfCons++;
				isLBreal = true;
			}
			if (numberOfCons) {
				unsigned int d = dimension(poly);
				matrix_t *cons = matrix_alloc(numberOfCons, d * 2);
				for (unsigned int j = 0; j < numberOfCons; j++) {
					for (unsigned int i = 0; i < d * 2; i++) {
						matrix_set(cons, j, i, -INFINITY);
					}
				}
				if (isUBreal) {
					matrix_set(cons, 0, 0, 0);
					matrix_set(cons, 0, d + clock, -ub);
					if (isLBreal) {
						matrix_set(cons, 1, clock, 0);
						matrix_set(cons, 1, d, lb);
					}
				} else {
					matrix_set(cons, 0, clock, 0);
					matrix_set(cons, 0, d, lb);
				}
#if DEBUG_PRINT||DEBUG_CONSTRAIN_INTERVAL
				std::cout << "cons matrix:" << std::endl;
				matrix_print(cons);
				std::cout << std::endl;
#endif
				poly_t *consPoly = meet_cons(poly, cons);
				matrix_free(cons);
				poly_free(poly);
				poly = consPoly;
			}
		}
	}

	void TPlibMPP::SwapClocks(int clock1, int clock2) {
		unsigned int d = dimension(poly);
		int perm[d];
		perm[0] = 0;
		for (unsigned int k = 1; k < d; k++) {
			if (k == clock1) {
				perm[k] = clock2;
			} else if (k == clock2) {
				perm[k] = clock1;
			} else {
				perm[k] = k;
			}
		}
#if DEBUG_PRINT || DEBUG_SWAP
		std::cout << "Clock1: " << clock1 << " - Clock2: " << clock2 << std::endl;
		std::cout << "Permutation array [ ";
		for (unsigned int i = 0; i < d; i++) {
			std::cout << perm[i] << " ";
		}
		std::cout << "]" << std::endl;
#endif
		poly_t *permPoly = permute_dimensions(poly, perm);
		poly_free(poly);
		poly = permPoly;
	}

	void TPlibMPP::Print(std::ostream& out) const {
		std::cout << "TPlibMPP" << std::endl;
		std::cout << "Dim: " << dimension(poly) << std::endl;
		print_canonical_type();
		print_poly(poly);
	}

	void TPlibMPP::ResetClock(int clock) {
		int clockArray[1] = { clock };
		reset_with(poly, 0, clockArray, 1);
	}

	bool TPlibMPP::IsEmpty() const {
		return is_bottom(poly);
	}

	void TPlibMPP::Delay() {
		poly_t *delayPoly = delay(poly, 0);
		poly_free(poly);
		poly = delayPoly;
	}

	void TPlibMPP::Constrain(int clock, const TAPN::TimeInterval& interval) {
		if (interval.IsLowerBoundStrict()
				|| (interval.IsUpperBoundStrict()
						&& (interval.GetUpperBound() != INT_MAX && interval.GetUpperBound() != INF))) {
			std::cout
					<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n";
		}
		ConstrainClock(clock, interval.GetUpperBound(), interval.GetLowerBound());
	}

	void TPlibMPP::Constrain(int clock, const TAPN::TimeInvariant& invariant) {
		if (invariant.GetBound() != INT_MAX && invariant.GetBound() != INF) {
			if (invariant.IsBoundStrict()) {
				std::cout
						<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n";
			}
			ConstrainClock(clock, invariant.GetBound(), 0);
		}
	}

	bool TPlibMPP::PotentiallySatisfies(int clock, const TAPN::TimeInterval& interval) const {
		if (interval.IsLowerBoundStrict()
				|| (interval.IsUpperBoundStrict()
						&& (interval.GetUpperBound() != INT_MAX && interval.GetUpperBound() != INF))) {
			std::cout
					<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n";
		}
		unsigned int numberOfCons = 0;
		bool ub = false;
		bool lb = false;
		if (interval.GetUpperBound() != INT_MAX && interval.GetUpperBound() != INF) {
			numberOfCons++;
			ub = true;
		}
		if (interval.GetLowerBound() > 0) {
			numberOfCons++;
			lb = true;
		}
		if (numberOfCons) {
			unsigned int d = dimension(poly);
			matrix_t *cons = matrix_alloc(numberOfCons, d * 2);
			for (unsigned int j = 0; j < numberOfCons; j++) {
				for (unsigned int i = 0; i < d * 2; i++) {
					matrix_set(cons, j, i, -INFINITY);
				}
			}
			if (ub) {
				matrix_set(cons, 0, 0, 0);
				matrix_set(cons, 0, d + clock, -interval.GetUpperBound());
				if (lb) {
					matrix_set(cons, 1, clock, 0);
					matrix_set(cons, 1, d, interval.GetLowerBound());
				}
			} else {
				matrix_set(cons, 0, clock, 0);
				matrix_set(cons, 0, d, interval.GetLowerBound());
			}
#if DEBUG_PRINT
			std::cout << "Cons matrix:" << std::endl;
			matrix_print(cons);
#endif
			int sat = may_sat_cons(poly, cons);
			matrix_free(cons);
#if DEBUG_PRINT
			std::cout << "Pot sat? " << sat << std::endl;
			std::cout << "-----------------" << std::endl;
#endif
			return sat;
		}
#if DEBUG_PRINT
		std::cout << "Pot sat? " << true << std::endl;
		std::cout << "-----------------" << std::endl;
#endif
		return true;
	}

	void TPlibMPP::Extrapolate(const int* maxConstants) {
#if DEBUG_PRINT || DEBUG_EXTRAPOLATE
		std::cout << "before getting dimensions ";
#endif
		unsigned int d = dimension(poly);
#if DEBUG_PRINT || DEBUG_EXTRAPOLATE
		std::cout << "after getting dim - d = " << d << std::endl;
#endif
		for (unsigned int j = 1; j < d; ++j) {
			if (maxConstants[j] == -INF) {
				FreeClock(j);
			}
		}
	}

	relation TPlibMPP::Relation(const TPlibMPP& other) const {
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "TPlibMPP::Relation" << std::endl << "relating..." << std::endl;
#endif
		bool sub = is_leq(poly, other.poly);
		bool sup = is_leq(other.poly, poly);

#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "sub: " << sub << " - is_leq (this,other): " << is_leq(poly, other.poly) << std::endl;
		std::cout << "sup: " << sup << " - is_leq (other,this): " << is_leq(other.poly, poly) << std::endl;
#endif
		if (sub && sup) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation: EQUAL" << std::endl;
			std::cout << "-----------------" << std::endl;
#endif
			return EQUAL;
		}
		if (sup) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation: SUPERSET" << std::endl;
			std::cout << "-----------------" << std::endl;
#endif
			return SUPERSET;
		}
		if (sub) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation: SUBSET" << std::endl;
			std::cout << "-----------------" << std::endl;
#endif
			return SUBSET;
		}
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "Relation: DIFFERENT" << std::endl;
		std::cout << "-----------------" << std::endl;
#endif
		return DIFFERENT;
	}

	void TPlibMPP::AddClocks(int* clockArray, int* newDimArray, int numberOfClocks) {
		poly_t * polyAdd = add_dimensions(poly, clockArray, numberOfClocks, 1);
		poly_free(poly);
		poly = polyAdd;
		reset_with(poly, 0, newDimArray, numberOfClocks);
	}

	void TPlibMPP::RemoveClocks(int* clockArray, int numberOfClocks){
		poly_t *remPoly = remove_dimensions(poly, clockArray, numberOfClocks);
		poly_free(poly);
		poly = remPoly;
	}

	void TPlibMPP::ConvexHullUnion(TPlibMPP* other){
		poly_t* unionPoly = join(poly, other->poly);
		poly_free(poly);
		poly = unionPoly;
	}

	double TPlibMPP::GetLowerDiffBound(int clock1, int clock2) const{
		return lower_difference_bound(poly,clock1,clock2);
	}
}
