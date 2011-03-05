#ifndef TRACEINFO_HPP_
#define TRACEINFO_HPP_

#include "../../Core/TAPN/TimeInterval.hpp"
#include <vector>
#include "../../Core/SymbolicMarking/SymbolicMarking.hpp"

namespace VerifyTAPN
{
	class Participant
	{
	public:
		Participant(unsigned int tokenIndex, const TAPN::TimeInterval& ti, unsigned int indexAfterFiring)
			: tokenIndex(tokenIndex), clockIndex(tokenIndex), ti(ti), indexAfterFiring(indexAfterFiring) { };
	public:
		inline unsigned int ClockIndex() const { return clockIndex; }
		inline unsigned int TokenIndex() const { return tokenIndex; };
		inline const TAPN::TimeInterval& GetTimeInterval() const { return ti; };
		inline unsigned int IndexAfterFiring() const { return indexAfterFiring; };
	private:
		unsigned int tokenIndex;
		unsigned int clockIndex;
		TAPN::TimeInterval ti;
		unsigned int indexAfterFiring;
	};

	class TraceInfo
	{
	public:
		typedef long long id_type;
	public:
		TraceInfo(id_type prevState, int transitionIndex, id_type stateId)
			: stateId(stateId), prevState(prevState), transitionIndex(transitionIndex) {};
	public:
		inline id_type StateId() const { return stateId; };
		inline id_type PreviousStateId() const { return prevState; };
		inline int	TransitionIndex() const { return transitionIndex; };
		inline const std::vector<Participant>& Participants() const { return participants; };
	public:
		inline void AddParticipant(const Participant& participant) { participants.push_back(participant); };
		inline void setMarking(const SymbolicMarking* marking) { this->marking = marking; };
		inline const SymbolicMarking& Marking() const { return *marking; };
	private: // data
		id_type stateId;
		id_type prevState;
		int transitionIndex;
		std::vector<Participant> participants;
		const SymbolicMarking* marking;
	};
}

#endif /* TRACEINFO_HPP_ */
