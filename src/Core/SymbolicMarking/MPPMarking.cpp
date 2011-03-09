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
		W.insert(V.begin(), V.end());
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
		V.clear();
		W.clear();
		// TODO Check if this is the right size
		V.insert(MPVector(dp.size()));
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		// TODO Possible off-by-one error here, check this
		return token;
	}
}
