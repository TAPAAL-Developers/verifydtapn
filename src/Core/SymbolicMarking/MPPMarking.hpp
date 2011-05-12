#ifndef MPPMARKING_HPP_
#define MPPMARKING_HPP_

#include <list>
#include <iostream>
#include <algorithm>
#include <set>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"

#include "MPVector.hpp"

namespace VerifyTAPN {

	template <typename T>
	class MPPMarkingFactory;

	template <typename MPVec>
	class MPPMarking: public DiscreteMarking, public StoredMarking {
	public:
		typedef std::list<MPVec> MPVecSet;
		typedef typename MPVecSet::iterator MPVecIter;
		typedef typename MPVecSet::const_iterator MPVecConstIter;
		friend class MPPMarkingFactory<MPVec>;
		friend class DBMMarking;
	private:
		MPVecSet V, W;
		id_type id;

		void InitZero() {
			V.clear();
			W.clear();
			V.push_back(MPVec(dp.size()));
		}
		void InitMapping() {
			for(unsigned int i = 0; i < dp.size(); i++) {
				mapping.SetMapping(i, i+1);
			}
		}

		void PolyToCone() {
			for (MPVecIter it = V.begin(); it != V.end(); ++it) {
				it->Set(ConeIdx, 0);
			}
			W.splice(W.end(), V);
			isCone = true;
		}
		void ConeToPoly() {
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
		bool isCone;

		bool Contains(const MPPMarking<MPVec>& mpp) const {
			MPPMarking<MPVec> G = MPPMarking<MPVec>(*this);
			MPPMarking<MPVec> Gprime = MPPMarking<MPVec>(mpp);

			G.PolyToCone();
			Gprime.PolyToCone();

			for (MPVecIter it = Gprime.W.begin(); it != Gprime.W.end(); ++it)
				if (!G.ContainsPoint(*it))
					return false;

			return true;
		}
		bool ContainsPoint(const MPVec& v, MPVecIter* skipit = NULL) const {
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

			MPVec z(dp.size(), NegInf);
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

		void IntersectHalfspace(const MPVec &a, const MPVec &b) {
			MPVecSet Gleq, Ggt;
			for (MPVecIter it = W.begin(); it != W.end(); ++it) {
				MPVec g = *it;
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
					MPVec p1 = a+(*h)+(*g);
					MPVec p2 = b+(*g)+(*h);
					MPVec p = Max(p1, p2);
					W.push_back(p);
				}
			}
		}
		void Cleanup() {
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
		bool DiagonalFree(MPVecSet L, MPVecSet H, size_t idx) {
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

		void ResetClock(int clock) {
			for (MPVecIter it = V.begin(); it != V.end(); ++it) {
				it->Set(clock, 0);
			}
			for (MPVecIter it = W.begin(); it != W.end(); ++it) {
				it->Set(clock, NegInf);
			}
		}
		void FreeClock(int clock) {
			ResetClock(clock);
			MPVec g = MPVec(dp.size(), NegInf);
			g.Set(clock, 0);
			W.push_back(g);
		}

		void doConstrain(int x, int y, int value) {
			MPVec a = MPVec(dp.size(), NegInf);
			MPVec b = a;
			a.Set(x, 0);
			if (value != INF) {
				b.Set(y, value);
				/*std::cout << "a: " << a << "\n";
			std::cout << "b: " << b << "\n";*/
				IntersectHalfspace(a,b);
			}
		}

		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;
	public:
		MPPMarking(const DiscretePart &dp) : DiscreteMarking(dp), isCone(false) { InitMapping(); };
		MPPMarking(const DiscretePart &dp, MPVecSet v, MPVecSet w) : DiscreteMarking(dp),  V(v), W(w), isCone(false) { InitMapping(); };
		MPPMarking(const MPPMarking &mpp) : DiscreteMarking(mpp), V(mpp.V), W(mpp.W), isCone(false) { };
		virtual ~MPPMarking() { };
		virtual void Print() const {
			std::cout << "V: ";
			std::set<MPVec> sortedV, sortedW;
			sortedV.insert(V.begin(), V.end());
			for (MPVecConstIter it = W.begin(); it != W.end(); ++it)
				sortedW.insert(it->Normalize());
			for (typename std::set<MPVec>::iterator it = sortedV.begin(); it != sortedV.end(); ++it)
				std::cout << *it;
			std::cout << "\n";
			std::cout << "W: ";
			for (typename std::set<MPVec>::iterator it = sortedW.begin(); it != sortedW.end(); ++it)
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

		virtual id_type UniqueId() const {
			return id;
		}
		virtual unsigned int GetClockIndex(unsigned int token) const {
			return mapping.GetMapping(token);
		}

		virtual void Reset(int token) {
			LOG(std::cout << "Reset("<<token<<")\n")
				LOG(std::cout << "input:\n")
				LOG(Print());
			ResetClock(GetClockIndex(token));
			Cleanup();
			LOG(std::cout << "output:\n")
			LOG(Print());
		}
		virtual bool IsEmpty() const { 
			return V.empty(); 
		}

		virtual void Delay() {
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
		virtual void Free(int token) {
			FreeClock(mapping.GetMapping(token));
			Cleanup();
		}

		virtual void Constrain(int token, const TAPN::TimeInterval& interval) {
			//		if (interval.IsLowerBoundStrict() || (interval.IsUpperBoundStrict() && interval.GetUpperBound() != 2147483647))
			//			std::cerr << "WARNING: Interval has strictness\n";
			LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			PolyToCone();
			int clock = GetClockIndex(token);
			MPVec a = MPVec(dp.size(), NegInf);
			MPVec b = a;
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

		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant) {
			if(invariant.GetBound() != std::numeric_limits<int>::max())
			{
				PolyToCone();
				doConstrain(mapping.GetMapping(token), 0, invariant.GetBound());
				ConeToPoly();
				Cleanup();
				//Constrain(token, TAPN::TimeInterval(false, 0, invariant.GetBound(), invariant.IsBoundStrict()));
			}
		};

		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const {
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

		virtual void Extrapolate(const int* maxConstants) {
			return;
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
									MPVec ex = MPVec(dp.size(), NegInf);
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
					MPVec ex = MPVec(dp.size(), NegInf);
					ex.Set(i, 0);
					W.push_back(ex);
				}
			}
			LOG(std::cout << "output:\n")
			LOG(Print());
		}
		virtual void ConvexUnion(AbstractMarking* marking) {
			MPPMarking<MPVec>* m = static_cast<MPPMarking<MPVec>*>(marking);
			V.insert(V.end(), m->V.begin(), m->V.end());
			W.insert(W.end(), m->W.begin(), m->W.end());
		}

		virtual size_t HashKey() const {
			return VerifyTAPN::hash()(dp);
		}
		virtual relation Relation(const StoredMarking& other) const {
			const MPPMarking<MPVec> &mpp = static_cast<const MPPMarking<MPVec>&> (other);

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

		virtual void AddTokens(const std::list<int>& placeIndices) {
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

		virtual void RemoveTokens(const std::vector<int>& tokenIndices) {
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

	};

	template <typename MPVec>
	boost::shared_ptr<TAPN::TimedArcPetriNet> MPPMarking<MPVec>::tapn;

}
#endif /* MPPMARKING_HPP_ */
