/*
 * DebugTPlibMarking.hpp
 *
 *  Created on: May 29, 2012
 *      Author: ravn
 */

#ifndef DEBUGTPLIBMARKING_HPP_
#define DEBUGTPLIBMARKING_HPP_

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <assert.h>
#include <cmath>
#include <float.h>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"
#include "VectorizedMPP.hpp"
#include "TPlibMPPMarking.hpp"

namespace VerifyTAPN {
	class DebugTPlibMarking: public DiscreteMarking, public StoredMarking {
		friend class DiscreteInclusionMarkingFactory;
		friend class DebugTPlibMarkingFactory;
	public:
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;
	private:
		TokenMapping mapping;
		id_type id;
		VectorizedMPP vmpp;
		TPlibMPP tpmpp;
	public:
		DebugTPlibMarking(const DiscretePart &dp) :
				DiscreteMarking(dp), mapping() {
			InitMapping();
		}
		;
		DebugTPlibMarking(const DebugTPlibMarking &marking) :
				DiscreteMarking(marking), mapping(marking.mapping), vmpp(marking.vmpp), tpmpp(marking.tpmpp) {
		}
		;
		virtual ~DebugTPlibMarking() {
		}
		;
	private:

		void InitZero(const std::vector<int>& tokenPlacement) {
			vmpp = VectorizedMPP();
			vmpp.InitZero(NumberOfTokens());
			tpmpp = TPlibMPP();
			tpmpp.InitZero(NumberOfTokens());
#if DEBUG_PRINT || DEBUG_INIT
			Print(std::cout);
#endif
		}
		;
		void InitMapping() {
			for (unsigned int i = 0; i < NumberOfTokens(); i++) {
				mapping.SetMapping(i, i + 1);
			}
		}
		;

