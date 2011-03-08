#include "MPPMarking.hpp"

namespace VerifyTAPN {
	SymbolicMarking *MPPMarking::Clone() const {
		return NULL;
	}

	void MPPMarking::Reset(int token) {
	}

	void MPPMarking::Delay() {
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
	}

	bool MPPMarking::IsEmpty() const {
		return false;
	}

	void MPPMarking::Constrain(int token, const TAPN::TimeInterval &interval) {
	}

	bool MPPMarking::PotentiallySatisfies(int token,
			const TAPN::TimeInterval &interval) const {
		return false;
	}

	size_t MPPMarking::HashKey() const {
		return 0;
	}

	relation MPPMarking::Relation(const StoredMarking &other) const {
		return DIFFERENT;
	}
}
