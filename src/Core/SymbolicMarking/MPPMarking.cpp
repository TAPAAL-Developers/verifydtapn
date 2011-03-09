#include "MPPMarking.hpp"

namespace VerifyTAPN {
	MarkingFactory* MPPMarking::factory = NULL;

	SymbolicMarking *MPPMarking::Clone() const {
		return factory->Clone(*this);
	}

	void MPPMarking::Reset(int token) {
		// TODO If possible, find a way to avoid replacing the entire sets
		int clock = mapping.GetMapping(token);
		MPVecSet newV, newW;
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			MPVector v = *it;
			v.Set(clock, 0);
			newV.insert(v);
		}
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			MPVector w = *it;
			w.Set(clock, NegInf);
			newW.insert(w);
		}
		V = newV;
		W = newW;
	}

	void MPPMarking::Delay() {
		W.insert(V.begin(), V.end());
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
		// TODO Implement this
	}

	bool MPPMarking::IsEmpty() const {
		return V.empty();
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

	void MPPMarking::InitZero() {
		V.clear();
		W.clear();
		// TODO Check if this is the right size
		V.insert(MPVector(dp.size()));
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		return mapping.GetMapping(token);
	}

	void MPPMarking::InitMapping() {
		std::vector<int> pVector = dp.GetTokenPlacementVector();
		std::vector<unsigned int> map;
		int i = 0;

		for(std::vector<int>::const_iterator iter = pVector.begin(); iter != pVector.end(); ++iter)
		{
			map.push_back(i+1);
			i++;
		}

		mapping = TokenMapping(map);
	}

	void MPPMarking::PolyToCone() {
		MPVecSet newV;
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			MPVector v = *it;
			v.Set(ConeIdx, 0);
			newV.insert(v);
		}
		newV.insert(W.begin(), W.end());
		W.clear();
		V = newV;
	}

	void MPPMarking::ConeToPoly() {
		MPVecSet newV, newW;
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			MPVector v = *it;
			if (v.Get(ConeIdx) == NegInf)
				newW.insert(v);
			else {
				v+=-v.Get(ConeIdx);
				v.Set(ConeIdx, NegInf);
				newV.insert(v);
			}
		}
		V = newV;
		W = newW;
	}
}
