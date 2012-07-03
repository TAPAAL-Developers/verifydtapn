#include "VectorizedMPPMarking.hpp"
#include <cstring>
#include <iostream>
#include <cstdio>

//#include <xmmintrin.h>

namespace VerifyTAPN {

	boost::shared_ptr<TAPN::TimedArcPetriNet> VectorizedMPPMarking::tapn;

	void VectorizedMPPMarking::InitZero(){
		poly.InitZero(NumberOfTokens());
	}

	void VectorizedMPPMarking::InitMapping() {
		for (unsigned int i = 0; i < dp.size(); i++) {
			mapping.SetMapping(i, i + 1);
		}
	}

	void VectorizedMPPMarking::Swap(int x, int y) {
#if DEBUG_PRINT
		std::cout << "Swap!" << std::endl << "Marking BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "Swapping tokens: " << x << " and " << y << std::endl;
		std::cout << "Swapping clocks: " << GetClockIndex(x) << " and " << GetClockIndex(y) << std::endl;
		std::cout << "swapping..." << std::endl;
#endif
		dp.Swap(x, y);
		poly.SwapClocks(GetClockIndex(x), GetClockIndex(y));
#if DEBUG_PRINT
		std::cout << "Marking AFTER:" << std::endl;
		Print(std::cout);
		std::cout << std::endl;
		std::cout << "-----------------" << std::endl;
#endif
	}

	bool VectorizedMPPMarking::IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const {
		int placeUpper = dp.GetTokenPlacement(upper);
		int pivot = dp.GetTokenPlacement(pivotIndex);
		unsigned int mapUpper = GetClockIndex(upper);
		unsigned int mapPivot = GetClockIndex(pivotIndex);
		if( DiscreteMarking::IsUpperPositionGreaterThanPivot(upper, pivotIndex)){
			return true;
		} else if(placeUpper == pivot){
			int zeroUpper = poly.GetUpperDiffBound(0, mapUpper);
			int zeroPivot = poly.GetUpperDiffBound(0, mapPivot);
			if (zeroUpper > zeroPivot){
				return true;
			} else if (zeroUpper == zeroPivot){
				int upperZero = poly.GetUpperDiffBound(mapUpper,0);
				int pivotZero = poly.GetUpperDiffBound(mapPivot,0);
				if (upperZero > pivotZero){
					return true;
				} else if (upperZero == pivotZero){
					int pivotUpper = poly.GetUpperDiffBound(mapPivot,mapUpper);
					int upperPivot = poly.GetUpperDiffBound(mapUpper,mapPivot);
					return mapPivot > mapUpper ? pivotUpper > upperPivot : upperPivot > pivotUpper;
				}
			}
		}
		return false;
	}

	void VectorizedMPPMarking::Print(std::ostream& out) const {
		out << "Marking ID: " << id << std::endl;
		out << "Number of tokens: " << NumberOfTokens() << std::endl;
		out << "Placement: ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			std::cout << GetTokenPlacement(i) << ", ";
		}
		out << std::endl << "Mapping (token:clock): ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			out << i << ":" << GetClockIndex(i) << ", ";
		}
		out << "Poly:" << std::endl;
		poly.Print(out);
		out << std::endl;
	}

	id_type VectorizedMPPMarking::UniqueId() const {
		return id;
	}

	unsigned int VectorizedMPPMarking::GetClockIndex(unsigned int token) const {
		return mapping.GetMapping(token);
	}

	/*
	 * Resets a tokens time to 0
	 */
	void VectorizedMPPMarking::Reset(int token) {
#if DEBUG_PRINT
		std::cout << "Reset!" << std::endl << "BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "resetting.." << std::endl;
#endif
		poly.ResetClock(GetClockIndex(token));
#if DEBUG_PRINT
		std::cout << "AFTER:" << std::endl;
		Print(std::cout);
		std::cout << "-----------------" << std::endl;
#endif
	}

	/*
	 * function assumes that no calculation bring us out of positive space (delay, intersection, reset etc.)
	 */
	bool VectorizedMPPMarking::IsEmpty() const {
		return poly.IsEmpty();
	}

	/*
	 * Delay a polyhedra by making a linear copy of all convex generators
	 */
	void VectorizedMPPMarking::Delay() {
#if DEBUG_PRINT
		std::cout << "Delay!" << std::endl << "Marking BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "delaying..." << std::endl;
#endif
		poly.Delay();
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
			if (invariant.GetBound() < INF) {
				Constrain(i, invariant);
			}
		}
		poly.Cleanup();

#if DEBUG_PRINT
		std::cout << "Marking AFTER:" << std::endl;
		Print(std::cout);
		std::cout << "-----------------" << std::endl;
