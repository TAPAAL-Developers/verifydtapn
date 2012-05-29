#ifndef VECTORIZEDMPPMARKING_HPP
#define VECTORIZEDMPPMARKING_HPP

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef DEBUG_PRINT_MPV
#define DEBUG_PRINT_MPV false
#endif

namespace VerifyTAPN {

	class VectorizedMPPMarking: public DiscreteMarking, public StoredMarking {
		friend class DiscreteInclusionMarkingFactory;
		friend class VectorizedMPPMarkingFactory;
	public:
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;

	private:
		// data
		TokenMapping mapping;
		std::vector<int> G; /* array of generators */
		unsigned int n; /*number of tokens + zero clock */
		unsigned int gens; /* number of generators */
		id_type id;

	public:
		//constructors
		VectorizedMPPMarking(const DiscretePart &dp) :
				DiscreteMarking(dp), mapping() {
			InitMapping();
		}
		;
		VectorizedMPPMarking(const DiscretePart &dp, const TokenMapping& mapping, std::vector<int> g, int generators) :
				DiscreteMarking(dp), mapping(mapping), G(g), n(dp.size()+1), gens(generators) {
		}
		;
		VectorizedMPPMarking(const VectorizedMPPMarking &mpp) :
				DiscreteMarking(mpp), mapping(mpp.mapping), G(mpp.G), n(mpp.n), gens(mpp.gens) {
		}
		;
		virtual ~VectorizedMPPMarking() {
		}
		;

	private:
		//internal functions
		void InitZero();
		void InitMapping();

		bool ContainsPoint(const std::vector<int>& x, int skipGen = -1) const;
		bool Contains(const VectorizedMPPMarking& mpp) const;
		void IntersectHalfspace(std::vector<int>& a, std::vector<int>& b);
		void Cleanup();
		void CleanupOS();
		void Norm();
		int Lexmin(const std::vector<int>& C, unsigned int gens, unsigned int dim = 0) const;
		bool ExSetContainsPoint(const std::vector<int>& C, int Cgens, const std::vector<int>& x) const;
		std::vector<int> ArgmaxPsi(const std::vector<int>& P, int Pgens, int dim, const std::vector<int> w,
				int skipgent = -1) const;

		void ResetClock(int clock, int resetVal = 0);
		void FreeClock(int clock, int resetVal = 0);

		void PrintMarking() const;

		void Extrapolate49(const int* maxConstants);
		void Extrapolate411(const int* maxConstants);
		void Extrapolate413(const int* maxConstants);
		void ExtrapolateClaim(const int* maxConstants);

		void AddUnitVec(unsigned int dim);

	protected:
		virtual void Swap(int i, int j);
		//	virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;

	public:
		virtual void Print(std::ostream& out) const;
		virtual id_type UniqueId() const;
		virtual unsigned int GetClockIndex(unsigned int token) const;

		virtual void Reset(int token);
		virtual bool IsEmpty() const;
		virtual void Delay();
		virtual void Free(int token);
		virtual void Constrain(int token, const TAPN::TimeInterval& interval);
		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant);
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const;
		virtual void Extrapolate(const int* maxConstants);
		void ConvexUnion(AbstractMarking* marking);
		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::set<int>& tokenIndices);

		/*
		 virtual void MakeSymmetric(BiMap& inderictionTable);
		 virtual void MoveToken(int tokenIndex, int newPlaceIndex);
		 virtual int GetTokenPlacement(int token) const;
		 virtual unsigned int NumberOfTokens() const;
		 virtual unsigned int NumberOfTokensInPlace(int placeIndex) const;*/

	};
}

#endif /* VECTORIZEDMPPMARKING_HPP */
