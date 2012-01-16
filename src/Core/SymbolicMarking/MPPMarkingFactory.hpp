#ifndef MPPMARKINGFACTORY_HPP_
#define MPPMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "MPPMarking.hpp"

namespace VerifyTAPN {
	template <typename MPVec>
	class MPPMarkingFactory: public MarkingFactory {
	private:
		static id_type nextId;
		int clocks;
	public:
		MPPMarkingFactory<MPVec>(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn) {
			MPPMarking<MPVec>::tapn = tapn;
		};
		virtual ~MPPMarkingFactory<MPVec>() {};

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const {
			MPPMarking<MPVec> *marking = new MPPMarking<MPVec>(DiscretePart(tokenPlacement));
			marking->InitZero();
			marking->id = 0;
			return marking;
		}

		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const {
			MPPMarking<MPVec> *clone = new MPPMarking<MPVec>(static_cast<const MPPMarking<MPVec>&>(marking));
			clone->id = nextId++;
			return clone;
		}

		virtual StoredMarking* Convert(SymbolicMarking* marking) const {
			return static_cast<MPPMarking<MPVec>*>(marking);
		}

		virtual SymbolicMarking* Convert(StoredMarking* marking) const {
			return static_cast<MPPMarking<MPVec>*>(marking);
		}

		virtual void Release(SymbolicMarking* marking) {};
		virtual void Release(StoredMarking* marking) {};
	};

	template <typename MPVec>
	id_type MPPMarkingFactory<MPVec>::nextId = 1;
}

#endif /* MPPMARKINGFACTORY_HPP_ */
