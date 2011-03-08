#include "MPPMarkingFactory.hpp"
#include <cstdlib>

namespace VerifyTAPN {
	SymbolicMarking *MPPMarkingFactory::InitialMarking(const DiscretePart &dp) const {
		return NULL;
	}

	SymbolicMarking *MPPMarkingFactory::Convert(StoredMarking *marking) const {
		return NULL;
	}

	SymbolicMarking *MPPMarkingFactory::Clone(const SymbolicMarking &marking) const {
		return NULL;
	}

	StoredMarking *MPPMarkingFactory::Convert(SymbolicMarking *marking) const {
		return NULL;
	}
}

