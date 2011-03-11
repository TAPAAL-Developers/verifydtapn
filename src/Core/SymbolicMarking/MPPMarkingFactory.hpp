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
		MPPMarkingFactory(int clocks) : clocks(clocks) { };
		virtual ~MPPMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const DiscretePart& dp) const;
		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const;
		virtual StoredMarking* Convert(SymbolicMarking* marking) const;
		virtual SymbolicMarking* Convert(StoredMarking* marking) const;
	};
}

#endif /* MPPMARKINGFACTORY_HPP_ */