		void ComparePolies() {
			matrix_t *vmppMatrix = matrix_alloc(vmpp.NumberOfGens(), dp.size() + 1);
			for (int j = 0; j < vmpp.NumberOfGens(); j++) {
				for (unsigned int i = 0; i < dp.size() + 1; i++) {
					if (vmpp.GetGIndex(j, i) == INT_MIN) {
						matrix_set(vmppMatrix, j, i, -INFINITY);
					} else {
						matrix_set(vmppMatrix, j, i, vmpp.GetGIndex(j, i));
					}
				}
			}
			poly_t *vmppOcaml = of_gen(dp.size() + 1, vmppMatrix);
			if (!(is_leq(vmppOcaml, tpmpp.GetPoly()) && is_leq(tpmpp.GetPoly(), vmppOcaml))) {
				std::cout << "Markings not equal!" << std::endl << "vmppOcaml:" << std::endl;
				print_poly(vmppOcaml);
				Print(std::cout);
				assert(false);
			}
			matrix_free(vmppMatrix);
			poly_free(vmppOcaml);
		}
	protected:
		virtual void Swap(int i, int j) {
#if DEBUG_PRINT || DEBUG_SWAP
			std::cout << std::endl << "Swap" << std::endl;
			std::cout << "i: " << i << " - j: " << j << std::endl << "Before:" << std::endl;
			Print(std::cout);
			std::cout << std::endl << "swapping..." << std::endl;
#endif
			dp.Swap(i, j);
			vmpp.SwapClocks(GetClockIndex(i), GetClockIndex(j));
			tpmpp.SwapClocks(GetClockIndex(i), GetClockIndex(j));
			ComparePolies();
		}
		;
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const {
#if DEBUG_PRINT || DEBUG_ISUPP
			std::cout << "IsUpperPositionGreatherThanPivot" << std::endl;
			Print(std::cout);
#endif
			int placeUpper = dp.GetTokenPlacement(upper);
			int placePivot = dp.GetTokenPlacement(pivotIndex);
			unsigned int clockUpper = GetClockIndex(upper);
			unsigned int clockPivot = GetClockIndex(pivotIndex);
#if DEBUG_PRINT || DEBUG_ISUPP
			std::cout << "clockUpper: " << clockUpper << " - clockPivot: " << clockPivot << std::endl;
#endif
			if (DiscreteMarking::IsUpperPositionGreaterThanPivot(upper, pivotIndex)) {
				return true;
			} else if (placeUpper == placePivot) {
				int VMPP_zero_upper = vmpp.GetUpperDiffBound(0, clockUpper);
				int VMPP_zero_pivot = vmpp.GetUpperDiffBound(0, clockPivot);
				int TPMPP_zero_upper = tpmpp.GetLowerDiffBound(clockUpper, 0);
				int TPMPP_zero_pivot = tpmpp.GetLowerDiffBound(clockPivot, 0);
#if DEBUG_PRINT || DEBUG_ISUPP
				std::cout << "VMPP_zero_upper = " << VMPP_zero_upper << " - TPMPP_zero_upper = " << TPMPP_zero_upper << std::endl;
				std::cout << "VMPP_zero_pivot = " << VMPP_zero_pivot << " - TPMPP_zero_pivot = " << TPMPP_zero_pivot << std::endl;
#endif
				assert((VMPP_zero_upper > VMPP_zero_pivot) == (TPMPP_zero_upper < TPMPP_zero_pivot));
				assert((VMPP_zero_upper == VMPP_zero_pivot) == (TPMPP_zero_upper == TPMPP_zero_pivot));
				if (VMPP_zero_upper > VMPP_zero_pivot) {
					return true;
				} else if (VMPP_zero_upper == VMPP_zero_pivot) {
					int VMPP_upper_zero = vmpp.GetUpperDiffBound(clockUpper, 0);
					int VMPP_pivot_zero = vmpp.GetUpperDiffBound(clockPivot, 0);
					int TPMPP_upper_zero = tpmpp.GetLowerDiffBound(0, clockUpper);
					int TPMPP_pivot_zero = tpmpp.GetLowerDiffBound(0, clockPivot);
#if DEBUG_PRINT || DEBUG_ISUPP
					std::cout << "VMPP_upper_zero = " << VMPP_upper_zero << " - TPMPP_upper_zero = " << TPMPP_upper_zero << std::endl;
					std::cout << "VMPP_pivot_zero = " << VMPP_pivot_zero << " - TPMPP_pivot_zero = " << TPMPP_pivot_zero << std::endl;
#endif
					assert((VMPP_upper_zero > VMPP_pivot_zero) == (TPMPP_upper_zero < TPMPP_pivot_zero));
					assert((VMPP_upper_zero == VMPP_pivot_zero) == (TPMPP_upper_zero == TPMPP_pivot_zero));
					if (VMPP_upper_zero > VMPP_pivot_zero) {
						return true;
					} else if (VMPP_upper_zero == VMPP_pivot_zero) {
						int VMPP_pivot_upper = vmpp.GetUpperDiffBound(clockPivot, clockUpper);
						int VMPP_upper_pivot = vmpp.GetUpperDiffBound(clockUpper, clockPivot);
						int TPMPP_pivot_upper = tpmpp.GetLowerDiffBound(clockUpper, clockPivot);
						int TPMPP_upper_pivot = tpmpp.GetLowerDiffBound(clockPivot, clockUpper);
#if DEBUG_PRINT || DEBUG_ISUPP
						std::cout << "VMPP_upper_pivot = " << VMPP_upper_pivot << " - TPMPP_upper_pivot = " << TPMPP_upper_pivot << std::endl;
						std::cout << "VMPP_pivot_upper = " << VMPP_pivot_upper << " - TPMPP_pivot_upper = " << TPMPP_pivot_upper << std::endl;
#endif
						assert((VMPP_pivot_upper > VMPP_upper_pivot) == (TPMPP_pivot_upper < TPMPP_upper_pivot));
						assert((VMPP_upper_pivot > VMPP_pivot_upper) == ( TPMPP_upper_pivot < TPMPP_pivot_upper));
						return clockPivot > clockUpper ? VMPP_pivot_upper > VMPP_upper_pivot :
								VMPP_upper_pivot > VMPP_pivot_upper;
					}
				}
			}
			return false;
		}
		;

