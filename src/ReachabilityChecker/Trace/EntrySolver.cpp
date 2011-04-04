#include "EntrySolver.hpp"
#include "dbm/print.h"

namespace VerifyTAPN
{
	std::vector<decimal> EntrySolver::CalculateDelays()
	{
		CreateLastResetAtLookupTable();
		CreateEntryTimeDBM();
		return FindSolution();
	}

	// See CTU - DCCreator.cpp for details!
	void EntrySolver::CreateLastResetAtLookupTable()
	{
		unsigned int locations = traceInfos.size()+1;

		if(lraTable != 0) delete[] lraTable;
		lraTable = new unsigned int[locations*clocks]; // TODO: check that this gives correct number of places

		for(unsigned int i = 0; i < clocks; i++)
		{
			lraTable[i] = 0;
		}

		for(unsigned int loc = 1; loc < locations; loc++)
		{
			unsigned int index = loc-1;
			for(unsigned int clock = 0; clock < clocks; clock++)
			{
				bool isClockUsed = IsClockResetInStep(clock, traceInfos[index]);
				if(isClockUsed)
					lraTable[loc*clocks+clock] = loc;
				else
					lraTable[loc*clocks+clock] = lraTable[(loc-1)*clocks+clock];
			}
		}
	}

	bool EntrySolver::IsClockResetInStep(unsigned int clock, const TraceInfo& traceInfo) const
	{
		const std::vector<Participant>& participants = traceInfo.Participants();
		for(std::vector<Participant>::const_iterator it = participants.begin(); it != participants.end(); it++)
		{
			const Participant& participant = *it;

			if(static_cast<unsigned int>(participant.ClockIndexAfterDiscreteUpdate()) == clock)
			{
				return true;
			}

		}
		return false;
	}

	// See CTU - DCCreator.cpp for details!
	void EntrySolver::CreateEntryTimeDBM()
	{
		unsigned int dim = traceInfos.size();
		entryTimeDBM.setInit();


		// preliminary invariants using AfterAction here!

		// Algorithm: // TODO: add invariants to algorithm
		// For each step i in the trace do
		// 		For each time interval ti in the guards of step i
		//			let ti_low be constraint representing lower bound
		//			let ti_up be constraint representing upper bound
		//			compute AfterDelay(i, ti_low), add to DBM
		// 			compute AfterDelay(i, ti_up), add to DBM
		for(unsigned int i = 0; i < dim; i++)
		{
			const TraceInfo& traceInfo = traceInfos[i];

			typedef std::vector<Participant>::const_iterator const_iterator;
			const_iterator begin = traceInfo.Participants().begin();
			const_iterator end = traceInfo.Participants().end();
			for(const_iterator it = begin; it != end; it++)
			{
				if(it->TokenIndex() != -1)
				{
					const TAPN::TimeInterval& interval = it->GetTimeInterval();

					constraint_t lower(0, it->ClockIndex(), interval.LowerBoundToDBMRaw());
					constraint_t upper(it->ClockIndex(), 0, interval.UpperBoundToDBMRaw());

					constraint_t entryLower(AfterDelay(i, lower));
					entryTimeDBM.constrain(entryLower);
					if(interval.UpperBoundToDBMRaw() != dbm_LS_INFINITY)
					{
						constraint_t entryUpper(AfterDelay(i, upper));
						entryTimeDBM.constrain(entryUpper);
					}
				}
			}
		}

		// add constraints e_i - e_i+1 <= 0
		for(unsigned int i = 0; i < dim; i++)
		{
			entryTimeDBM.constrain(i, i+1, 0, false);
		}
	}

	// Theory: AfterAction(Trace, index, guard/invariant)
	constraint_t EntrySolver::AfterAction(unsigned int locationIndex, const constraint_t& constraint) const
	{
		if(constraint.j == 0 && constraint.i != 0)
			return constraint_t(locationIndex, LastResetAt(locationIndex, constraint.i), constraint.value);
		else if(constraint.i == 0 && constraint.j != 0)
			return constraint_t(LastResetAt(locationIndex, constraint.j), locationIndex, constraint.value);
		else
			return constraint_t(LastResetAt(locationIndex, constraint.j), LastResetAt(locationIndex, constraint.i), constraint.value);
	}

