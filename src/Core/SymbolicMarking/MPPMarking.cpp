#include "MPPMarking.hpp"

namespace VerifyTAPN {
	MarkingFactory* MPPMarking::factory = NULL;

	SymbolicMarking *MPPMarking::Clone() const {
		return factory->Clone(*this);
	}

	void MPPMarking::Reset(int token) {
		// TODO Implement this
	}

	void MPPMarking::Delay() {
		// TODO Implement this
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
		// TODO Implement this
	}

	bool MPPMarking::IsEmpty() const {
		// TODO Implement this
		return false;
	}

	void MPPMarking::Constrain(int token, const TAPN::TimeInterval &interval) {
		// TODO Implement this
	}

	bool MPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval &interval) const {
		// TODO Implement this
		return false;
	}

	size_t MPPMarking::HashKey() const {
		return VerifyTAPN::hash()(dp);
	}

	relation MPPMarking::Relation(const StoredMarking &other) const {
		// TODO Implement this
		return DIFFERENT;
	}

	void MPPMarking::initZero() {
		// TODO Check if this is the right size
		V.insert(MPVector(dp.size()));
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	void MPPMarking::AddTokens(const std::list<int>& placeIndices) {
		// TODO Awaiting reply from Morten about whether this is necessary to implement, or if DiscreteMarking is supposed to handle that
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		// TODO Possible off-by-one error here, check this
		return token;
	}
}
