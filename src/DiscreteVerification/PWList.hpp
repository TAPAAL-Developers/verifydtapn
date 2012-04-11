/*
 * PWList.hpp
 *
 *  Created on: 01/03/2012
 *      Author: MathiasGS
 */

#ifndef PWLIST_HPP_
#define PWLIST_HPP_

#include "WaitingList.hpp"
#include <iostream>
#include "google/sparse_hash_map"
#include  "NonStrictMarking.hpp"

namespace VerifyTAPN {
namespace DiscreteVerification {

class PWList {
public:
	typedef google::sparse_hash_map<size_t, NonStrictMarking> HashMap;
public:
	PWList() : markings_storage(256000) {};
	virtual ~PWList();
	friend std::ostream& operator<<(std::ostream& out, PWList& x);

public: // inspectors
	virtual bool HasWaitingStates() {
		return (waiting_list.Size() > 0);
	};

	virtual long long Size() const {
		//TODO: implement!
		return markings_storage.size();
	};

	virtual void Print() const {
		//TODO: implement!
	};

public: // modifiers
	virtual bool Add(NonStrictMarking* marking);
	virtual NonStrictMarking* GetNextUnexplored();

private:
	HashMap markings_storage;
	StackWaitingList waiting_list;
};

std::ostream& operator<<(std::ostream& out, PWList& x);

} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
#endif /* PWLIST_HPP_ */
