#ifndef MPPMARKING_HPP_
#define MPPMARKING_HPP_

#include <set>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"

#include "MPVector.hpp"

namespace VerifyTAPN {

	typedef std::set<MPVector> MPVecSet;
	typedef MPVecSet::iterator MPVecIter;

	class MPPMarking: public DiscreteMarking, public StoredMarking {
		friend class MPPMarkingFactory;
	private:
		MPVecSet V, W;
		TokenMapping mapping;
		id_type id;
		size_t clocks;

		void InitZero();
		void InitMapping();

		void PolyToCone();
		void ConeToPoly();
		bool isCone;

		bool Contains(const MPPMarking& mpp) const;
		bool ContainsPoint(const MPVector& v) const;
		void IntersectHalfspace(const MPVector &a, const MPVector &b);
		void Cleanup();
		bool DiagonalFree(MPVecSet L, MPVecSet H, size_t idx);

		void Print() const;
	public:
		static MarkingFactory *factory;

		MPPMarking(const DiscretePart &dp) : DiscreteMarking(dp), isCone(false) { InitMapping(); clocks = dp.size(); };
		MPPMarking(const MPPMarking &mpp) : DiscreteMarking(mpp), V(mpp.V), W(mpp.W), mapping(mpp.mapping), clocks(mpp.clocks), isCone(false) { };

		virtual ~MPPMarking() { };

		virtual id_type UniqueId() const;
		virtual unsigned int GetClockIndex(unsigned int token) const;

		virtual SymbolicMarking* Clone() const;
		virtual void Reset(int token);
		virtual bool IsEmpty() const;
		virtual void Delay();
		virtual void Constrain(int token, const TAPN::TimeInterval& interval);
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const;
		virtual void Extrapolate(const int* maxConstants);

		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;
	};

}
#endif /* MPPMARKING_HPP_ */
