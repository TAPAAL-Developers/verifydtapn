/*
 * TPlibMPPMarking.cpp
 *
 *  Created on: May 9, 2012
 *      Author: ravn
 */

#include "TPlibMPPMarking.hpp"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <climits>

namespace VerifyTAPN {

	boost::shared_ptr<TAPN::TimedArcPetriNet> TPlibMPPMarking::tapn;

	void TPlibMPPMarking::InitZero() {
		matrix_t *initGen = matrix_alloc(1, 1 + NumberOfTokens());
		for (unsigned int i = 0; i <= NumberOfTokens(); i++) {
			matrix_set(initGen, 0, i, 0.);
		}
		poly = of_gen(1 + NumberOfTokens(), initGen);
		matrix_free(initGen);
		set_canonical(POLY,0);

		#if DEBUG_PRINT || DEBUG_INIT
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::InitMapping() {
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			mapping.SetMapping(i, i + 1);
		}
	}

	void TPlibMPPMarking::FreeClock(int j) {
		int clock[1] = { j };
		semi_project_with(poly, 0, clock, 1);
	}

	/*
	 * constrains the polyhedron such that lb <= clock <= ub
	 */
	void TPlibMPPMarking::ConstrainClock(int clock, int ub, int lb) {
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
				unsigned int d = NumberOfTokens() + 1;
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

	void TPlibMPPMarking::Swap(int i, int j) {
		#if DEBUG_PRINT || DEBUG_SWAP
			std::cout << "Swap!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Swapping tokens: " << i << " and " << j << std::endl;
			std::cout << "Swapping clocks: " << GetClockIndex(i) << " and " << GetClockIndex(j) << std::endl;
			std::cout << "swapping..." << std::endl;
		#endif

		dp.Swap(i, j);
		int perm[NumberOfTokens() + 1];
		perm[0] = 0;
		for (unsigned int k = 0; k < NumberOfTokens(); k++) {
			if (k == i) {
				perm[GetClockIndex(k)] = GetClockIndex(j);
			} else if (k == j) {
				perm[GetClockIndex(k)] = GetClockIndex(i);
			} else {
				perm[GetClockIndex(k)] = GetClockIndex(k);
			}
		}

		#if DEBUG_PRINT || DEBUG_SWAP
			std::cout << "Permutation array [ ";
			for (unsigned int i = 0; i <= NumberOfTokens(); i++) {
				std::cout << perm[i] << " ";
			}
			std::cout << "]" << std::endl;
		#endif

		poly_t *permPoly = permute_dimensions(poly, perm);
		poly_free(poly);
		poly = permPoly;

		#if DEBUG_PRINT || DEBUG_SWAP
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	bool TPlibMPPMarking::IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const {
		int placeUpper = dp.GetTokenPlacement(upper);
		int pivot = dp.GetTokenPlacement(pivotIndex);
		unsigned int mapUpper = GetClockIndex(upper);
		unsigned int mapPivot = GetClockIndex(pivotIndex);
		if (mapPivot > NumberOfTokens() + 1) {
			std::cout << "*";
		}

		if (DiscreteMarking::IsUpperPositionGreaterThanPivot(upper, pivotIndex)) {
			return true;
		} else if (placeUpper == pivot) {
			double zeroUpper = lower_difference_bound(poly, mapUpper, 0);
			double zeroPivot = lower_difference_bound(poly, mapPivot, 0);
			if (zeroUpper > zeroPivot) {
				return true;
			} else if (zeroUpper == zeroPivot) {
				double upperZero = lower_difference_bound(poly, 0, mapUpper);
				double pivotZero = lower_difference_bound(poly, 0, mapPivot);
				if (upperZero > pivotZero) {
					return true;
				} else if (upperZero == pivotZero) {
					double pivotUpper = lower_difference_bound(poly, mapUpper, mapPivot);
					double upperPivot = lower_difference_bound(poly, mapPivot, mapUpper);
					return mapPivot > mapUpper ? pivotUpper > upperPivot : upperPivot > pivotUpper;
				}
			}
		}
		return false;
	}

	void TPlibMPPMarking::Print(std::ostream& out) const {
		out << "Marking id: " << id << std::endl << "Number of tokens: " << NumberOfTokens() << std::endl
				<< "Placement: ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			out << GetTokenPlacement(i) << ", ";
		}
		out << std::endl << "Mapping (token:clock): ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			out << i << ":" << GetClockIndex(i) << ", ";
		}
		out << std::endl << "Poly:" << std::endl;
		print_poly(poly);
	}

	id_type TPlibMPPMarking::UniqueId() const {
		return id;
	}

	unsigned int TPlibMPPMarking::GetClockIndex(unsigned int token) const {
		return mapping.GetMapping(token);
	}

