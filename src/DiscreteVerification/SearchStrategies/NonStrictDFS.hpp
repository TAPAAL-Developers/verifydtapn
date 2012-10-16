/*
 * NonStrictDFS.hpp
 *
 *  Created on: 05/03/2012
 *      Author: MathiasGS
 */

#ifndef NONSTRICTDFS_HPP_
#define NONSTRICTDFS_HPP_

#include "../DataStructures/PWList.hpp"
#include "boost/smart_ptr.hpp"
#include "../../Core/TAPN/TAPN.hpp"
#include "../../Core/QueryParser/AST.hpp"
#include "../../Core/VerificationOptions.hpp"

#include "../../Core/TAPN/TimedPlace.hpp"
#include "../../Core/TAPN/TimedTransition.hpp"
#include "../../Core/TAPN/TimedInputArc.hpp"
#include "../../Core/TAPN/TransportArc.hpp"
#include "../../Core/TAPN/InhibitorArc.hpp"
#include "../../Core/TAPN/OutputArc.hpp"

#include "../SuccessorGenerator.hpp"

#include "../QueryVisitor.hpp"
#include "boost/any.hpp"

#include <stack>
#include "SearchStrategy.hpp"

namespace VerifyTAPN {

namespace DiscreteVerification {

class NonStrictDFS : public SearchStrategy {
public:
	virtual WaitingList* CreateWaitingList(AST::Query* query) const { return new StackWaitingList; };
};

}
}
#endif /* NONSTRICTDFS_HPP_ */