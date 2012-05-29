/*
 * TPlibMPPMarkingFactory.hpp
 *
 *  Created on: May 9, 2012
 *      Author: ravn
 */

#ifndef TPLIBMPPMARKINGFACTORY_HPP_
#define TPLIBMPPMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "TPlibMPPMarking.hpp"
#include "../VerificationOptions.hpp"

namespace VerifyTAPN{

	class SymbolicMarking;
	class StoredMarking;

	class TPlibMPPMarkingFactory: public MarkingFactory {
	private:
		static id_type nextId;

	public:
		TPlibMPPMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn){
			init();
			TPlibMPPMarking::tapn = tapn;
		};

		virtual ~TPlibMPPMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const {
			TPlibMPPMarking *marking = new TPlibMPPMarking(DiscretePart(tokenPlacement));
			marking->InitZero();
			marking->id = 0;
			return marking;
		};

		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const {
			TPlibMPPMarking *clone = new TPlibMPPMarking(static_cast<const TPlibMPPMarking&>(marking));
			clone->id = nextId++;
			return clone;
		};

		virtual StoredMarking* Convert(SymbolicMarking* marking) const {
			return static_cast<TPlibMPPMarking*>(marking);
		};

		virtual SymbolicMarking* Convert(StoredMarking* marking) const {
			return static_cast<TPlibMPPMarking*>(marking);
		};

		virtual void Release(SymbolicMarking* marking) {};

		virtual void Release(StoredMarking* marking) {};


	};

	id_type TPlibMPPMarkingFactory::nextId = 1;
}
#endif /* TPLIBMPPMARKINGFACTORY_HPP_ */
