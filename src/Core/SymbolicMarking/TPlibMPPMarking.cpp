/*
 * TPlibMPPMarking.cpp
 *
 *  Created on: May 9, 2012
 *      Author: ravn
 */

#include "TPlibMPPMarking.hpp"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <climits>


namespace VerifyTAPN {

	boost::shared_ptr<TAPN::TimedArcPetriNet> TPlibMPPMarking::tapn;

	void TPlibMPPMarking::InitZero() {
		matrix_t *initGen = matrix_alloc(1,dp.size()+1);
		for(unsigned int i = 0; i<=dp.size(); i++){
			matrix_set(initGen, 0, i, 0.);
		}
		poly = of_gen(dp.size()+1, initGen);
		matrix_free(initGen);
	}

	void TPlibMPPMarking::InitMapping() {
		for (unsigned int i = 0; i < dp.size(); i++) {
			mapping.SetMapping(i, i + 1);
		}
	}

	void TPlibMPPMarking::Swap(int i, int j) {
		/*TODO*/
	}

	void TPlibMPPMarking::Print(std::ostream& out) const{
		/*TODO*/
	}
	id_type TPlibMPPMarking::UniqueId() const{
		return id;
	}
	unsigned int TPlibMPPMarking::GetClockIndex(unsigned int token) const{
		return mapping.GetMapping(token);
	}
	void TPlibMPPMarking::Reset(int token){
		/*TODO*/
	}
	bool TPlibMPPMarking::IsEmpty() const{
		/*TODO*/
	}
	void TPlibMPPMarking::Delay(){
		/*TODO*/
	}
	void TPlibMPPMarking::Free(int token){
		/*TODO*/
	}
	void TPlibMPPMarking::Constrain(int token, const TAPN::TimeInterval& interval){
		/*TODO*/
	}
	void TPlibMPPMarking::Constrain(int token, const TAPN::TimeInvariant& invariant){
		/*TODO*/
	}
	bool TPlibMPPMarking::PotentiallySatisfies(int token, const TAPN::TimeInterval& interval){
		/*TODO*/
	}
	void TPlibMPPMarking::Extrapolate(const int* maxConstants){
		/*TODO*/
		/*ignoring this function until implemented in the TPlib */
	}
	size_t TPlibMPPMarking::HashKey() const{
		return VerifyTAPN::hash()(dp);
	}
	relation TPlibMPPMarking::Relation(const StoredMarking& other) const{
		/*TODO*/
	}
	void TPlibMPPMarking::AddTokens(const std::list<int>& placeIndices){
		/*TODO*/
	}
	void TPlibMPPMarking::RemoveTokens(const std::set<int>& tokenIndices){
		/*TODO*/
	}

}
