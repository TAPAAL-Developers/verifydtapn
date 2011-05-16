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

//for debugging
#include "MPPMarking.hpp"
#include "MPVector.hpp"

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
				LOG(std::cout << "input:\n")
				LOG(Print());
			dbm(mapping.GetMapping(token)) = 0;
			LOG(std::cout << "output:\n")
			LOG(Print());
		};

		virtual bool IsEmpty() const { return dbm.isEmpty(); };
		virtual void Delay()
		{
			LOG(std::cout << "Delay()\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			dbm.up();
			for(unsigned int i = 0; i < NumberOfTokens(); i++)
			{
				const TAPN::TimeInvariant& invariant = tapn->GetPlace(GetTokenPlacement(i)).GetInvariant();
				if(invariant.GetBound() != std::numeric_limits<int>::max()) {
					LOG(std::cout << "Found invariant in place " << GetTokenPlacement(i) << "\n";)
				}
				Constrain(i, invariant);
				assert(!IsEmpty()); // this should not be possible
			}
			LOG(std::cout << "output:\n")
			LOG(Print());
		};
		virtual void Constrain(int token, const TAPN::TimeInterval& interval)
		{
			LOG(std::cout << "Constrain(" << token << ", " << interval.GetLowerBound() << ".." << interval.GetUpperBound() << ")\n");
			LOG(std::cout << "input:\n")
			LOG(Print());
			int clock = mapping.GetMapping(token);
			if (interval.GetUpperBound() != std::numeric_limits<int>::max()) {
			dbm.constrain(clock, 0, interval.UpperBoundToDBMRaw());
			LOG(std::cout << "After " << clock << " <= " << interval.GetUpperBound() << "\n";)
			LOG(Print();)
			}
			dbm.constrain(0,clock, interval.LowerBoundToDBMRaw());
			LOG(std::cout << "After " << clock << " >= " << interval.GetLowerBound() << "\n";)
			LOG(Print();)
		};

		virtual void Constrain(int token, const TAPN::TimeInvariant& invariant)
		{
			if(invariant.GetBound() != std::numeric_limits<int>::max())
			{
				//LOG(std::cout << "Constrain(" << token << " <= " << invariant.GetBound() << ")\n";)
				/*LOG(std::cout << "input:\n")
				LOG(Print());*/
				dbm.constrain(mapping.GetMapping(token), 0, dbm_boundbool2raw(invariant.GetBound(), invariant.IsBoundStrict()));
				//LOG(std::cout << "output:\n")
				//LOG(Print());
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
		{//return;
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
	public:
		virtual void Print() const{
			//std::cout << dbm;
			PrintAsMPP();
			return;

			std::cout << "Placement vector:";
			std::vector<int> places = dp.GetTokenPlacementVector();
			for (std::vector<int>::iterator place = places.begin(); place != places.end(); ++place)
				std::cout << " " << *place;
			std::cout << "\n";
		}

		void PrintAsMPP() const {
			if (dbm.isEmpty())
				return;
			MPPMarking<MPVector>::MPVecSet v;
			MPPMarking<MPVector>::MPVecSet w;
			v.push_back(MPVector(dp.size(),0));
			for (size_t i = 1; i <= dp.size(); i++)
			{
				MPVector vec = MPVector(dp.size(), NegInf);
				vec.Set(i,0);
				w.push_back(vec);
			}
			MPPMarking<MPVector> mpp = MPPMarking<MPVector>(dp,v,w);

			//mpp.PolyToCone();
			for (size_t i = 0; i <= dp.size(); i++) {
				//mpp.PolyToCone();
				for (size_t j = 0; j <= dp.size(); j++) {
					if (i==j)
						continue;
					mpp.PolyToCone();
					mpp.doConstrain(i,j,dbm_raw2bound(dbm[i][j]));
					mpp.ConeToPoly();
					mpp.Cleanup();
				}
				//mpp.ConeToPoly();
				//mpp.Cleanup();
			}
			//std::cout << mpp.W.size() <<"\n";
			//mpp.ConeToPoly();
			//mpp.Cleanup();
			mpp.Print();
		}
	};

}

#endif /* DBMMARKING_HPP_ */
