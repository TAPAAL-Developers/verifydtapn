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
		poly.InitZero(NumberOfTokens());
		#if DEBUG_PRINT || DEBUG_INIT
			poly.Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::InitMapping() {
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			mapping.SetMapping(i, i + 1);
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
		poly.SwapClocks(GetClockIndex(i),GetClockIndex(j));
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
			double zeroUpper = poly.GetLowerDiffBound(mapUpper, 0);
			double zeroPivot = poly.GetLowerDiffBound(mapPivot, 0);
			if (zeroUpper > zeroPivot) {
				return true;
			} else if (zeroUpper == zeroPivot) {
				double upperZero = poly.GetLowerDiffBound(0, mapUpper);
				double pivotZero = poly.GetLowerDiffBound(0, mapPivot);
				if (upperZero > pivotZero) {
					return true;
				} else if (upperZero == pivotZero) {
					double pivotUpper = poly.GetLowerDiffBound(mapUpper, mapPivot);
					double upperPivot = poly.GetLowerDiffBound(mapPivot, mapUpper);
					return mapPivot > mapUpper ? pivotUpper > upperPivot : upperPivot > pivotUpper;
				}
			}
		}
		return false;
	}

	void TPlibMPPMarking::Print(std::ostream& out) const {
		out << "Marking id: " << id << std::endl;
		std::cout << "Number of tokens: " << NumberOfTokens() << std::endl
				<< "Placement: ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			out << GetTokenPlacement(i) << ", ";
		}
		out << std::endl << "Mapping (token:clock): ";
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			out << i << ":" << GetClockIndex(i) << ", ";
		}
		out << std::endl << "Poly:" << std::endl;
		poly.Print(out);
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
			poly.ResetClock(GetClockIndex(token));
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
			std::cout << "Is empty? " << poly.IsEmpty() << std::endl << std::endl;
			std::cout << "-----------------" << std::endl;
		#endif
			return poly.IsEmpty();
	}

	void TPlibMPPMarking::Delay() {
		#if DEBUG_PRINT || DEBUG_DELAY
			std::cout << "Delay!" << std::endl << "Marking BEFORE:" << std::endl;
			Print(std::cout);
			std::cout << "delaying..." << std::endl << "^^^^^^^^^^^^^^^^^^^^" << std::endl;
		#endif
			poly.Delay();
		for (unsigned int i = 0; i < NumberOfTokens(); i++) {
			const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
			poly.Constrain(GetClockIndex(i), invariant);
		}
		#if DEBUG_PRINT || DEBUG_DELAY
			std::cout << "^^^^^^^^^^^^^^^^^" << std::endl << "Marking AFTER Delay:" << std::endl;
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
		poly.FreeClock(GetClockIndex(token));
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
			poly.Constrain(GetClockIndex(token),interval);
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
			poly.Constrain(GetClockIndex(token),invariant);
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
			return poly.PotentiallySatisfies(GetClockIndex(token),interval);
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
			poly.Extrapolate(maxConstants);
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
		const TPlibMPPMarking &mppm = static_cast<const TPlibMPPMarking&>(other);
		#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation!" << std::endl;
			std::cout << "Left marking:" << std::endl;
			Print(std::cout);
			std::cout << std::endl;
			std::cout << "Right marking:" << std::endl;
			mppm.Print(std::cout);
			std::cout << std::endl;
		#endif
			return poly.Relation(mppm.poly);
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
			i++;
		}
		poly.AddClocks(addDim,newDim,placeIndices.size());
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
		poly.RemoveClocks(remDims, tokenIndices.size());
		#if DEBUG_PRINT
			std::cout << "Marking AFTER:" << std::endl;
			Print(std::cout);
			std::cout << "-----------------" << std::endl;
		#endif
	}

	void TPlibMPPMarking::ConvexHullUnion(AbstractMarking* marking){
		TPlibMPPMarking* m = static_cast<TPlibMPPMarking*>(marking);
		poly.ConvexHullUnion(&(m->poly));
	}
}