	void TPlibMPPMarking::Reset(int token) {
		#if DEBUG_PRINT || DEBUG_RESET
			std::cout << "Reset!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Resetting token: " << token << std::endl;
			std::cout << "Resetting clock: " << GetClockIndex(token) << std::endl;
			std::cout << "resetting..." << std::endl;
		#endif

		int clock[1] = { GetClockIndex(token) };
		reset_with(poly, 0, clock, 1);

		#if DEBUG_PRINT || DEBUG_RESET
			std::cout << "Marking AFTER:";
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	bool TPlibMPPMarking::IsEmpty() const {
		#if DEBUG_PRINT || DEBUG_ISEMPTY
			std::cout << "IsEmpty!" << std::endl << "Marking:" << std::endl;
			Print(std::cout);
			std::cout << "Is empty? " << is_bottom(poly) << std::endl << std::endl;
			std::cout << "-----------------" << std::endl;
		#endif

		return is_bottom(poly);
	}

	void TPlibMPPMarking::Delay() {
		#if DEBUG_PRINT || DEBUG_DELAY
			std::cout << "Delay!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "delaying..." << std::endl;
		#endif

		poly_t *delayPoly = delay(poly, 0);
		poly_free(poly);
		poly = delayPoly;

		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
			Constrain(i, invariant);
		}

		#if DEBUG_PRINT || DEBUG_DELAY
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::Free(int token) {
		#if DEBUG_PRINT || DEBUG_FREE
			std::cout << "Free!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Freeing token: " << token << std::endl;
			std::cout << "Freeing clock: " << GetClockIndex(token) << std::endl;
			std::cout << "freeing..." << std::endl;
		#endif

		FreeClock(GetClockIndex(token));

		#if DEBUG_PRINT || DEBUG_FREE
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::Constrain(int token, const TAPN::TimeInterval& interval) {
		#if DEBUG_PRINT || DEBUG_CONSTRAIN_INTERVAL
			std::cout << "Constrain(TimeInteravl)!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Constraining clock: " << GetClockIndex(token) << " on interval: " << interval.GetLowerBound()
					<< " <= clock <= " << interval.GetUpperBound() << std::endl << "constraining..." << std::endl;
		#endif

		if (interval.IsLowerBoundStrict()
				|| (interval.IsUpperBoundStrict()
						&& (interval.GetUpperBound() != INT_MAX && interval.GetUpperBound() != INF))) {
			std::cout
					<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n";
		}
		ConstrainClock(GetClockIndex(token), interval.GetUpperBound(), interval.GetLowerBound());

		#if DEBUG_PRINT || DEBUG_CONSTRAIN_INTERVAL
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::Constrain(int token, const TAPN::TimeInvariant& invariant) {
		#if DEBUG_PRINT
			std::cout << "Constrain(TimeInvariant)!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Constraining clock: " << GetClockIndex(token) << " on invariant: " << " clock <= "
					<< invariant.GetBound() << std::endl;
			std::cout << "constraining..." << std::endl;
		#endif

		if (invariant.GetBound() != INT_MAX && invariant.GetBound() != INF) {
			if (invariant.IsBoundStrict()) {
				std::cout
						<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n";
			}
			ConstrainClock(GetClockIndex(token), invariant.GetBound(), 0);
		}

		#if DEBUG_PRINT
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	bool TPlibMPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const {
		#if DEBUG_PRINT
			std::cout << "PotentiallySatisfies!" << std::endl << "Marking:" << std::endl;
			Print(std::cout);
			std::cout << "clock: " << GetClockIndex(token) << " on interval: " << interval.GetLowerBound()
					<< " <= clock <= " << interval.GetUpperBound() << std::endl;
		#endif

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
			unsigned int d = NumberOfTokens() + 1;
			matrix_t *cons = matrix_alloc(numberOfCons, d * 2);
			for (unsigned int j = 0; j < numberOfCons; j++) {
				for (unsigned int i = 0; i < d * 2; i++) {
					matrix_set(cons, j, i, -INFINITY);
				}
			}
			if (ub) {
				matrix_set(cons, 0, 0, 0);
				matrix_set(cons, 0, d + GetClockIndex(token), -interval.GetUpperBound());
				if (lb) {
					matrix_set(cons, 1, GetClockIndex(token), 0);
					matrix_set(cons, 1, d, interval.GetLowerBound());
				}
			} else {
				matrix_set(cons, 0, GetClockIndex(token), 0);
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

	/*
	 * Only freeing of inactive clocks implemented.
	 * TODO add full extrapolation when supported by TPlib
	 */
	void TPlibMPPMarking::Extrapolate(const int* maxConstants) {
		#if DEBUG_PRINT
			std::cout << "Extrapolate!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Max constants :";
			for (unsigned int i = 0; i <= NumberOfTokens(); i++) {
				std::cout << maxConstants[i] << ", ";
			}
			std::cout << std::endl << "extrapolating..." << std::endl;
		#endif

		for (unsigned int j = 1; j <= NumberOfTokens(); ++j) {
			if (maxConstants[j] == -INF) {
				FreeClock(j);
			}
		}

		#if DEBUG_PRINT
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	size_t TPlibMPPMarking::HashKey() const {
		return VerifyTAPN::hash()(dp);
	}

	relation TPlibMPPMarking::Relation(const StoredMarking& other) const {
		const TPlibMPPMarking &mpp = static_cast<const TPlibMPPMarking&>(other);

		#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation!" << std::endl;
			std::cout << "Left marking:" << std::endl;
			Print(std::cout);
			std::cout << std::endl;
			std::cout << "Right marking:" << std::endl;
			mpp.Print(std::cout);
			std::cout << std::endl;
		#endif

		bool sub = is_leq(poly, mpp.poly);
		bool sup = is_leq(mpp.poly, poly);

		#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "sub: " << sub << " - is_leq (this,other): " << is_leq(poly, mpp.poly) << std::endl;
			std::cout << "sup: " << sup << " - is_leq (other,this): " << is_leq(mpp.poly, poly) << std::endl;
		#endif

		if (sup && sup) {
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

	void TPlibMPPMarking::AddTokens(const std::list<int>& placeIndices) {
		#if DEBUG_PRINT
			std::cout << "AddTokens!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Number of tokens to add: " << placeIndices.size() << std::endl;
			std::cout << "Place incices: [ ";
			for (std::list<int>::const_reverse_iterator it = placeIndices.rbegin(); it != placeIndices.rend(); ++it) {
				std::cout << (*it) << ", ";
			}
			std::cout << "]" << std::endl << "adding tokens..." << std::endl;
		#endif

		unsigned int newToken = NumberOfTokens();
		int oldDim = NumberOfTokens() + 1;
		int addDim[placeIndices.size()];
		int newDim[placeIndices.size()];
		unsigned int i = 0;
		for (std::list<int>::const_reverse_iterator it = placeIndices.rbegin(); it != placeIndices.rend(); ++it) {
			mapping.SetMapping(newToken, NumberOfTokens() + 1);
			dp.AddTokenInPlace(*it);
			newToken++;
			addDim[i] = oldDim;
			newDim[i] = oldDim + i;
		}
		poly_t * polyAdd = add_dimensions(poly, addDim, placeIndices.size(), 1);
		poly_free(poly);
		poly = polyAdd;
		reset_with(poly, 0, newDim, placeIndices.size());

		#if DEBUG_PRINT
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::RemoveTokens(const std::set<int>& tokenIndices) {
		#if DEBUG_PRINT
			std::cout << "RemoveTokens!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "Number of tokens to remove: " << tokenIndices.size() << std::endl << "Tokens to remove: [ ";
			for (std::set<int>::const_reverse_iterator it = tokenIndices.rbegin(); it != tokenIndices.rend(); ++it) {
				std::cout << *it << " ";
			}
			std::cout << " ]" << std::endl << "Clocks to remove: [ ";
			for (std::set<int>::const_reverse_iterator it = tokenIndices.rbegin(); it != tokenIndices.rend(); ++it) {
				std::cout << GetClockIndex(*it) << ", ";
			}
			std::cout << " ]" << std::endl << "removing tokens..." << std::endl;
		#endif

		int remDims[tokenIndices.size()];
		int i = 0;
		for (std::set<int>::const_reverse_iterator it = tokenIndices.rbegin(); it != tokenIndices.rend(); ++it) {
			remDims[i] = GetClockIndex(*it);
			i++;
			mapping.RemoveToken(*it);
			dp.RemoveToken(*it);
		}
		for (unsigned int j = 0; j < NumberOfTokens(); ++j) {
			int offset = 0;
			unsigned int currentMapping = GetClockIndex(j);
			for (unsigned int i = 0; i < tokenIndices.size(); i++) {
				if (currentMapping > remDims[i]) {
					offset++;
				}
			}
			mapping.SetMapping(j, currentMapping - offset);
		}
		poly_t *remPoly = remove_dimensions(poly, remDims, tokenIndices.size());
		poly_free(poly);
		poly = remPoly;

		#if DEBUG_PRINT
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::ConvexHullUnion(AbstractMarking* marking){
		TPlibMPPMarking* m = static_cast<TPlibMPPMarking*>(marking);
		poly_t* unionPoly = join(poly, m->poly);
		poly_free(poly);
		poly = unionPoly;
	}
}
