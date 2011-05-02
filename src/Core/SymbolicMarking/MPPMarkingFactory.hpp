#ifndef MPPMARKINGFACTORY_HPP_
#define MPPMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "MPPMarking.hpp"

namespace VerifyTAPN {
	class MPPMarkingFactory: public MarkingFactory {
	private:
		static id_type nextId;
		int clocks;
	public:
		MPPMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn);
		virtual ~MPPMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const;
		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const;
		virtual StoredMarking* Convert(SymbolicMarking* marking) const;
		virtual SymbolicMarking* Convert(StoredMarking* marking) const;

		virtual void Release(SymbolicMarking* marking) {};
		virtual void Release(StoredMarking* marking) {};
	};
}

#endif /* MPPMARKINGFACTORY_HPP_ */