#endif
	}

	/*
	 * freeing the constraints on a token
	 */
	void VectorizedMPPMarking::Free(int token) {
#if DEBUG_PRINT
		std::cout << "Free!" << std::endl << "BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "freeing.." << std::endl;
#endif

		poly.FreeClock(GetClockIndex(token));
		poly.Cleanup();

#if DEBUG_PRINT
		std::cout << "AFTER:" << std::endl;
		Print(std::cout);
		std::cout << "-----------------" << std::endl;
#endif
	}

	/*
	 * NOTE: since we don't have any good representation of strictness with max-plus
	 * polyhedra, there is an implicit conversion of strict constraints to non-strict
	 * with the same bound. This may cause incorrect behaviour.
	 */
	void VectorizedMPPMarking::Constrain(int token, const TAPN::TimeInterval& interval) {
#if DEBUG_PRINT || DEBUG_CONSTRAIN_INTERVAL
		std::cout << "Constrain(TimeInterval)!" << std::endl << "Marking BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "Constraining clock: " << GetClockIndex(token) << " on interval: " << interval.GetLowerBound()
		<< " <= clock <= " << interval.GetUpperBound() << std::endl << "constraining..." << std::endl;
#endif
		poly.Constrain(GetClockIndex(token), interval);
#if DEBUG_PRINT||DEBUG_CONSTRAIN_INTERVAL
		std::cout << "Marking AFTER:" << std::endl;
		Print(std::cout);
		std::cout << "----------------------" << std::endl;
#endif
	}
	/*
	 * Since we do not have a good representation for strct constraints with
	 * max-plus polyhedra, in this function there is an implicit conversion of
	 * strict constraints to non-strict. This may cause incorrect behaviour when
	 * dealing with models contain strict constraints.
	 */
	void VectorizedMPPMarking::Constrain(int token, const TAPN::TimeInvariant& invariant) {
#if DEBUG_PRINT
		std::cout << "Constrain(TimeInvariant)!" << std::endl;
#endif
		poly.Constrain(GetClockIndex(token), invariant);
	}

	bool VectorizedMPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const {
#if DEBUG_PRINT
		std::cout << "PotentiallySatisfies!" << std::endl;
#endif
		return poly.PotentiallySatisfies(GetClockIndex(token), interval);
	}

	/*
	 * extrapolation - not compleate but sound
	 * principles: freeing unbounded clocks, and theorems 4.9 and 4.11
	 */
	void VectorizedMPPMarking::Extrapolate(const int* maxConstants) {
#if DEBUG_PRINT
		std::cout << "Extrapolate!" << std::endl << "Marking BEFORE:" << std::endl;
		Print(std::cout);
		std::cout << "Max constants: ";
		for (unsigned int i = 1; i < dp.size(); i++) {
			std::cout << maxConstants[i] << ", ";
		}
		std::cout << std::endl << "extrapolating..." << std::endl;
#endif
		poly.Extrapolate(maxConstants);
		poly.Cleanup();
#if DEBUG_PRINT
		std::cout << "Marking AFTER: " << std::endl;
		Print(std::cout);
		std::cout << std::endl;
#endif
	}

	void VectorizedMPPMarking::ConvexHullUnion(AbstractMarking* marking) {
		VectorizedMPPMarking* mppm = static_cast<VectorizedMPPMarking*>(marking);
		poly.ConvexHullUnion(&(mppm->poly));
		poly.Cleanup();
	}

	size_t VectorizedMPPMarking::HashKey() const {
		return VerifyTAPN::hash()(dp);
	}

	relation VectorizedMPPMarking::Relation(const StoredMarking& other) const {
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "Relation!" << std::endl << "Left poly:" << std::endl;
		Print(std::cout);
		std::cout << "Right poly:" << std::endl;
		other.Print(std::cout);
#endif
		const VectorizedMPPMarking &mppm = static_cast<const VectorizedMPPMarking&>(other);
		return poly.Relation(mppm.poly);
	}

	void VectorizedMPPMarking::AddTokens(const std::list<int>& placeIndicies) {
#if DEBUG_PRINT
		std::cout << "AddTokens!" << std::endl;
#endif
		for (std::list<int>::const_reverse_iterator iter = placeIndicies.rbegin(); iter != placeIndicies.rend();
				++iter) {
			poly.AddClock();
			mapping.SetMapping(NumberOfTokens(), NumberOfTokens() + 1);
			dp.AddTokenInPlace(*iter);
		}
	}

	void VectorizedMPPMarking::RemoveTokens(const std::set<int>& tokenIndices) {
#if DEBUG_PRINT
		std::cout << "RemoveTokens!" << std::endl;
#endif
		//Since we have to remove clocks one at a time, we have to guarantee that we remove them in order.
		std::vector<unsigned int> removeClocks;
		for (std::set<int>::const_reverse_iterator it = tokenIndices.rbegin(); it != tokenIndices.rend(); ++it) {
			removeClocks.push_back(mapping.GetMapping(*it));
			mapping.RemoveToken(*it);
			dp.RemoveToken(*it);
		}
		for (unsigned int j = 0; j < mapping.size(); ++j) {
			int offset = 0;
			unsigned int currentMapping = mapping.GetMapping(j);
			for (std::vector<unsigned int>::iterator it = removeClocks.begin(); it != removeClocks.end(); ++it) {
				if (currentMapping > *it)
					offset++;
			}
			mapping.SetMapping(j, currentMapping - offset);
		}
		poly.RemoveClocks(removeClocks);
	}
}

