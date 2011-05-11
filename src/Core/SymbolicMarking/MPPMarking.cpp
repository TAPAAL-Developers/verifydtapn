#include "MPPMarking.hpp"
#include <iostream>
#include <algorithm>
#include <set>

namespace VerifyTAPN {
	boost::shared_ptr<TAPN::TimedArcPetriNet> MPPMarking::tapn;

	void MPPMarking::AddTokens(const std::list<int>& placeIndices) {
		int oldDimension = dp.size()+1;
		unsigned int newToken = NumberOfTokens();

		unsigned int i = 0;
		for(std::list<int>::const_iterator iter = placeIndices.begin(); iter != placeIndices.end(); ++iter)
		{
			for (MPVecIter v = V.begin(); v != V.end(); ++v)
				v->AddDim(0);
			for (MPVecIter w = W.begin(); w != W.end(); ++w)
				w->AddDim(NegInf);
			mapping.SetMapping(newToken, oldDimension+i);
			dp.AddTokenInPlace(*iter);
			i++;
			newToken++;
		}
	}

	void MPPMarking::RemoveTokens(const std::vector<int>& tokenIndices) {
		//Since we have to remove clocks one at a time, we have to guarantee that we remove them in order.
		std::vector<unsigned int> removeClocks;
		for(int i = tokenIndices.size()-1; i >= 0; --i) {
			removeClocks.push_back(mapping.GetMapping(tokenIndices[i]));
			mapping.RemoveToken(tokenIndices[i]);
			dp.RemoveToken(tokenIndices[i]);
		}
		for(unsigned int j = 0; j < mapping.size(); ++j) {
			int offset = 0;
			unsigned int currentMapping = mapping.GetMapping(j);
			for (std::vector<unsigned int>::iterator it = removeClocks.begin(); it != removeClocks.end(); ++it) {
				if (currentMapping>*it)
					offset++;
			}
			mapping.SetMapping(j, currentMapping - offset);
		}
		//std::sort(removeClocks.begin(), removeClocks.end());
		for (std::vector<unsigned int>::reverse_iterator it = removeClocks.rbegin(); it != removeClocks.rend(); ++it) {

			/*{
				if(mapping.GetMapping(j) > *it)
					mapping.SetMapping(j, *it-1);
			}*/
			for (MPVecIter v = V.begin(); v != V.end(); ++v)
				v->RemoveDim(*it);
			for (MPVecIter w = W.begin(); w != W.end(); ++w)
				w->RemoveDim(*it);
		}
	}

	void MPPMarking::Print() const {
		std::cout << "V: ";
		std::set<MPVector> sortedV, sortedW;
		sortedV.insert(V.begin(), V.end());
		for (MPVecConstIter it = W.begin(); it != W.end(); ++it)
			sortedW.insert(it->Normalize());
		for (std::set<MPVector>::iterator it = sortedV.begin(); it != sortedV.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
		std::cout << "W: ";
		for (std::set<MPVector>::iterator it = sortedW.begin(); it != sortedW.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
		std::cout << "Placement vector:";
		std::vector<int> places = dp.GetTokenPlacementVector();
		for (std::vector<int>::iterator place = places.begin(); place != places.end(); ++place)
			std::cout << " " << *place;
		std::cout << "\n";
		std::cout << "Mapping vector:";
		for (unsigned int i = 0; i < dp.size(); i++)
			std::cout << " " << mapping.GetMapping(i);
		std::cout << "\n";
	}

	void MPPMarking::Reset(int token) {
		LOG(std::cout << "Reset("<<token<<")\n")
		LOG(std::cout << "input:\n")
		LOG(Print());
		ResetClock(GetClockIndex(token));
		Cleanup();
		LOG(std::cout << "output:\n")
		LOG(Print());
	}

	void MPPMarking::Delay() {
		LOG(std::cout << "Delay()\n")
		LOG(std::cout << "input:\n")
		LOG(Print());
		W.insert(W.end(), V.begin(), V.end());
		for(unsigned int i = 0; i < NumberOfTokens(); i++)
		{
			const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
			if(invariant.GetBound() != std::numeric_limits<int>::max()) {
				LOG(std::cout << "Found invariant in place " << GetTokenPlacement(i) << "\n";)
			}
			Constrain(i, invariant);
		}
		Cleanup();
		LOG(std::cout << "output:\n")
		LOG(Print());
	}

	void MPPMarking::Free(int token) {
		FreeClock(mapping.GetMapping(token));
		Cleanup();
	}

	void MPPMarking::ResetClock(int clock) {
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			it->Set(clock, 0);
		}
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			it->Set(clock, NegInf);
		}
	}

