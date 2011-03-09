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
		int clock = mapping.GetMapping(token);
		bool lowerSat = false, upperSat = false;
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			lowerSat = lowerSat || it->Get(clock) > interval.GetLowerBound();
			upperSat = upperSat || it->Get(clock) < interval.GetUpperBound();

			if (upperSat && lowerSat)
				return true;
		}

		if (!upperSat)
			return false;

		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			if (it->Get(clock) != NegInf)
				return true;
		}
		return false;
	}

	size_t MPPMarking::HashKey() const {
		return VerifyTAPN::hash()(dp);
	}

	relation MPPMarking::Relation(const StoredMarking &other) const {
		const MPPMarking &mpp = static_cast<const MPPMarking&> (other);
		bool sub = false, sup = false;

		if (Contains(mpp))
			sup = true;
		if (mpp.Contains(*this))
			sub = true;

		if (sub && sup)
			return EQUAL;
		if (sub)
			return SUBSET;
		if (sup)
			return SUPERSET;

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
		MPVecSet newW;
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			MPVector v = *it;
			v.Set(ConeIdx, 0);
			newW.insert(v);
		}
		newW.insert(W.begin(), W.end());
		V.clear();
		W = newW;
		isCone = true;
	}

	void MPPMarking::ConeToPoly() {
		MPVecSet newV, newW;
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
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
		isCone = false;
	}

	bool MPPMarking::Contains(const MPPMarking& mpp) const {
		MPPMarking G = MPPMarking(*this);
		MPPMarking Gprime = MPPMarking(mpp);

		G.PolyToCone();
		Gprime.PolyToCone();

		for (MPVecIter it = Gprime.W.begin(); it != Gprime.W.end(); ++it)
			if (!G.ContainsPoint(*it))
				return false;

		return true;
	}

	bool MPPMarking::ContainsPoint(const MPVector& v) const {
		//TODO Implement this
		return false;
	}

	void MPPMarking::IntersectHalfspace(const MPVector &a, const MPVector &b) {
		MPVecSet Gleq, Ggt;
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			MPVector g = *it;
			if (a+g <= b+g)
				Gleq.insert(g);
			else
				Ggt.insert(g);
		}
		W = Gleq;
		for (MPVecIter g = Gleq.begin(); g != Gleq.end(); ++g) {
			for (MPVecIter h = Ggt.begin(); h != Ggt.end(); ++g) {
				W.insert(Max(a+(*h)+(*g), b+(*g)+(*h)));
			}
		}
	}

	void MPPMarking::Cleanup() {
			//TODO Implement this
	}
}
