#ifndef MPPMARKING_HPP_
#define MPPMARKING_HPP_

#include <set>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"

#include "MPVector.hpp"

namespace VerifyTAPN {

	class MPPMarking: public DiscreteMarking, public StoredMarking {
		friend class MPPMarkingFactory;
	private:
		static MarkingFactory *factory;

		std::set<MPVector> V, W;

		id_type id;

		void initZero();
	public:
		MPPMarking(const DiscretePart &dp) : DiscreteMarking(dp) { };
		MPPMarking(const MPPMarking &mpp) : DiscreteMarking(mpp), V(mpp.V), W(mpp.W) { };
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
