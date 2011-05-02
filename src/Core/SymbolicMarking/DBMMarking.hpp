#ifndef DBMMARKING_HPP_
#define DBMMARKING_HPP_

#include "DiscreteMarking.hpp"
#include "StoredMarking.hpp"
#include "TokenMapping.hpp"
#include "dbm/fed.h"
#include "dbm/print.h"
#include "MarkingFactory.hpp"
#include "../TAPN/TimedArcPetriNet.hpp"
#include <iostream>

namespace VerifyTAPN {

	class DBMMarking: public DiscreteMarking, public StoredMarking {
		friend class UppaalDBMMarkingFactory;
		friend class DiscreteInclusionMarkingFactory;
		DBMMarking(const DiscretePart& dp, const dbm::dbm_t& dbm) : DiscreteMarking(dp), dbm(dbm) { InitMapping(); };
		DBMMarking(const DiscretePart& dp, const TokenMapping& mapping, const dbm::dbm_t& dbm) : DiscreteMarking(dp, mapping), dbm(dbm) { assert(IsConsistent()); };
		DBMMarking(const DBMMarking& dm) : DiscreteMarking(dm), dbm(dm.dbm) { };
		static boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;

		virtual ~DBMMarking() { };

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
			for(unsigned int i = 0; i < NumberOfTokens(); i++)
			{
				const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
				Constrain(i, invariant);
				assert(!IsEmpty()); // this should not be possible
			}
			LOG(Print());
		};
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

		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant)
		{
			if(invariant.GetBound() != std::numeric_limits<int>::max())
			{
				dbm.constrain(mapping.GetMapping(token), 0, dbm_boundbool2raw(invariant.GetBound(), invariant.IsBoundStrict()));
			}
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

		virtual void ConvexUnion(AbstractMarking* marking) {
			DBMMarking* m = static_cast<DBMMarking*>(marking);
			dbm_convexUnion(dbm.getDBM(), m->dbm.getDBM(), dbm.getDimension());
		}

		virtual void Extrapolate(const int* maxConstants)
		{
			LOG(std::cout << "Extrapolate(...)\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			dbm.diagonalExtrapolateMaxBounds(maxConstants);
			LOG(std::cout << "output:\n")
			LOG(Print());
		};
		virtual unsigned int GetClockIndex(unsigned int token) const { return mapping.GetMapping(token); };
		virtual void AddTokens(const std::list<int>& placeIndices);
		virtual void RemoveTokens(const std::vector<int>& tokenIndices);

		raw_t GetLowerBound(int clock) const { return dbm(0,clock); };
		const dbm::dbm_t& GetDBM() const { return dbm; };

	private:
		void InitMapping();

		bool IsConsistent() const
		{
			if(dp.size() != dbm.getDimension()-1)
			{
				return false;
			}

			if(mapping.size() != dp.size()) return false;

			for(unsigned int i = 0; i < dp.size(); i++)
			{
				unsigned int mappedIndex = mapping.GetMapping(i);
				if(mappedIndex == 0 || mappedIndex >= dbm.getDimension())
					return false;
			}
			return true;
		};

	protected:
		virtual void Swap(int i, int j);
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const;
		relation ConvertToRelation(relation_t relation) const;

	protected: // data
		dbm::dbm_t dbm;
		id_type id;

		void Print() {
			dbm_cppPrint(std::cout, dbm.getDBM(), dbm.getDimension());
			std::cout << "Placement vector:";
			std::vector<int> places = dp.GetTokenPlacementVector();
			for (std::vector<int>::iterator place = places.begin(); place != places.end(); ++place)
				std::cout << " " << *place;
			std::cout << "\n";
		}
	};

}

#endif /* DBMMARKING_HPP_ */
