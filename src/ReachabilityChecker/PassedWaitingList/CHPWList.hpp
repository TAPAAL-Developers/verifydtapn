#ifndef CHPWLIST_HPP_
#define CHPWLIST_HPP_

#include "google/sparse_hash_map"
#include <list>
#include "PassedWaitingList.hpp"
#include "Node.hpp"
#include "../../Core/SymbolicMarking/DiscretePart.hpp"
#include "WaitingList.hpp"

namespace VerifyTAPN {
	class SymbolicMarking;
	class Node;
	class MarkingFactory;

	class CHPWList : public PassedWaitingList {
	private:
		//typedef google::sparse_hash_map<const DiscretePart, NodeList, VerifyTAPN::hash, VerifyTAPN::eqdp > HashMap;
		typedef google::sparse_hash_map<size_t, Node*> HashMap; // TODO: Check if we need to explicitly change hash function to identity?
	public:
		CHPWList(WaitingList* waitingList, MarkingFactory* factory) : map(256000), stats(), waitingList(waitingList), factory(factory) {};
		virtual ~CHPWList();

	public: // inspectors
		virtual bool HasWaitingStates() const;
		virtual long long Size() const;
		virtual Stats GetStats() const;

		virtual void Print() const;

	public: // modifiers
		virtual bool Add(const SymbolicMarking& symMarking);
		virtual SymbolicMarking* GetNextUnexplored();
	private:
		HashMap map;
		Stats stats;
		WaitingList* waitingList;
		MarkingFactory* factory;
	};
}

#endif /* CHPWLIST_HPP_ */
