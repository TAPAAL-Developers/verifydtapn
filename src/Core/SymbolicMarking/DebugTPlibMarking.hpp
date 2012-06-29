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
		TPlibMPPMarking *tpmpp;
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
			vmpp.InitZero(dp.size());
			TPlibMPPMarking::tapn = tapn;
			tpmpp = new TPlibMPPMarking(DiscretePart(tokenPlacement));
			tpmpp->InitZero();
			tpmpp->dp = dp;

			Print(std::cout);
		}
		;
		void InitMapping() {
			for (unsigned int i = 0; i < NumberOfTokens(); i++) {
				mapping.SetMapping(i, i + 1);
			}
		}
		;

		void CompareMarkings() {
			matrix_t *vmppMatrix = matrix_alloc(vmpp.NumberOfGens(), dp.size()+1);
			for (int j = 0; j < vmpp.NumberOfGens(); j++) {
				for (unsigned int i = 0; i < dp.size()+1; i++) {
					if (vmpp.GetGIndex(j,i) == INT_MIN) {
						matrix_set(vmppMatrix, j, i, -INFINITY);
					} else {
						matrix_set(vmppMatrix, j, i, vmpp.GetGIndex(j,i));
					}
				}
			}
			poly_t *vmppOcaml = of_gen(dp.size()+1, vmppMatrix);
			if (!(is_leq(vmppOcaml, tpmpp->poly) && is_leq(tpmpp->poly, vmppOcaml))) {
				std::cout << "Markings not equal!" << std::endl;
				std::cout << "vmppOcaml:" << std::endl;
				print_poly(vmppOcaml);

				Print(std::cout);
				assert(false);
			}
			matrix_free(vmppMatrix);
			poly_free(vmppOcaml);
		}
	protected:
		virtual void Swap(int i, int j) {
			std::cout << "Swap" << std::endl;
			std::cout << "i: " << i << " - j: " << j << std::endl;
			std::cout << "Mapping:";
			for (unsigned int k = 0; k < mapping.size(); k++) {
				std::cout << " " << mapping.GetMapping(k);
			}
			std::cout << std::endl;
			Print(std::cout);
			tpmpp->dp = dp;

			dp.Swap(i, j);
			vmpp.SwapClocks(GetClockIndex(i), GetClockIndex(j));
			tpmpp->Swap(i, j);

			CompareMarkings();
		}
		;
/*		virtual bool IsUpperPositionGreatherThanPivot(int upper, int pivotIndex) const {
			std::cout << "IsUpperPositionGreatherThanPivot" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);

			bool vmppIsU = vmpp->IsUpperPositionGreaterThanPivot(upper, pivotIndex);
			bool tpmppIsU = tpmpp->IsUpperPositionGreaterThanPivot(upper, pivotIndex);
			std::cout << "vmpp is upper: " << vmppIsU << " - tpmpp is upper: " << tpmppIsU << std::endl;
			return vmpp->IsUpperPositionGreaterThanPivot(upper, pivotIndex);
		}
		;*/
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
			tpmpp->Print(out);
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
			std::cout << "Reset" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);

			vmpp.ResetClock(GetClockIndex(token));
			tpmpp->Reset(token);

			CompareMarkings();
		}
		;
		virtual bool IsEmpty() const {
			std::cout << "IsEmpty" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			bool vmppEmpty = vmpp.IsEmpty();
			bool tpmppEmpty = tpmpp->IsEmpty();
			std::cout << "vmpp empty? " << vmppEmpty << " - tpmpp empty? " << tpmppEmpty << std::endl;

			return vmppEmpty;
		}
		;
		virtual void Delay() {
			std::cout << "Delay" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			vmpp.Delay();
			tpmpp->Delay();

			CompareMarkings();
		}
		;
		virtual void Constrain(int token, const TAPN::TimeInterval& interval) {
			std::cout << "Constrain Interval" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			vmpp.Constrain(GetClockIndex(token), interval);
			tpmpp->Constrain(token, interval);

			CompareMarkings();
		}
		;
		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant) {
			std::cout << "Constrain Invariant" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			vmpp.Constrain(GetClockIndex(token), invariant);
			tpmpp->Constrain(token, invariant);

			CompareMarkings();
		}
		;
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const {
			std::cout << "PotentiallySatisfies:" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			bool vmppPot = vmpp.PotentiallySatisfies(GetClockIndex(token), interval);
			bool tpmppPot = tpmpp->PotentiallySatisfies(token, interval);
			std::cout << "vmpp potentially sat: " << vmppPot << " - tpmpp: " << tpmppPot << std::endl;

			return vmppPot;
		}
		;
		virtual void Extrapolate(const int* maxConstants) {
			std::cout << "Extrapolate" << std::endl;
			tpmpp->dp = dp;
			std::cout << "Mapping:";
			for (unsigned int k = 0; k < mapping.size(); k++) {
				std::cout << " " << mapping.GetMapping(k);
			}
			Print(std::cout);
			vmpp.Extrapolate(maxConstants);
			tpmpp->Extrapolate(maxConstants);

			CompareMarkings();
		}
		;
		virtual void AddTokens(const std::list<int>& placeIndices) {
			std::cout << "AddTokens" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			unsigned int newToken = NumberOfTokens();
			for (std::list<int>::const_reverse_iterator iter = placeIndices.rbegin(); iter != placeIndices.rend();
					++iter) {
				mapping.SetMapping(newToken, NumberOfTokens() + 1);
				dp.AddTokenInPlace(*iter);
				newToken++;
			}

			vmpp.AddClock();
			tpmpp->AddTokens(placeIndices);

			CompareMarkings();
		}
		;
		virtual void RemoveTokens(const std::set<int>& tokenIndices) {
			std::cout << "RemoveTokens" << std::endl;
			tpmpp->dp = dp;

			Print(std::cout);

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
					if (currentMapping > *it) {
						offset++;
					}
				}
				mapping.SetMapping(j, currentMapping - offset);
			}
			std::cout << "removing tokens..." << std::endl;
			vmpp.RemoveClocks(removeClocks);
			tpmpp->RemoveTokens(tokenIndices);
			Print(std::cout);

			CompareMarkings();
		}
		;
		/* TODO broken */
		virtual void ConvexHullUnion(AbstractMarking* marking) {
			std::cout << "ConvexHullMarking" << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);
			//vmpp.ConvexHullUnion(marking);
			tpmpp->ConvexHullUnion(marking);

			CompareMarkings();
		}
		;
		virtual size_t HashKey() const {
			return VerifyTAPN::hash()(dp);
		}
		;
		virtual relation Relation(const StoredMarking& marking) const {
			std::cout << "Relation: " << std::endl;
			tpmpp->dp = dp;
			Print(std::cout);

			const DebugTPlibMarking *dbt = static_cast<const DebugTPlibMarking*>(&marking);
			TPlibMPPMarking *tMarking = new TPlibMPPMarking(dp, mapping, *(dbt->tpmpp->poly));
			relation vmppRel = vmpp.Relation(dbt->vmpp);
			relation tpmppRel = tpmpp->Relation(*tMarking);
			std::cout << "vmpp relation: " << vmppRel << " - tpmpp relation " << tpmppRel << std::endl;

			//delete(dbt);
			//delete(vMarking);
			//delete(tMarking);

			return vmppRel;
		}
		;
	};

	boost::shared_ptr<TAPN::TimedArcPetriNet> DebugTPlibMarking::tapn;
}

#endif /* DEBUGTPLIBMARKING_HPP_ */
