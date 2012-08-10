/*
 * VectorizedMPP.hpp
 *
 *  Created on: Jun 26, 2012
 *      Author: ravn
 */

#ifndef VECTORIZEDMPP_HPP_
#define VECTORIZEDMPP_HPP_

#include "../TAPN/TimeInterval.hpp"
#include "../TAPN/TimeInvariant.hpp"
#include "StoredMarking.hpp"
#include "SymbolicMarking.hpp"
#include <vector>
#include <cstring>

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

namespace VerifyTAPN {
	class VectorizedMPP {
	private:
		//data
		unsigned int n; /*number of tokens + zero clock */
		unsigned int gens; /* number of generators */
		std::vector<int> G; /* array of generators */
	public:
		//constructors
		VectorizedMPP() {
		}
		;
		VectorizedMPP(const std::vector<int> g, int generators, int N) :
				n(N), gens(generators), G(g) {
		}
		;
		VectorizedMPP(const VectorizedMPP &mpp) :
				n(mpp.n), gens(mpp.gens), G(mpp.G) {
		}
		;

		~VectorizedMPP() {
		}
		;

	private:
		//internal functions
		bool ContainsPoint(const std::vector<int>& x, int skipGen = -1) const;
		bool Contains(const VectorizedMPP& mpp) const;
		void IntersectHalfspace(std::vector<int>& a, std::vector<int>& b);
		void AddUnitVec(unsigned int dim);

		/*
		 * Extrapolation principle that adds a unit vector for each dimension that have a generator above the max diagonal.
		 * Sound, not complete.
		 */
		void Extrapolate49(const int* maxConstants);
		/*
		 * Principle that resets a dimension to max constant + 1 whenever the entire polyhedron is above the max constant.
		 * Sound, not complete.
		 */
		void Extrapolate411(const int* maxConstants);
		/*
		 * Principle that delays a single generator if it above the max constant for all dimensions.
		 * Sound, not complete.
		 */
		void Extrapolate413(const int* maxConstants);
		/*
		 * Principle that removes upper single clock bounds when above the max constant.
		 * Sound, not complete.
		 */
		void ExtrapolateNew(const int* maxConstants); //experiment

		/*
		 * Fucntion to normalize the generators of the polyhedron.
		 * Not currently used.
		 */
		void Norm();

	public:
		void InitZero(int numberOfTokens);
		int NumberOfGens() const { return gens; };
		int GetGIndex(int gen, int dim) const { return G.at(gen*n+dim); };
		//Reachability primitives
		void SwapClocks(int clock1, int clock2);
		void Print(std::ostream& out) const;
		bool IsEmpty() const;
		void Delay();
		void Constrain(int clock, const TAPN::TimeInterval& interval);
		void Constrain(int clock, const TAPN::TimeInvariant& invariant);
		bool PotentiallySatisfies(int clock, const TAPN::TimeInterval& interval) const;
		/*
		 * Sound, not complete.
		 */
		void Extrapolate(const int* maxConstants);
		void ConvexHullUnion(VectorizedMPP* mpp);
		relation Relation(const VectorizedMPP& other) const;
		void AddClock();
		void RemoveClocks(const std::vector<unsigned int> removeClocks);
		int GetUpperDiffBound(int clock1, int clock2) const;
		void ResetClock(int clock, int resetVal = 0);
		void FreeClock(int clock, int resetVal = 0);
		void Cleanup();
	};
}

#endif /* VECTORIZEDMPP_HPP_ */
