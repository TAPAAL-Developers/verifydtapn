#ifndef VECTORIZEDMPPMARKINGFACTORY_HPP_
#define VECTORIZEDMPPMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "VectorizedMPPMarking.hpp"
#include "../VerificationOptions.hpp"

namespace VerifyTAPN{

	class SymbolicMarking;
	class StoredMarking;

	class VectorizedMPPMarkingFactory: public MarkingFactory {
	private:
		static id_type nextId;
		unsigned int n; /* maximal number of clocks + "cone reference clock" */

	public:
		VectorizedMPPMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn, const unsigned int kBound){
			VectorizedMPPMarking::tapn = tapn;
			n = kBound+1; // maximal allowed number of clocks + cone reference (zero) clock
		};

		virtual ~VectorizedMPPMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const {
			VectorizedMPPMarking *marking = new VectorizedMPPMarking(DiscretePart(tokenPlacement));
			marking->InitZero(n);
			marking->id = 0;
			return marking;
		};

		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const {
			VectorizedMPPMarking *clone = new VectorizedMPPMarking(static_cast<const VectorizedMPPMarking&>(marking));
			clone->id = nextId++;
			return clone;
		};

		virtual StoredMarking* Convert(SymbolicMarking* marking) const {
		//	VectorizedMPPMarking *mpp = static_cast<VectorizedMPPMarking*>(marking);
		//	mpp->Cleanup();
		//	return mpp;
			return static_cast<VectorizedMPPMarking*>(marking);
		};

		virtual SymbolicMarking* Convert(StoredMarking* marking) const {
			return static_cast<VectorizedMPPMarking*>(marking);
		};

		virtual void Release(SymbolicMarking* marking) {};

		virtual void Release(StoredMarking* marking) {};


	};

	id_type VectorizedMPPMarkingFactory::nextId = 1;
}


#endif /* VECTORIZEDMPPMARKINGFACTORY_HPP_ */
