
#include "DiscretePart.hpp"
#include "../TAPN/TimedPlace.hpp"
#include <algorithm>
#include <exception>

namespace VerifyTAPN {

	void DiscretePart::MoveToken(unsigned int tokenIndex, int newPlaceIndex)
	{
		assert(tokenIndex >= 0 && tokenIndex < placement.size());

		placement[tokenIndex] = newPlaceIndex;
	}

	int DiscretePart::NumberOfTokensInPlace(int placeIndex) const
	{
		int i = 0;
		for(std::vector<int>::const_iterator iter = placement.begin(); iter != placement.end(); ++iter)
		{
			if((*iter) == placeIndex)
			{
				i++;
			}
		}

		return i;
	}

	void DiscretePart::Swap(int tokenI, int tokenJ)
	{
		int temp = placement[tokenI];
		placement[tokenI] = placement[tokenJ];
		placement[tokenJ] = temp;
	}

	void DiscretePart::RemoveListOfTokens(std::vector<int> tokenIndex){
		std::vector<int> newPlacements;
		newPlacements.reserve(placement.size());
		for(unsigned int i = 0; i<placement.size(); ++i){
			if(!tokenIndex.at(i)){
				newPlacements.push_back(placement.at(i));
			}
		}
		placement = newPlacements;
	}
}
