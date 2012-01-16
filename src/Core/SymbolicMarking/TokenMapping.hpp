#ifndef TOKENMAPPING_HPP_
#define TOKENMAPPING_HPP_

#include <vector>
#include <limits>
#include "assert.h"

namespace VerifyTAPN {
	// A token mapping maps a token index 0 <= i <= dbm dimension in the DBM to
	// a new token index in the current marking. Needed for active clock reduction.
	class TokenMapping {
		public: // Constructors
			TokenMapping() : mapping() { };
			explicit TokenMapping(const std::vector<unsigned int>& mapping) : mapping(mapping) {};
			virtual ~TokenMapping() {};

		public: // inspectors
			inline const unsigned int GetMapping(unsigned int index) const
			{
				assert(index < mapping.size());
				return mapping[index];
			};

			const unsigned int size() const { return mapping.size(); };

		public: // modifiers
			void SetMapping(unsigned int index, unsigned int dbmIndex)
			{
				if(index >= mapping.size())	mapping.resize(index+1, -1);

				mapping[index] = dbmIndex;
			};
			void RemoveToken(unsigned int index) { mapping.erase(mapping.begin() + index); }

			void RemoveListOfTokens(std::vector<int> tokenIndex){
				std::vector<unsigned int> newMapping;
				newMapping.reserve(mapping.size());
				for(unsigned int i = 0; i<mapping.size(); ++i){
					if(!tokenIndex.at(i)){
						newMapping.push_back(mapping.at(i));
					}
				}
				mapping = newMapping;
			}

		private:
			std::vector<unsigned int> mapping;
	};

}
#endif /* TOKENMAPPING_HPP_ */
