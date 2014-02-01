/*
 * NonStrictSearch.cpp
 *
 *  Created on: 26/04/2012
 *      Author: MathiasGS
 */

#include "WorkflowSoundness.hpp"

namespace VerifyTAPN {
namespace DiscreteVerification {

WorkflowSoundness::WorkflowSoundness(TAPN::TimedArcPetriNet& tapn, NonStrictMarking& initialMarking, AST::Query* query, VerificationOptions options, WaitingList<NonStrictMarking>* waiting_list)
: Workflow(tapn, initialMarking, query, options, waiting_list), final_set(new vector<NonStrictMarking*>), min_exec(INT_MAX), linearSweepTreshold(3), coveredMarking(NULL), modelType(calculateModelType()){
}

bool WorkflowSoundness::verify(){
	if(addToPW(&initialMarking, NULL)){
		return false;
	}

	// Phase 1
	while(pwList->hasWaitingStates()){
		NonStrictMarking& next_marking = *pwList->getNextUnexplored();
		tmpParent = &next_marking;
		bool noDelay = false;
		Result res = successorGenerator.generateAndInsertSuccessors(next_marking);
		if(res == QUERY_SATISFIED){
			return false;
		}  else if (res == URGENT_ENABLED) {
			noDelay = true;
		}

		// Generate next markings
		if(!noDelay && isDelayPossible(next_marking)){
			NonStrictMarking* marking = new NonStrictMarking(next_marking);
			marking->incrementAge();
			marking->setGeneratedBy(NULL);
			if(addToPW(marking, &next_marking)){
				return false;
			}
		}
	}

	// Phase 2
        // find shortest trace
	for(vector<NonStrictMarking*>::iterator iter = final_set->begin(); iter != final_set->end(); iter++){
		pwList->addToWaiting(*iter);
		if(((WorkflowSoundnessMetaData*)(*iter)->meta)->min < min_exec){
			min_exec = ((WorkflowSoundnessMetaData*)(*iter)->meta)->min;
			lastMarking = (*iter);
		}
	}
        int passed = 0;
        // mark all as passed which ends in a final marking
	while(pwList->hasWaitingStates()){
		NonStrictMarking* next_marking = pwList->getNextUnexplored();
		if(next_marking->meta->passed)	continue;
		next_marking->meta->passed = true;
                ++passed;
		for(vector<NonStrictMarking*>::iterator iter = ((WorkflowSoundnessMetaData*)next_marking->meta)->parents->begin(); iter != ((WorkflowSoundnessMetaData*)next_marking->meta)->parents->end(); iter++){
			pwList->addToWaiting(*iter);
		}
	}

        if(passed < pwList->stored){
                lastMarking = pwList->getUnpassed();
                return false;
        } else {
		return true;
	}
}

bool WorkflowSoundness::addToPW(NonStrictMarking* marking, NonStrictMarking* parent){
	marking->cut();

	unsigned int size = marking->size();

	// Check K-bound
	pwList->setMaxNumTokensIfGreater(size);
	if(modelType == ETAWFN && size > options.getKBound()) {
		lastMarking = marking;
		return true;	// Terminate false
	}

	// Map to existing marking if any
	NonStrictMarking* old = pwList->addToPassed(marking);
        bool isNew = false;
	if(old == NULL){
                isNew = true;
		marking->meta = new WorkflowSoundnessMetaData();
	} else  {
            delete marking;
            marking = old;
        }

	WorkflowSoundnessMetaData* marking_meta_data = ((WorkflowSoundnessMetaData*)marking->meta);

	// add to parents_set
	if(parent != NULL){
		WorkflowSoundnessMetaData* parent_meta_data = ((WorkflowSoundnessMetaData*)parent->meta);
		marking_meta_data->parents->push_back(parent);
		if(marking->getGeneratedBy() == NULL){
			marking_meta_data->min = min(marking_meta_data->min, parent_meta_data->min+1);	// Delay
		}else{
			marking_meta_data->min = min(marking_meta_data->min, parent_meta_data->min);	// Transition
		}
	}else{
		marking_meta_data->min = 0;
	}



	// Test if final place
	if(marking->numberOfTokensInPlace(out->getIndex()) > 0){
		if(size == 1){
			marking_meta_data = ((WorkflowSoundnessMetaData*)marking->meta);
			marking_meta_data->parents->push_back(parent);
			// Set min
			marking_meta_data->min = min(marking_meta_data->min, ((WorkflowSoundnessMetaData*)parent->meta)->min);	// Transition
			final_set->push_back(marking);
		}else{
			lastMarking = marking;
			return true;	// Terminate false
		}
	}else{
		// If new marking
		if(isNew){
                        pwList->addToWaiting(marking);
			if(parent != NULL && marking->canDeadlock(tapn, 0)){
				lastMarking = marking;
				return true;
			}
			if(modelType == MTAWFN && checkForCoveredMarking(marking)){
				lastMarking = marking;
				return true;	// Terminate false
			}
		}
	}
	return false;
}

bool WorkflowSoundness::checkForCoveredMarking(NonStrictMarking* marking){
	if(marking->size() <= options.getKBound()){
		return false;	// Do not run check on small markings (invoke more rarely)
	}

	NonStrictMarking* covered = pwList->getCoveredMarking(marking, (marking->size() > linearSweepTreshold));
	if(covered != NULL){
		coveredMarking = covered;
		return true;
	}

	return false;
}

void WorkflowSoundness::getTrace(NonStrictMarking* base){
	stack < NonStrictMarking*> printStack;
	NonStrictMarking* next = base;
	do{
		int min = INT_MAX;
		NonStrictMarking* parent = NULL;
		for(vector<NonStrictMarking*>::const_iterator iter = ((WorkflowSoundnessMetaData*)next->meta)->parents->begin(); iter != ((WorkflowSoundnessMetaData*)next->meta)->parents->end(); ++iter){
			if(((WorkflowSoundnessMetaData*)(*iter)->meta)->min < min){
				min = ((WorkflowSoundnessMetaData*)(*iter)->meta)->min;
				parent = *iter;
			}
		}

		if(((WorkflowSoundnessMetaData*)next->meta)->inTrace){ break;	}
		((WorkflowSoundnessMetaData*)next->meta)->inTrace = true;
		printStack.push(next);
		next = parent;

	}while(((WorkflowSoundnessMetaData*)next->meta)->parents != NULL && !((WorkflowSoundnessMetaData*)next->meta)->parents->empty());

	if(printStack.top() != next){
		printStack.push(next);
	}

	stack < NonStrictMarking*> tempStack;
	while(!printStack.empty()){
		printStack.top()->meta->inTrace = false;
		tempStack.push(printStack.top());
		printStack.pop();
	}

	while(!tempStack.empty()){
		printStack.push(tempStack.top());
		tempStack.pop();
	}

	if(options.getXmlTrace()){
		printXMLTrace(lastMarking, printStack, query, tapn);
	} else {
		printHumanTrace(lastMarking, printStack, query->getQuantifier());
	}
}

WorkflowSoundness::ModelType WorkflowSoundness::calculateModelType() {
            bool isin, isout;
            bool hasInvariant = false;
            for (TimedPlace::Vector::const_iterator iter = tapn.getPlaces().begin(); iter != tapn.getPlaces().end(); iter++) {
                isin = isout = true;
                TimedPlace* p = (*iter);
                if (p->getInputArcs().empty() && p->getOutputArcs().empty() && p->getTransportArcs().empty()) {
                    bool continueOuter = true;
                    // Test if really orphan place or if out place
                    for (TransportArc::Vector::const_iterator trans_i = tapn.getTransportArcs().begin(); trans_i != tapn.getTransportArcs().end(); ++trans_i) {
                        if (&((*trans_i)->getDestination()) == p) {
                            continueOuter = false;
                            break;
                        }
                    }
                    if (continueOuter) continue; // Fix orphan places
                }

                if (!hasInvariant && p->getInvariant() != p->getInvariant().LS_INF) {
                    hasInvariant = true;
                }

                if (p->getInputArcs().size() > 0) {
                    isout = false;
                }

                if (p->getOutputArcs().size() > 0) {
                    isin = false;
                }

                if (isout) {
                    for (TransportArc::Vector::const_iterator iter = p->getTransportArcs().begin(); iter != p->getTransportArcs().end(); iter++) {
                        if (&(*iter)->getSource() == p) {
                            isout = false;
                            break;
                        }
                    }
                }

                if (isin) {
                    for (TransportArc::Vector::const_iterator iter = tapn.getTransportArcs().begin(); iter != tapn.getTransportArcs().end(); ++iter) { // TODO maybe transportArcs should contain both incoming and outgoing? Might break something though.
                        if (&(*iter)->getDestination() == p) {
                            isin = false;
                            break;
                        }
                    }
                }

                if (isin) {
                    if (in == NULL) {
                        in = p;
                    } else {
                        return NOTTAWFN;
                    }
                }

                if (isout) {
                    if (out == NULL) {
                        out = p;
                    } else {
                        return NOTTAWFN;
                    }
                }

            }

            if (in == NULL || out == NULL || in == out) {
                return NOTTAWFN;
            }

            if (initialMarking.size() != 1 || initialMarking.numberOfTokensInPlace(in->getIndex()) != 1) {
                return NOTTAWFN;
            }

            bool hasUrgent = false;
            bool hasInhibitor = false;
            // All transitions must have preset
            for (TimedTransition::Vector::const_iterator iter = tapn.getTransitions().begin(); iter != tapn.getTransitions().end(); iter++) {
                if ((*iter)->getPresetSize() == 0 && (*iter)->getNumberOfTransportArcs() == 0) {
                    return NOTTAWFN;
                }

                if (!hasUrgent && (*iter)->isUrgent()) {
                    hasUrgent = true;
                }

                if (!hasInhibitor && !(*iter)->getInhibitorArcs().empty()) {
                    hasInhibitor = true;
                }
            }

            if (hasUrgent || hasInvariant || hasInhibitor) {
                return ETAWFN;
            }

            return MTAWFN;
        }

WorkflowSoundness::~WorkflowSoundness() {
}

} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