	public:
		virtual void Print(std::ostream& out) const {
			out << std::endl << "Marking: " << id << std::endl;
			out << "dp.size(): " << dp.size() << std::endl;
			out << "Placement:";
			for (unsigned int i = 0; i < NumberOfTokens(); i++) {
				out << " " << GetTokenPlacement(i) << ",";
			}
			out << std::endl;
			out << "Mapping (token:clock):";
			for (unsigned int i = 0; i < NumberOfTokens(); i++) {
				out << " " << i << ":" << GetClockIndex(i) << ",";
			}
			out << std::endl;
			out << "C++ (vectorized)" << std::endl;
			vmpp.Print(out);
			out << "OCaml" << std::endl;
			tpmpp.Print(out);
		}
		;
		virtual id_type UniqueId() const {
			return id;
		}
		;
		virtual unsigned int GetClockIndex(unsigned int token) const {
			return mapping.GetMapping(token);
		}
		;
		virtual void Reset(int token) {
#if DEBUG_PRINT || DEBUG_RESET
			std::cout << "Reset" << std::endl;
			Print(std::cout);
#endif
			vmpp.ResetClock(GetClockIndex(token));
			tpmpp.ResetClock(GetClockIndex(token));
			ComparePolies();
		}
		;
		virtual bool IsEmpty() const {
#if DEBUG_PRINT || DEBUG_ISEMPTY
			std::cout << "IsEmpty" << std::endl;
			Print(std::cout);
#endif
			bool vmppEmpty = vmpp.IsEmpty();
			bool tpmppEmpty = tpmpp.IsEmpty();
			assert(vmppEmpty == tpmppEmpty);
			return vmppEmpty;
		}
		;
		virtual void Delay() {
#if DEBUG_PRINT || DEBUG_DELAY
			std::cout << "Delay" << std::endl;
			Print(std::cout);
#endif
			vmpp.Delay();
			tpmpp.Delay();
			for (unsigned int i = 0; i < NumberOfTokens(); i++) {
				const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
				if (invariant.GetBound() < INF) {
					Constrain(i, invariant);
				}
			}
			vmpp.Cleanup();
			ComparePolies();
		}
		;
		virtual void Constrain(int token, const TAPN::TimeInterval& interval) {
#if DEBUG_PRINT || DEBUG_CONSTRAIN_INTERVAL
			std::cout << "Constrain Interval" << std::endl;
			Print(std::cout);
#endif
			vmpp.Constrain(GetClockIndex(token), interval);
			tpmpp.Constrain(GetClockIndex(token), interval);
			ComparePolies();
		}
		;
		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant) {
#if DEBUG_PRINT
			std::cout << "Constrain Invariant" << std::endl;
			Print(std::cout);
#endif
			vmpp.Constrain(GetClockIndex(token), invariant);
			tpmpp.Constrain(GetClockIndex(token), invariant);
			ComparePolies();
		}
		;
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const {
#if DEBUG_PRINT
			std::cout << "PotentiallySatisfies:" << std::endl;
			Print(std::cout);
#endif
			bool vmppPot = vmpp.PotentiallySatisfies(GetClockIndex(token), interval);
			bool tpmppPot = tpmpp.PotentiallySatisfies(GetClockIndex(token), interval);
			assert(vmppPot == tpmppPot);
			return vmppPot;
		}
		;
		virtual void Extrapolate(const int* maxConstants) {
#if DEBUG_PRINT
			std::cout << "Extrapolate" << std::endl;
			std::cout << "Mapping:";
			for (unsigned int k = 0; k < mapping.size(); k++) {
				std::cout << " " << mapping.GetMapping(k);
			}
			Print(std::cout);
#endif
			vmpp.Extrapolate(maxConstants);
			tpmpp.Extrapolate(maxConstants);
			ComparePolies();
		}
		;
		virtual void AddTokens(const std::list<int>& placeIndices) {
#if DEBUG_PRINT
			std::cout << "AddTokens" << std::endl;
			Print(std::cout);
#endif
			int clockArray[placeIndices.size()];
			int newDimArray[placeIndices.size()];
			int i = 0;
			int oldDim = NumberOfTokens() + 1;
			for (std::list<int>::const_reverse_iterator iter = placeIndices.rbegin(); iter != placeIndices.rend();
					++iter) {
				mapping.SetMapping(NumberOfTokens(), NumberOfTokens() + 1);
				dp.AddTokenInPlace(*iter);
				vmpp.AddClock();
				newDimArray[i] = oldDim + i;
				clockArray[i] = oldDim;
				i++;
			}
			tpmpp.AddClocks(clockArray, newDimArray, placeIndices.size());
			ComparePolies();
		}
		;
		virtual void RemoveTokens(const std::set<int>& tokenIndices) {
#if DEBUG_PRINT
			std::cout << "RemoveTokens" << std::endl;
			Print(std::cout);
#endif
			std::vector<unsigned int> removeClocks;
			int clockArray[tokenIndices.size()];
			int i = 0;
			for (std::set<int>::const_reverse_iterator it = tokenIndices.rbegin(); it != tokenIndices.rend(); ++it) {
				removeClocks.push_back(GetClockIndex(*it));
				clockArray[i] = GetClockIndex(*it);
				i++;
				mapping.RemoveToken(*it);
				dp.RemoveToken(*it);
			}
			for (unsigned int j = 0; j < mapping.size(); ++j) {
				int offset = 0;
				unsigned int currentMapping = GetClockIndex(j);
				for (std::vector<unsigned int>::iterator it = removeClocks.begin(); it != removeClocks.end(); ++it) {
					if (currentMapping > *it) {
						offset++;
					}
				}
				mapping.SetMapping(j, currentMapping - offset);
			}
#if DEBUG_PRINT
			std::cout << "removing tokens..." << std::endl;
#endif
			vmpp.RemoveClocks(removeClocks);
			tpmpp.RemoveClocks(clockArray, tokenIndices.size());
			ComparePolies();
		}
		;

