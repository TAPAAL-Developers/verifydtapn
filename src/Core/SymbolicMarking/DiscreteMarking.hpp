#ifndef DISCRETEMARKING_HPP_
#define DISCRETEMARKING_HPP_

#include "SymbolicMarking.hpp"
#include "DiscretePart.hpp"
#include "TokenMapping.hpp"
#include <queue>

#ifdef DEBUG
#define LOG(x) { if (DiscreteMarking::debug) {x;} }
#else
#define LOG(x)
#endif

namespace VerifyTAPN {

	class DiscreteMarking : public SymbolicMarking {
	public:
		static bool debug;
		DiscreteMarking(const DiscretePart& dp) : unusedClocks(), dp(dp), mapping() { };
		DiscreteMarking(const DiscreteMarking& dm) : unusedClocks(dm.unusedClocks), dp(dm.dp), mapping(dm.mapping) { };
		virtual ~DiscreteMarking() { };

	public:
		virtual void MoveToken(int token, int targetPlace) { dp.MoveToken(token, targetPlace); };
		virtual unsigned int NumberOfTokens() const { return dp.size(); };
		virtual unsigned int NumberOfTokensInPlace(int placeIndex) const { return dp.NumberOfTokensInPlace(placeIndex); };
		virtual int GetTokenPlacement(int token) const { return dp.GetTokenPlacement(token); };
		virtual void AddTokens(const std::list<int>& placeIndices)
		{
			for(std::list<int>::const_iterator i = placeIndices.begin(); i != placeIndices.end(); i++)
			{
				if (unusedClocks.empty()) {
					mapping.AddTokenToMapping(dp.size()+1);
				} else {
					mapping.AddTokenToMapping(unusedClocks.front());
					unusedClocks.pop();
				}
				dp.AddTokenInPlace(*i);
				Reset(dp.size()-1);
			}
		};

		virtual void RemoveTokens(const std::vector<int>& tokenIndices)
		{
			for(std::vector<int>::const_iterator i = tokenIndices.begin(); i != tokenIndices.end(); i++)
			{
				unusedClocks.push(mapping.GetMapping(*i));
				Free(*i);
			}

			for(int i = tokenIndices.size() - 1; i >= 0; --i) {
				mapping.RemoveToken(tokenIndices[i]);
				dp.RemoveToken(tokenIndices[i]);
			}
		};

		// Sort the state internally to obtain a canonical form.
		// Used for symmetry reduction: if two states are symmetric they will have the same canonical form.
		// The placement vector is sorted in ascending order, tokens in the same place are sorted on their lower bound,
		// subsequently on their upper bound and finally by diagonal constraints if necessary.
		virtual void Canonicalize() { quickSort(0, dp.size()-1); };

	private:
		void quickSort(int left, int right)
		{
			if(right > left)
			{
				int pivot = left + (right - left)/2;
				int newPivot = Partition(left, right, pivot);
				quickSort(left, newPivot - 1);
				quickSort(newPivot + 1, right);
			}
		};

		int Partition(int left, int right, int pivot)
		{
			Swap(pivot, right);
			int indexToReturn = left;
			for(int i = left; i < right; ++i)
			{
				//if(IsLowerPositionLessThanPivot(i, right)) // Does not give optimal stored states unless its changed to mapPivot > mapLower in function
				if(!IsUpperPositionGreaterThanPivot(i, right))
				{
					Swap(i, indexToReturn);
					indexToReturn++;
				}
			}
			Swap(indexToReturn, right);
			return indexToReturn;
		};

	protected:
		virtual bool IsUpperPositionGreaterThanPivot(int upper, int pivotIndex) const
		{
			int placeUpper = dp.GetTokenPlacement(upper);
			int pivot = dp.GetTokenPlacement(pivotIndex);
			return placeUpper > pivot;
		};

		virtual void Swap(int i, int j)
		{
			dp.Swap(i,j);
		};
	private:
		std::queue<int> unusedClocks;
	protected: // data
		DiscretePart dp;
		TokenMapping mapping;
	};

}

#endif /* DISCRETEMARKING_HPP_ */
