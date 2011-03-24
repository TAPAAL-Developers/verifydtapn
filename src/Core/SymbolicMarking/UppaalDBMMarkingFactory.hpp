#ifndef UPPAALDBMMARKINGFACTORY_HPP_
#define UPPAALDBMMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "DBMMarking.hpp"

namespace VerifyTAPN {

	class UppaalDBMMarkingFactory : public MarkingFactory {
	private:
		static id_type nextId;
#ifdef DBM_NORESIZE
		int clocks;
	public:
		UppaalDBMMarkingFactory(int clocks) : clocks(clocks) { };

#else
	public:
		UppaalDBMMarkingFactory() { };
#endif
		virtual ~UppaalDBMMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const DiscretePart& dp) const
		{
#ifdef DBM_NORESIZE
			dbm::dbm_t dbm(clocks+1);
#else
			dbm::dbm_t dbm(dp.size()+1);
#endif
			dbm.setZero();
#ifdef DBM_NORESIZE
			for (int i = dp.size(); i < clocks; ++i)
						dbm.freeClock(i+1);
#endif
			DBMMarking* marking = new DBMMarking(dp, dbm);
			marking->id = nextId++;
			return marking;
		};

		virtual SymbolicMarking* Clone(const SymbolicMarking& marking) const
		{
			DBMMarking* clone = new DBMMarking(static_cast<const DBMMarking&>(marking));
			clone->id = nextId++;
			return clone;
		};
		virtual StoredMarking* Convert(SymbolicMarking* marking) const { return static_cast<DBMMarking*>(marking); };
		virtual SymbolicMarking* Convert(StoredMarking* marking) const { return static_cast<DBMMarking*>(marking); };
	};

}

#endif /* UPPAALDBMMARKINGFACTORY_HPP_ */