	void MPPMarking::FreeClock(int clock) {
		ResetClock(clock);
		MPVector g = MPVector(dp.size(), NegInf);
		g.Set(clock, 0);
		W.push_back(g);
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
		LOG(std::cout << "Extrapolate(...)\n");
		LOG(std::cout << "input:\n")
		LOG(Print());
		for (size_t i = FirstClock; i <= dp.size(); i++) {
			int k = maxConstants[i];
			if (k == -INF) {
				FreeClock(i);
				continue;
			}
			MPVecSet T, U;
			for (MPVecIter v = V.begin(); v != V.end(); ++v) {
				if (v->Get(i) <= k)
					T.push_back(*v);
				else
					U.push_back(*v);
			}
			if (U.empty())
				continue;
			bool hori = true;
			if (DiagonalFree(T,U,i)) {
				for (MPVecIter it = V.begin(); it != V.end(); ++it) {
					if (it->Get(i) > k)
						it->Set(i,k+1);
					}
			} else {
				for (size_t j = FirstClock; j <= dp.size(); j++) {
					if (i != j) {
						for (MPVecIter u = U.begin(); u != U.end(); ++u) {
							if (u->Get(i) > k || u->Get(j) > maxConstants[j-1]) {
								MPVector ex = MPVector(dp.size(), NegInf);
								ex.Set(i, u->Get(i));
								ex.Set(j, u->Get(j));
								W.push_back(ex);
							}
							if (u->Get(i) < u->Get(j) + k)
								hori = false;
						}
					}
				}
			}
			if (hori) {
				MPVector ex = MPVector(dp.size(), NegInf);
				ex.Set(i, 0);
				W.push_back(ex);
			}
		}
		LOG(std::cout << "output:\n")
		LOG(Print());
	}

	bool MPPMarking::IsEmpty() const {
		return V.empty();
	}

	//Adds the constraint x-y<=value.
	void MPPMarking::doConstrain(int x, int y, int value) {
		MPVector a = MPVector(dp.size(), NegInf);
		MPVector b = a;
		a.Set(x, 0);
		if (value != INF) {
			b.Set(y, value);
			/*std::cout << "a: " << a << "\n";
			std::cout << "b: " << b << "\n";*/
			IntersectHalfspace(a,b);
		}
	}

	void MPPMarking::Constrain(int token, const TAPN::TimeInterval &interval) {
//		if (interval.IsLowerBoundStrict() || (interval.IsUpperBoundStrict() && interval.GetUpperBound() != 2147483647))
//			std::cerr << "WARNING: Interval has strictness\n";
		LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
		LOG(std::cout << "input:\n")
		LOG(Print());
		PolyToCone();
		int clock = GetClockIndex(token);
		MPVector a = MPVector(dp.size(), NegInf);
		MPVector b = a;
		a.Set(clock, 0);
		if (interval.GetUpperBound() != std::numeric_limits<int>::max()) {
			b.Set(ConeIdx, interval.GetUpperBound());
			IntersectHalfspace(a,b);
			LOG(ConeToPoly();
			Cleanup();
			std::cout << "After " << clock << " <= " << interval.GetUpperBound() << "\n";
			Print();
			PolyToCone();)
		}

		a.Set(clock, NegInf);
		b.Set(ConeIdx, NegInf);

		a.Set(ConeIdx, 0);
		b.Set(clock, -interval.GetLowerBound());
		IntersectHalfspace(a,b);

		ConeToPoly();
		Cleanup();
		LOG(std::cout << "After " << clock << " >= " << interval.GetLowerBound() << "\n";)
		LOG(Print();)
	}

	void MPPMarking::Constrain(int token, const TAPN::TimeInvariant& invariant)
	{
		if(invariant.GetBound() != std::numeric_limits<int>::max())
		{
			PolyToCone();
			doConstrain(mapping.GetMapping(token), 0, invariant.GetBound());
			ConeToPoly();
			Cleanup();
			//Constrain(token, TAPN::TimeInterval(false, 0, invariant.GetBound(), invariant.IsBoundStrict()));
		}
	};

