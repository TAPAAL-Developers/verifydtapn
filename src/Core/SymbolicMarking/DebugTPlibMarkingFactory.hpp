/*
 * DebugTPlibMarkingFactory.hpp
 *
 *  Created on: May 29, 2012
 *      Author: ravn
 */

#ifndef DEBUGTPLIBMARKINGFACTORY_HPP_
#define DEBUGTPLIBMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
//#include "VectorizedMPPMarking.hpp"
//#include "TPlibMPPMarking.hpp"
#include "../VerificationOptions.hpp"
#include "DebugTPlibMarking.hpp"

namespace VerifyTAPN {
	class SymbolicMarking;
	class StoredMarking;
	//class DebugTPlibMarking;

	class DebugTPlibMarkingFactory: public MarkingFactory {
//		friend class DebugTPlibMarking;
//		friend class VectorizedMPPMarking;
//		friend class TPlibMPPMarking;
	private:
		static id_type nextId;
	public:
		DebugTPlibMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn, Cleanup cleanup) {
			init();
			set_weakbasis_type(cleanup);
			DebugTPlibMarking::tapn = tapn;
		}
		;
		virtual ~DebugTPlibMarkingFactory() {
		}
		;

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const {
			DebugTPlibMarking *marking = new DebugTPlibMarking(DiscretePart(tokenPlacement));
			marking->InitZero(tokenPlacement);
			marking->id = 0;
			return marking;
		}
		;

		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const {
			DebugTPlibMarking *clone = new DebugTPlibMarking(static_cast<const DebugTPlibMarking&>(marking));
			clone->id = nextId++;
			return clone;
		}
		;

		virtual StoredMarking* Convert(SymbolicMarking* marking) const {
			return static_cast<DebugTPlibMarking*>(marking);
		}
		;

		virtual SymbolicMarking* Convert(StoredMarking* marking) const {
			return static_cast<DebugTPlibMarking*>(marking);
		}
		;

		virtual void Release(SymbolicMarking* marking) {
		}
		;
		virtual void Release(StoredMarking* marking) {
		}
		;
	};

	id_type DebugTPlibMarkingFactory::nextId = 1;


}
#endif /* DEBUGTPLIBMARKINGFACTORY_HPP_ */
