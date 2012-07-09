#ifndef VECTORIZEDMPPMARKING_HPP
#define VECTORIZEDMPPMARKING_HPP

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

#include "VectorizedMPP.hpp"

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"

namespace VerifyTAPN {
	class VectorizedMPPMarking: public DiscreteMarking, public StoredMarking {
		friend class DiscreteInclusionMarkingFactory;
		friend class VectorizedMPPMarkingFactory;
	public:
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;
	private:
		// data
		TokenMapping mapping;
		id_type id;
		VectorizedMPP poly;
	public:
		//constructors
		VectorizedMPPMarking(const DiscretePart &dp) :
				DiscreteMarking(dp), mapping(){
			InitMapping();
		}
		;
		VectorizedMPPMarking(const VectorizedMPPMarking &mppm) :
				DiscreteMarking(mppm.dp), mapping(mppm.mapping), poly(mppm.poly) {
		}
		;
		virtual ~VectorizedMPPMarking() {
		}
		;

	private:
		//internal functions
		void InitZero();
		void InitMapping();

	protected:
		virtual void Swap(int i, int j);
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;

	public:
		virtual void Print(std::ostream& out) const;
		virtual id_type UniqueId() const;
		virtual unsigned int GetClockIndex(unsigned int token) const;
		virtual void Reset(int token);
		virtual bool IsEmpty() const;
		virtual void Delay();
		virtual void Free(int token);
		virtual void Constrain(int token, const TAPN::TimeInterval& interval);
		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant);
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const;
		virtual void Extrapolate(const int* maxConstants);
		virtual void ConvexHullUnion(AbstractMarking* marking);
		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::set<int>& tokenIndices);
	};
}

#endif /* VECTORIZEDMPPMARKING_HPP */
