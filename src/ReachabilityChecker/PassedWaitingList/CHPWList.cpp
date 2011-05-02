#include "CHPWList.hpp"
#include "../../Core/SymbolicMarking/SymbolicMarking.hpp"
#include "../../Core/SymbolicMarking/MarkingFactory.hpp"

namespace VerifyTAPN {
	CHPWList::~CHPWList()
	{
		 delete waitingList;
		 for(HashMap::iterator iter = map.begin(); iter != map.end(); ++iter){
			delete iter->second;
		 }
	}

	bool CHPWList::Add(const SymbolicMarking& symMarking)
	{
		StoredMarking* storedMarking = factory->Convert(const_cast<SymbolicMarking*>(&symMarking));
		stats.discoveredStates++;
		//const DiscretePart& dp = symMarking.GetDiscretePart();
		Node* currentNode = map[storedMarking->HashKey()];

		bool retval = true;

		if(currentNode)
		{
			relation relation 	= storedMarking->Relation(*currentNode->GetMarking());
			//assert(eqdp()(currentNode->GetMarking().GetDiscretePart(), dp));

			if((relation & SUBSET) != 0)
			{ // check subseteq
				factory->Release(storedMarking);
				return false;
			}
			else if(relation == DIFFERENT)
			{
				SymbolicMarking* currMarking = factory->Convert(currentNode->GetMarking());
				SymbolicMarking* newMarking = factory->Clone(symMarking);
				newMarking->ConvexUnion(currMarking);
				storedMarking = factory->Convert(newMarking);
				retval = false;
			}
			assert(currentNode->GetColor() != COVERED);
			if (currentNode->GetColor() == WAITING) {
				waitingList->DecrementActualSize();
				currentNode->Recolor(COVERED);
			} else {
				delete currentNode;
			}
		} else {
			stats.storedStates++;
		}

		Node* node = new Node(storedMarking, WAITING);
		map[storedMarking->HashKey()] = node;
		waitingList->Add(node);

		return retval;
	}

	long long CHPWList::Size() const
	{
		return map.size();
	}

	bool CHPWList::HasWaitingStates() const
	{
		return waitingList->Size() > 0;
	}

	SymbolicMarking* CHPWList::GetNextUnexplored()
	{
		stats.exploredStates++;
		StoredMarking* next = waitingList->Next()->GetMarking();
		return factory->Convert(next);
	}

	Stats CHPWList::GetStats() const
	{
		return stats;
	}

	void CHPWList::Print() const
	{
		std::cout << stats;
		std::cout << ", waitingList: " << waitingList->Size() << "/" << waitingList->SizeIncludingCovered();
	}
}