		virtual void ConvexHullUnion(AbstractMarking* marking) {
#if DEBUG_PRINT
			std::cout << "ConvexHullMarking" << std::endl;
#endif
			DebugTPlibMarking* other = static_cast<DebugTPlibMarking*>(marking);
			vmpp.ConvexHullUnion(&(other->vmpp));
			tpmpp.ConvexHullUnion(&(other->tpmpp));
			ComparePolies();
		}
		;
		virtual size_t HashKey() const {
			return VerifyTAPN::hash()(dp);
		}
		;
		virtual relation Relation(const StoredMarking& marking) const {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Relation: " << std::endl;
			Print(std::cout);
#endif
			const DebugTPlibMarking &other = static_cast<const DebugTPlibMarking&>(marking);
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "Other:" << std::endl;
			other.Print(std::cout);
#endif
			relation vmppRel = vmpp.Relation(other.vmpp);
			relation tpmppRel = tpmpp.Relation(other.tpmpp);
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "vmppRel: " << vmppRel << " - tpmppRel: " << tpmppRel << std::endl;
#endif
			assert(vmppRel == tpmppRel);
			return vmppRel;
		}
		;
	};

	boost::shared_ptr<TAPN::TimedArcPetriNet> DebugTPlibMarking::tapn;
}

#endif /* DEBUGTPLIBMARKING_HPP_ */
