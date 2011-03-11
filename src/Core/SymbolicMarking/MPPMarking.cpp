#include "MPPMarking.hpp"
#include <iostream>

#define DEBUG

#ifdef DEBUG
#define LOG(x) x;
#else
#define LOG(x)
#endif

namespace VerifyTAPN {
	MarkingFactory* MPPMarking::factory = NULL;

	void MPPMarking::Print() const {
		std::cout << "V: ";
		for (MPVecIter it = V.begin(); it != V.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
		std::cout << "W: ";
		for (MPVecIter it = W.begin(); it != W.end(); ++it)
			std::cout << *it;
		std::cout << "\n";
	}

	SymbolicMarking *MPPMarking::Clone() const {
		return factory->Clone(*this);
	}

	void MPPMarking::Reset(int token) {
		LOG(std::cout << "Reset(" << token << ")\n");
		// TODO If possible, find a way to avoid replacing the entire sets
		int clock = GetClockIndex(token);
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
		LOG(Print();)
		Cleanup();
	}

	void MPPMarking::Delay() {
		LOG(std::cout << "Delay()\n");
		W.insert(V.begin(), V.end());
		LOG(Print();)
		Cleanup();
	}

	void MPPMarking::Extrapolate(const int *maxConstants) {
		LOG(std::cout << "Extrapolate(...)\n");
		for (size_t i = 1; i <= clocks; i++) {
			int k = maxConstants[i-1];
			if (k == -INF) {
				//FIXME Reset algorithm must be repeated here because we have a clock, not a token
				//Refactor so we have internal methods working on clocks, which are just called to when we have a token
				MPVecSet newV, newW;
				for (MPVecIter it = V.begin(); it != V.end(); ++it) {
					MPVector v = *it;
					v.Set(i, 0);
					newV.insert(v);
				}
				for (MPVecIter it = W.begin(); it != W.end(); ++it) {
					MPVector w = *it;
					w.Set(i, NegInf);
					newW.insert(w);
				}
				V = newV;
				W = newW;
				MPVector g = MPVector(clocks, NegInf);
				g.Set(i, 0);
				W.insert(g);
				continue;
			}
			MPVecSet T, U;
			for (MPVecIter v = V.begin(); v != V.end(); ++v) {
				if (v->Get(i) <= k)
					T.insert(*v);
				else
					U.insert(*v);
			}
			if (U.empty())
				continue;
			bool hori = true;
			if (DiagonalFree(T,U,i)) {
				MPVecSet newV;
				for (MPVecIter it = V.begin(); it != V.end(); ++it) {
					MPVector v = *it;
					if (v.Get(i) > k)
						v.Set(i, k+1);
					newV.insert(v);
				}
				V = newV;
			} else {
				for (size_t j = 1; j <= clocks; j++) {
					if (i != j) {
						for (MPVecIter u = U.begin(); u != U.end(); ++u) {
							if (u->Get(i) > k || u->Get(j) > maxConstants[j-1]) {
								MPVector ex = MPVector(clocks, NegInf);
								ex.Set(i, u->Get(i));
								ex.Set(j, u->Get(j));
								W.insert(ex);
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
				W.insert(ex);
			}
		}
		LOG(Print();)
	}

	bool MPPMarking::IsEmpty() const {
		return V.empty();
	}

	void MPPMarking::Constrain(int token, const TAPN::TimeInterval &interval) {
		LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
		PolyToCone();
		int clock = GetClockIndex(token);
		// TODO Check if this is the right size
		MPVector a = MPVector(clocks, NegInf);
		MPVector b = a;
		a.Set(clock, 0);
		b.Set(ConeIdx, interval.GetUpperBound());
		IntersectHalfspace(a,b);
		LOG(std::cout << "After " << clock << " <= " << interval.GetUpperBound() << "\n";)
		LOG(Print();)

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
		LOG(std::cout << "Relation()\nthis:\n";)
		LOG(Print();)
		LOG(std::cout << "mpp:\n";)
		LOG(mpp.Print();)
		bool sup = Contains(mpp);
		bool sub = mpp.Contains(*this);

		if (sub && sup) {
			LOG(std::cout << "Equal\n")
			return EQUAL;
		}
		if (sub) {
			LOG(std::cout << "Subset\n")
			return SUBSET;
		}
		if (sup) {
			LOG(std::cout << "Superset\n")
			return SUPERSET;
		}

		LOG(std::cout << "Different\n")
		return DIFFERENT;
	}

	void MPPMarking::InitZero() {
		LOG(std::cout << "InitZero()\n");
		V.clear();
		W.clear();
		// TODO Check if this is the right size
		V.insert(MPVector(clocks));
		LOG(Print();)
	}

	id_type MPPMarking::UniqueId() const {
		return id;
	}

	unsigned int MPPMarking::GetClockIndex(unsigned int token) const {
		unsigned int retVal = mapping.GetMapping(token);
		LOG(std::cout << "Mapping token " << token << " to index " << retVal << "\n");
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
		int count = W.size();
		int* y = new int[count];
		for (int i = 0; i < count; i++)
			y[i] = 0;

		int i = 0;
		for (MPVecIter it = W.begin(); it != W.end(); ++it, i++) {
			for (size_t j = 1; j <= clocks; j++) {
				y[i] = min(y[i], v.Get(j) - it->Get(j));
			}
		}

		MPVector z(clocks, NegInf);
		i = 0;
		for (MPVecIter it = W.begin(); it != W.end(); ++it, i++)
			z = Max(z, y[i] + (*it));
		delete[] y;
		return v == z;
	}

	void MPPMarking::IntersectHalfspace(const MPVector &a, const MPVector &b) {
		MPVecSet Gleq, Ggt;
		LOG(std::cout << "a = " << a << ", b = " << b << "\n")
		for (MPVecIter it = W.begin(); it != W.end(); ++it) {
			MPVector g = *it;
			LOG(std::cout << "a+g = " << a+g << ", b+g = " << b+g << "\n";)
			if (a+g <= b+g) {
				Gleq.insert(g);
				LOG(std::cout << "Adding " << g << " to G<=\n";)
			}
			else {
				Ggt.insert(g);
				LOG(std::cout << "Adding " << g << " to G>\n";)
			}
		}
		W = Gleq;
		for (MPVecIter g = Gleq.begin(); g != Gleq.end(); ++g) {
			for (MPVecIter h = Ggt.begin(); h != Ggt.end(); ++h) {
				MPVector p1 = a+(*h)+(*g);
				MPVector p2 = b+(*g)+(*h);
				MPVector p = Max(p1, p2);
				LOG(std::cout << "Adding " << p << " to W\n";)
				W.insert(p);
			}
		}
	}

	void MPPMarking::Cleanup() {
		LOG(std::cout << "Cleanup()\n");
		PolyToCone();
		MPPMarking copy(*this);

		for(MPVecIter it = W.begin(); it!=W.end(); ++it) {
			copy.W.erase(*it);
			if(!copy.ContainsPoint(*it)) {
				copy.W.insert(*it);
			}
		}

		W=copy.W;
		ConeToPoly();
		LOG(Print();)
	}

	bool MPPMarking::DiagonalFree(MPVecSet L, MPVecSet H, size_t idx) {
		for(size_t i=1; i<clocks; ++i) {
			if(i!=idx) {
				int minL = INT_MAX;
				int minH = INT_MAX;
				int maxL = NegInf;
				int maxH = NegInf;
				for(MPVecIter it=L.begin(); it!=L.end(); ++it) {
					minL = min(minL, it->Get(i));
					maxL = max(maxL, it->Get(i));
				}
				for(MPVecIter it=H.begin(); it!=H.end(); ++it) {
					minH = min(minH, it->Get(i));
					maxH = max(maxH, it->Get(i));
				}
				if(minL<minH || maxL<maxH) {
					return false;
				}
			}
		}
		return true;
	}
}