	// Theory: AfterDelay(Trace, index, guard/invariant)
	constraint_t EntrySolver::AfterDelay(unsigned int locationIndex, const constraint_t& constraint) const
	{
		if(constraint.i != 0 && constraint.j == 0)
			return constraint_t(locationIndex+1, LastResetAt(locationIndex, constraint.i), constraint.value);
		else if(constraint.i == 0 && constraint.j != 0)
			return constraint_t(LastResetAt(locationIndex, constraint.j), locationIndex+1, constraint.value);
		else
			return constraint_t(LastResetAt(locationIndex, constraint.j), LastResetAt(locationIndex, constraint.i), constraint.value);
	}

	// This is straight port from CTU implementation. See CTU -- SolutionFinder.cpp for details
	std::vector<decimal> EntrySolver::FindSolution() const
	{
		unsigned int dim = entryTimeDBM.getDimension();
		std::vector<decimal> entry_times(dim);
		bool restricted[dim];

		for(unsigned int i = 0; i < dim; i++)
		{
			restricted[i] = false;
		}

		// make sure we start at time 0
		entry_times[0] = decimal(0);
		restricted[0] = true; // ensure time 0 is final

		for(unsigned int i = 1; i < dim; i++)
		{
			if(!restricted[i])
			{
				bool lowerStrict = dbm_rawIsStrict(entryTimeDBM(0,i));
				decimal lower = decimal(-dbm_raw2bound(entryTimeDBM(0,i)));

				bool upperStrict = dbm_rawIsStrict(entryTimeDBM(i,0));
				decimal upper = decimal(dbm_raw2bound(entryTimeDBM(i,0)));

				// try to derive tighter bounds
				for(unsigned int j = 1; j < dim; j++)
				{
					if(restricted[j] && i != j)
					{
						bool strict = dbm_rawIsStrict(entryTimeDBM(i,j));
						decimal bound = decimal(dbm_raw2bound(entryTimeDBM(i,j))) + entry_times[j];
						if(bound < upper || (bound == upper && strict))
						{
							upperStrict = strict;
							upper = bound;
						}

						strict = dbm_rawIsStrict(entryTimeDBM(j,i));
						bound = decimal(-dbm_raw2bound(entryTimeDBM(j,i))) + entry_times[j];
						if(bound > lower || (bound == lower && strict))
						{
							lowerStrict = strict;
							lower = bound;
						}
					}
				}

				// These are the tightest bounds so we find a value in this range
				entry_times[i] = FindValueInRange(lowerStrict, lower, upper, upperStrict, entry_times[i-1]);
				restricted[i] = true;
			}
		}

		std::vector<decimal> delays(entry_times.size());
		ConvertEntryTimesToDelays(entry_times, delays);

		return delays;
	}

	decimal EntrySolver::FindValueInRange(bool lowerStrict, decimal lower, decimal upper, bool upperStrict, decimal lastEntryTime) const
	{
		decimal diff = upper - lower;

		assert(lower <= upper);
		assert((!lowerStrict && !upperStrict) || diff > 0); // ensures range is not (a,a], [a,a) or (a,a)

		if(!lowerStrict)
		{
			return lower; // Safe due to assert above
		}
		else if(lowerStrict && diff > 1)
		{
			return lower + decimal(1);
		}
		else // diff <= 1 && lowerStrict
		{
			if(!upperStrict)
			{
				return upper;
			}
			else
			{
				return (lower + upper) / 2; // return mean
			}
		}
	}

	void EntrySolver::ConvertEntryTimesToDelays(const std::vector<decimal>& entry_times, std::vector<decimal>& delays) const
	{
		for(unsigned int i = 0; i < entry_times.size()-1; i++)
		{
			delays[i] = entry_times[i+1] - entry_times[i];
		}
	}
}
