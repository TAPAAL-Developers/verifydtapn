#include "MPPMarking.hpp"
#include <iostream>
#include <algorithm>

#ifndef DBM_NORESIZE
#define clocks (dp.size())
#endif

namespace VerifyTAPN {
	MarkingFactory* MPPMarking::factory = NULL;

#ifndef DBM_NORESIZE
	void MPPMarking::AddTokens(const std::list<int>& placeIndices) {
		int oldDimension = dp.size()+1;

		unsigned int i = 0;
		for(std::list<int>::const_iterator iter = placeIndices.begin(); iter != placeIndices.end(); ++iter)
		{
			for (MPVecIter v = V.begin(); v != V.end(); ++v)
				v->AddDim(0);
			for (MPVecIter w = W.begin(); w != W.end(); ++w)
				w->AddDim(NegInf);
			mapping.AddTokenToMapping(oldDimension+i);
			dp.AddTokenInPlace(*iter);
			i++;
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
		std::sort(removeClocks.begin(), removeClocks.end());
		for (std::vector<unsigned int>::reverse_iterator it = removeClocks.rbegin(); it != removeClocks.rend(); ++it) {
			for(unsigned int j = 0; j < mapping.size(); ++j)
			{
				if(mapping.GetMapping(j) > *it)
					mapping.SetMapping(j, *it-1);
			}
			for (MPVecIter v = V.begin(); v != V.end(); ++v)
				v->RemoveDim(*it);
			for (MPVecIter w = W.begin(); w != W.end(); ++w)
				w->RemoveDim(*it);
		}
	}
#endif

	void MPPMarking::Print() const {
		std::cout << "V: ";
		for (MPVecConstIter it = V.begin(); it != V.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
		std::cout << "W: ";
		for (MPVecConstIter it = W.begin(); it != W.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
		std::cout << "Placement vector:";
		std::vector<int> places = dp.GetTokenPlacementVector();
		for (std::vector<int>::iterator place = places.begin(); place != places.end(); ++place)
			std::cout << " " << *place;
		std::cout << "\n";
	}

	SymbolicMarking *MPPMarking::Clone() const {
		return factory->Clone(*this);
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
		MPVector g = MPVector(clocks, NegInf);
		g.Set(clock, 0);
		W.push_back(g);
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
		LOG(std::cout << "Extrapolate(...)\n");
		LOG(std::cout << "input:\n")
		LOG(Print());
		for (size_t i = FirstClock; i <= clocks; i++) {
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
				for (size_t j = FirstClock; j <= clocks; j++) {
					if (i != j) {
						for (MPVecIter u = U.begin(); u != U.end(); ++u) {
							if (u->Get(i) > k || u->Get(j) > maxConstants[j-1]) {
								MPVector ex = MPVector(clocks, NegInf);
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
				MPVector ex = MPVector(clocks, NegInf);
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

	void MPPMarking::Constrain(int token, const TAPN::TimeInterval &interval) {
		if (interval.IsLowerBoundStrict() || (interval.IsUpperBoundStrict() && interval.GetUpperBound() != 2147483647))
			std::cerr << "WARNING: Interval has strictness\n";
		LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
		LOG(std::cout << "input:\n")
		LOG(Print());
		PolyToCone();
		int clock = GetClockIndex(token);
		MPVector a = MPVector(clocks, NegInf);
		MPVector b = a;
		a.Set(clock, 0);
		if (interval.GetUpperBound() != 2147483647) {
			b.Set(ConeIdx, interval.GetUpperBound());
			IntersectHalfspace(a,b);
			LOG(std::cout << "After " << clock << " <= " << interval.GetUpperBound() << "\n";)
			LOG(Print();)
		}

		b.Set(ConeIdx, interval.GetLowerBound());
		IntersectHalfspace(b,a);

		ConeToPoly();
		LOG(std::cout << "After " << clock << " >= " << interval.GetLowerBound() << "\n";)
		LOG(Print();)
		Cleanup();
	}

	bool MPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval &interval) const {
		int clock = GetClockIndex(token);
		bool lowerSat = false, upperSat = false;
		for (MPVecConstIter it = V.begin(); it != V.end(); ++it) {
			lowerSat = lowerSat || it->Get(clock) > interval.GetLowerBound();
			upperSat = upperSat || it->Get(clock) < interval.GetUpperBound();

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
		V.push_back(MPVector(clocks));
		for (size_t i = dp.size(); i < clocks; ++i)
			FreeClock(i);
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		unsigned int retVal = mapping.GetMapping(token);
		return retVal;
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
			for (size_t j = 0; j <= clocks; j++) {
				if (it->Get(j) == NegInf)
					;
				else if (v.Get(j) == NegInf)
					y[i] = NegInf;
				else
					y[i] = MIN(y[i], v.Get(j) - it->Get(j));
			}
			++i;
		}

		MPVector z(clocks, NegInf);
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
		for(size_t i=FirstClock; i<clocks; ++i) {
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
