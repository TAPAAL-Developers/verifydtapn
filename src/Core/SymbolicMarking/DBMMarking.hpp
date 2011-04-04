#ifndef DBMMARKING_HPP_
#define DBMMARKING_HPP_

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "dbm/fed.h"
#include "dbm/print.h"
#include "MarkingFactory.hpp"
#include <iostream>

namespace VerifyTAPN {

	class DBMMarking: public DiscreteMarking, public StoredMarking {
		friend class UppaalDBMMarkingFactory;
	public:
		static MarkingFactory* factory;
#ifdef DBM_NORESIZE
	private:
		size_t clocks;
		DBMMarking(const DiscretePart& dp, const dbm::dbm_t& dbm) : DiscreteMarking(dp), clocks(dbm.getDimension()-1), dbm(dbm) { InitMapping(); };
		DBMMarking(const DBMMarking& dm) : DiscreteMarking(dm), clocks(dm.clocks), dbm(dm.dbm) { };
#else
		DBMMarking(const DiscretePart& dp, const dbm::dbm_t& dbm) : DiscreteMarking(dp), dbm(dbm) { InitMapping(); };
		DBMMarking(const DBMMarking& dm) : DiscreteMarking(dm), dbm(dm.dbm) { };
#endif
		virtual ~DBMMarking() { };

		virtual SymbolicMarking* Clone() const { return factory->Clone(*this); }; // TODO: this should somehow use the factory
		virtual id_type UniqueId() const { return id; };
		virtual size_t HashKey() const { return VerifyTAPN::hash()(dp); };

		virtual void Reset(int token)
		{
			LOG(std::cout << "Reset("<<token<<")\n")
			dbm(mapping.GetMapping(token)) = 0;
			LOG(Print())
		};

		virtual bool IsEmpty() const { return dbm.isEmpty(); };
		virtual void Delay()
		{
			LOG(std::cout << "Delay()\n");
			dbm.up();
			LOG(Print());
		}
		virtual void Constrain(int token, const TAPN::TimeInterval& interval)
		{
			LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			int clock = mapping.GetMapping(token);
			dbm.constrain(0,clock, interval.LowerBoundToDBMRaw());
			LOG(std::cout << "After " << clock << " >= " << interval.GetLowerBound() << "\n";)
			LOG(Print();)
			dbm.constrain(clock, 0, interval.UpperBoundToDBMRaw());
			LOG(std::cout << "After " << clock << " <= " << interval.GetUpperBound() << "\n";)
			LOG(Print();)
		};

		virtual bool PotentiallySatisfies(int token, const TAPN::TimeInterval& interval) const
		{
			int clock = mapping.GetMapping(token);
			bool isLowerBoundSat = dbm.satisfies(0, clock, interval.LowerBoundToDBMRaw());
			bool isUpperBoundSat = dbm.satisfies(clock, 0, interval.UpperBoundToDBMRaw());
			bool inappropriateAge = !isLowerBoundSat || !isUpperBoundSat;
			return !inappropriateAge;
		};

		virtual relation Relation(const StoredMarking& other) const
		{
			relation_t relation = dbm.relation(static_cast<const DBMMarking&>(other).dbm);
			return ConvertToRelation(relation);
		}

		virtual void Free(int token) {
			doFree(mapping.GetMapping(token));
		}

		void doFree(int clock) {
			dbm.freeClock(clock);
		}

		virtual void Extrapolate(const int* maxConstants)
		{
			LOG(std::cout << "Extrapolate(...)\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			dbm.extrapolateMaxBounds(maxConstants);
			LOG(std::cout << "output:\n")
			LOG(Print());
		};
		virtual unsigned int GetClockIndex(unsigned int token) const { return mapping.GetMapping(token); };
#ifndef DBM_NORESIZE
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::vector<int>& tokenIndices);
#endif
	private:
		void InitMapping();
		relation ConvertToRelation(relation_t relation) const;

	protected:
		virtual void Swap(int i, int j);
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;

	private: // data
		dbm::dbm_t dbm;
		id_type id;

		void Print() {
			dbm_cppPrint(std::cout, dbm.getDBM(), dbm.getDimension());
		}
	};

}

#endif /* DBMMARKING_HPP_ */
