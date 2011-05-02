#ifndef MPPMARKING_HPP_
#define MPPMARKING_HPP_

#include <list>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"

#include "MPVector.hpp"

namespace VerifyTAPN {

	typedef std::list<MPVector> MPVecSet;
	typedef MPVecSet::iterator MPVecIter;
	typedef MPVecSet::const_iterator MPVecConstIter;

	class MPPMarking: public DiscreteMarking, public StoredMarking {
		friend class MPPMarkingFactory;
	private:
		MPVecSet V, W;
		id_type id;
#ifdef DBM_NORESIZE
		size_t clocks;
#endif

		void InitZero();
		void InitMapping();

		void PolyToCone();
		void ConeToPoly();
		bool isCone;

		bool Contains(const MPPMarking& mpp) const;
		bool ContainsPoint(const MPVector& v, MPVecIter* skipit = NULL) const;
		void IntersectHalfspace(const MPVector &a, const MPVector &b);
		void Cleanup();
		bool DiagonalFree(MPVecSet L, MPVecSet H, size_t idx);

		void Print() const;
		void ResetClock(int clock);
		void FreeClock(int clock);

		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;
#ifdef DBM_NORESIZE
		MPPMarking(const DiscretePart &dp, int clocks) : DiscreteMarking(dp), clocks(clocks), isCone(false) { InitMapping(); };
		MPPMarking(const DiscretePart &dp, int clocks, MPVecSet v, MPVecSet w) : DiscreteMarking(dp),  V(v), W(w), clocks(clocks), isCone(false) { InitMapping(); };
		MPPMarking(const MPPMarking &mpp) : DiscreteMarking(mpp), V(mpp.V), W(mpp.W), clocks(mpp.clocks), isCone(false) { };
#else
		MPPMarking(const DiscretePart &dp) : DiscreteMarking(dp), isCone(false) { InitMapping(); };
		MPPMarking(const DiscretePart &dp, MPVecSet v, MPVecSet w) : DiscreteMarking(dp),  V(v), W(w), isCone(false) { InitMapping(); };
		MPPMarking(const MPPMarking &mpp) : DiscreteMarking(mpp), V(mpp.V), W(mpp.W), isCone(false) { };
#endif

		virtual ~MPPMarking() { };

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
		virtual void ConvexUnion(AbstractMarking* marking);

		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;

#ifndef DBM_NORESIZE
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::vector<int>& tokenIndices);
#endif

	};

}
#endif /* MPPMARKING_HPP_ */