	bool MPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval &interval) const {
		int clock = GetClockIndex(token);
		bool lowerSat = false, upperSat = false;
		for (MPVecConstIter it = V.begin(); it != V.end(); ++it) {
			lowerSat = lowerSat || it->Get(clock) >= interval.GetLowerBound();
			upperSat = upperSat || it->Get(clock) <= interval.GetUpperBound();

			if (upperSat && lowerSat)
				return true;
		}

		if (!upperSat)
			return false;

		for (MPVecConstIter it = W.begin(); it != W.end(); ++it) {
			if (it->Get(clock) != NegInf)
				return true;
		}
		return false;
	}

	void MPPMarking::ConvexUnion(AbstractMarking* marking) {
		MPPMarking* m = static_cast<MPPMarking*>(marking);
		V.insert(V.end(), m->V.begin(), m->V.end());
		W.insert(W.end(), m->W.begin(), m->W.end());
	}

	size_t MPPMarking::HashKey() const {
		return VerifyTAPN::hash()(dp);
	}

	relation MPPMarking::Relation(const StoredMarking &other) const {
		const MPPMarking &mpp = static_cast<const MPPMarking&> (other);

		bool sup = Contains(mpp);
		bool sub = mpp.Contains(*this);

		if (sub && sup) {
			return EQUAL;
		}
		if (sub) {
			return SUBSET;
		}
		if (sup) {
			return SUPERSET;
		}
		return DIFFERENT;
	}

	void MPPMarking::InitZero() {
		V.clear();
		W.clear();
		V.push_back(MPVector(dp.size()));
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		unsigned int retVal = mapping.GetMapping(token);
		return retVal;
	}

	void MPPMarking::InitMapping() {
		for(unsigned int i = 0; i < dp.size(); i++) {
			mapping.SetMapping(i, i+1);
		}
	}

	void MPPMarking::PolyToCone() {
		for (MPVecIter it = V.begin(); it != V.end(); ++it) {
			it->Set(ConeIdx, 0);
		}
		W.splice(W.end(), V);
		isCone = true;
	}

	void MPPMarking::ConeToPoly() {
		MPVecIter it = W.begin();
		while (it != W.end()) {
			if (it->Get(ConeIdx) != NegInf) {
				*it+=-it->Get(ConeIdx);
				it->Set(ConeIdx, NegInf);
				V.push_back(*it);
				it = W.erase(it);
				continue;
			}
			++it;
		}
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

	bool MPPMarking::ContainsPoint(const MPVector& v, MPVecIter* skipit) const {
		int count = W.size();
		if (skipit)
			count--;
		int* y = new int[count];
		for (int i = 0; i < count; i++)
			y[i] = INF;

		int i = 0;
		for (MPVecConstIter it = W.begin(); it != W.end(); ++it) {
			if (skipit && *skipit == it)
				continue;
			for (size_t j = 0; j <= dp.size(); j++) {
				if (it->Get(j) == NegInf)
					;
				else if (v.Get(j) == NegInf)
					y[i] = NegInf;
				else
					y[i] = MIN(y[i], v.Get(j) - it->Get(j));
			}
			++i;
		}

		MPVector z(dp.size(), NegInf);
		i = 0;
		for (MPVecConstIter it = W.begin(); it != W.end(); ++it) {
			if (skipit && *skipit == it)
				continue;
			z = Max(z, y[i] + (*it));
			i++;
		}
		delete[] y;
		return v == z;
	}

	void MPPMarking::IntersectHalfspace(const MPVector &a, const MPVector &b) {
		MPVecSet Gleq, Ggt;
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			MPVector g = *it;
			if (a+g <= b+g) {
				Gleq.push_back(g);
			}
			else {
				Ggt.push_back(g);
			}
		}
		W = Gleq;
		for (MPVecIter g = Gleq.begin(); g != Gleq.end(); ++g) {
			for (MPVecIter h = Ggt.begin(); h != Ggt.end(); ++h) {
				MPVector p1 = a+(*h)+(*g);
				MPVector p2 = b+(*g)+(*h);
				MPVector p = Max(p1, p2);
				W.push_back(p);
			}
		}
	}

	void MPPMarking::Cleanup() {
		PolyToCone();

		MPVecIter it = W.begin();
		while (it!=W.end()) {
			if(ContainsPoint(*it, &it)) {
				it=W.erase(it);
				continue;
			}
			++it;
		}

		ConeToPoly();
	}

	bool MPPMarking::DiagonalFree(MPVecSet L, MPVecSet H, size_t idx) {
		if(L.empty())
			return true;
		for(size_t i=FirstClock; i<=dp.size(); ++i) {
			if(i!=idx) {
				int minL = INT_MAX;
				int minH = INT_MAX;
				int maxL = NegInf;
				int maxH = NegInf;
				for(MPVecIter it=L.begin(); it!=L.end(); ++it) {
					minL = MIN(minL, it->Get(i));
					maxL = MAX(maxL, it->Get(i));
				}
				for(MPVecIter it=H.begin(); it!=H.end(); ++it) {
					minH = MIN(minH, it->Get(i));
					maxH = MAX(maxH, it->Get(i));
				}
				if(minL<minH || maxL<maxH) {
					return false;
				}
			}
		}
		return true;
	}
}
