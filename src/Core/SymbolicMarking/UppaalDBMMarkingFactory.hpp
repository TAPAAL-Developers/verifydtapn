#ifndef UPPAALDBMMARKINGFACTORY_HPP_
#define UPPAALDBMMARKINGFACTORY_HPP_

#include "MarkingFactory.hpp"
#include "DBMMarking.hpp"

namespace VerifyTAPN {

	class UppaalDBMMarkingFactory : public MarkingFactory {
	protected:
		static id_type nextId;
#ifdef DBM_NORESIZE
		int clocks;
	public:
		UppaalDBMMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn, int clocks) : clocks(clocks)
		{
			DBMMarking::tapn = tapn;
		};

#else
	public:
		UppaalDBMMarkingFactory(const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn)
		{
			DBMMarking::tapn = tapn;
		};
#endif
		virtual ~UppaalDBMMarkingFactory() {};

		virtual SymbolicMarking* InitialMarking(const std::vector<int>& tokenPlacement) const
		{
#ifdef DBM_NORESIZE
			dbm::dbm_t dbm(clocks+1);
#else
			dbm::dbm_t dbm(tokenPlacement.size()+1);
#endif
			dbm.setZero();
#ifdef DBM_NORESIZE
			for (int i = tokenPlacement.size(); i < clocks; ++i)
						dbm.freeClock(i+1);
#endif
			DBMMarking* marking = new DBMMarking(DiscretePart(tokenPlacement), dbm);
			marking->id = 0;

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

		virtual void Release(SymbolicMarking* marking)
		{

		};

		virtual void Release(StoredMarking* marking)
		{

		};
	};

}

#endif /* UPPAALDBMMARKINGFACTORY_HPP_ */
