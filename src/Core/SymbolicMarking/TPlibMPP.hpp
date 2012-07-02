/*
 * TPlibMPP.hpp
 *
 *  Created on: Jun 29, 2012
 *      Author: ravn
 */

#ifndef TPLIBMPP_HPP_
#define TPLIBMPP_HPP_

#include "SymbolicMarking.hpp"
#include "StoredMarking.hpp"
#include <tplib_double.h>
#include <climits>
#include <math.h>
#include <limits>
#include <cstdio>


namespace VerifyTAPN {
	class TPlibMPP {
	private:
		//data
		poly_t *poly; /* struct containing ocaml chunk representing the polyhedron */
	public:
		//constructors
		TPlibMPP() {
			poly = NULL;
		}
		;
		TPlibMPP(const TPlibMPP &mpp) {
			poly = copy(mpp.poly);
		}
		;
		~TPlibMPP() {
			poly_free(poly);
		}
		;
	public:
		void InitZero(int numberOfTokens);
		void Print(std::ostream& out) const;
		void SwapClocks(int clock1, int clock2);
		void ResetClock(int clock);
		bool IsEmpty() const;
		void Delay();
		void FreeClock(int clock);
		void Constrain(int clock, const TAPN::TimeInterval& interval);
		void Constrain(int clock, const TAPN::TimeInvariant& invariant);
		bool PotentiallySatisfies(int clock, const TAPN::TimeInterval& interval) const;
		void Extrapolate(const int* maxConstants);
		relation Relation(const TPlibMPP& other) const;
		void AddClocks(int* clockArray, int* newDimArray, int numberOfClocks);
		void RemoveClocks(int* clockArray, int numberOfClocks);
		void ConvexHullUnion(TPlibMPP* other);
		poly_t* GetPoly(){return poly;};
		double GetLowerDiffBound(int clock1, int clock2) const;
	private:
		void ConstrainClock(int clock, int ub, int lb);
	};
}

#endif /* TPLIBMPP_HPP_ */
