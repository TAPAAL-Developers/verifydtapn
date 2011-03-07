#ifndef DBMMARKING_HPP_
#define DBMMARKING_HPP_

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "dbm/fed.h"
#include "MarkingFactory.hpp"

namespace VerifyTAPN {

	class DBMMarking: public DiscreteMarking, public StoredMarking {
		friend class UppaalDBMMarkingFactory;
	private:
		static MarkingFactory* factory;
	public:
		DBMMarking(const DiscretePart& dp, const dbm::dbm_t& dbm) : DiscreteMarking(dp), dbm(dbm), mapping() { InitMapping(); };
		DBMMarking(const DiscretePart& dp, const TokenMapping& mapping, const dbm::dbm_t& dbm) : DiscreteMarking(dp), dbm(dbm), mapping(mapping) { };
		DBMMarking(const DBMMarking& dm) : DiscreteMarking(dm), dbm(dm.dbm), mapping(dm.mapping) { };
		virtual ~DBMMarking() { };

		//virtual SymbolicMarking* Clone() const { return factory->Clone(*this); }; // TODO: this should somehow use the factory
		virtual id_type UniqueId() const { return id; };
		virtual size_t HashKey() const { return VerifyTAPN::hash()(dp); };

		virtual void Reset(int token) { dbm(mapping.GetMapping(token)) = 0; };
		virtual bool IsEmpty() const { return dbm.isEmpty(); };
		virtual void Delay() { dbm.up(); };
		virtual void Constrain(int token, const TAPN::TimeInterval& interval)
		{
			int clock = mapping.GetMapping(token);
			dbm.constrain(0,clock, interval.LowerBoundToDBMRaw());
			dbm.constrain(clock, 0, interval.UpperBoundToDBMRaw());
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

		virtual void Extrapolate(const int* maxConstants) { dbm.extrapolateMaxBounds(maxConstants); };
		virtual unsigned int GetClockIndex(unsigned int token) const { return mapping.GetMapping(token); };

		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::vector<int>& tokenIndices);

		raw_t GetLowerBound(int clock) const { return dbm(0,clock); };
		const dbm::dbm_t& GetDBM() const { return dbm; };
	private:
		void InitMapping();

	protected:
		virtual void Swap(int i, int j);
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;
		relation ConvertToRelation(relation_t relation) const;

	protected: // data
		dbm::dbm_t dbm;
		TokenMapping mapping;
		id_type id;
	};

}

#endif /* DBMMARKING_HPP_ */
