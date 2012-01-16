#ifndef VECTORIZEDMPPMARKING_HPP
#define VECTORIZEDMPPMARKING_HPP

#include <iostream>
#include <algorithm>
#include <vector>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"
//#include "MPVector.hpp"		//using InvalidDimException defined here

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

const int initGens = 50; //number of initial generators for which to allocate memory

namespace VerifyTAPN {


	class VectorizedMPPMarking: public DiscreteMarking, public StoredMarking {
		friend class DiscreteInclusionMarkingFactory;
		friend class VectorizedMPPMarkingFactory;
	public:
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;

	private: // data
		TokenMapping mapping;
		std::vector<int> G; /* array of generators */
		std::vector<int> AC; /* active clocks */
		unsigned int n; /*number of maximal clocks + zero clock */
		unsigned int gens; /* number of generators */
		id_type id;


	public: //constructors
		VectorizedMPPMarking(const DiscretePart &dp) : DiscreteMarking(dp), mapping() { InitMapping(); };
		VectorizedMPPMarking(const DiscretePart &dp, const TokenMapping& mapping, std::vector<int> g, std::vector<int> ac, int generators) : DiscreteMarking(dp), mapping(mapping), G(g), AC(ac), n(ac.size()), gens(generators) {};
		VectorizedMPPMarking(const VectorizedMPPMarking &mpp) : DiscreteMarking(mpp), mapping(mpp.mapping), G(mpp.G), AC(mpp.AC), n(mpp.n), gens(mpp.gens) {};
		virtual ~VectorizedMPPMarking() { };


	private:  //internal functions
		void InitZero(const unsigned int n);
		void InitMapping();

		bool ContainsPoint(const std::vector<int>& x, int skipGen = -1) const;
		bool Contains(const VectorizedMPPMarking& mpp) const;
		void IntersectHalfspace(std::vector<int>& a, std::vector<int>& b);
		void Cleanup();

		void ResetClock(int clock, int resetVal = 0);
		void FreeClock(int clock, int resetVal = 0);

		void PrintLocal() const;

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
		virtual void AddTokens(const std::list<int>& placeIndicies);
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
