#ifndef MPPMARKING_HPP_
#define MPPMARKING_HPP_

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"

namespace VerifyTAPN {

	class MPPMarking: public SymbolicMarking, public StoredMarking {
	public:
		virtual SymbolicMarking* Clone() const;
		virtual void Reset(int token);
		virtual bool IsEmpty() const;
		virtual void Delay();
		virtual void Constrain(int token, const TAPN::TimeInterval& interval);
		virtual bool PotentiallySatisfies(int token,
				const TAPN::TimeInterval& interval) const;
		virtual void Extrapolate(const int* maxConstants);

		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;
	};

}
#endif /* MPPMARKING_HPP_ */
