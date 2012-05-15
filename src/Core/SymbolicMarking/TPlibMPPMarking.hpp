/*
 * TPlibMPPMarking.hpp
 *
 *  Created on: May 9, 2012
 *      Author: ravn
 */

#ifndef TPLIBMPPMARKING_HPP_
#define TPLIBMPPMARKING_HPP_

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"

#include "tplib_double.h"

namespace VerifyTAPN {
	class TPlibMPPMarking: public DiscreteMarking, public StoredMarking {
		friend class DiscreteInclusionMarkingFactory;
		friend class TPlibMPPMarkingFactory;
	public:
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;
	private:
		//data
		TokenMapping mapping;
		id_type id;
		poly_t *poly; /* struct containing ocaml chunk representing the polyhedron */

	public:
		//constructors
		TPlibMPPMarking(const DiscretePart) :
				DiscreteMarking(dp), mapping() {
			InitMapping();
		}
		;
		TPlibMPPMarking(const DiscretePart &dp, const TokenMapping &mapping, poly_t &poly) :
				DiscreteMarking(dp), mapping(mapping), poly(&poly) {
		}
		;
		TPlibMPPMarking(const TPlibMPPMarking &mpp) :
				DiscreteMarking(mpp), mapping(mpp.mapping), poly(mpp.poly) {
		}
		;
		virtual ~TPlibMPPMarking() {
			poly_free(poly);
		};

	private:
		//internal functions
		void InitZero();
		void InitMapping();

	protected:
		virtual void Swap(int i, int j);
		//virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;

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
		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval);
		virtual void Extrapolate(const int* maxConstants);
		virtual size_t HashKey() const;
		virtual relation Relation(const StoredMarking& other) const;
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::set<int>& tokenIndices);



	};
}

#endif /* TPLIBMPPMARKING_HPP_ */
