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
		int perm[dp.size()+1];
		perm[0] = 0;
		for(unsigned int k = 1; k<=dp.size(); k++){
			if(k == i){
				perm[k] = GetClockIndex(j);
			}
			else if (k == j){
				perm[k] = GetClockIndex(i);
			}
			else {
				perm[k] = GetClockIndex(k);
			}
		}
		poly_t *permPoly = permute_dimensions(poly,perm);
		poly_free(poly);
		poly = permPoly;
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

	/* checks if the polyhedron is empty in the entire Eucledian space.
	 * As long as the algorithms preserve positive space this is fine.
	 *
	 * TODO
	 * Problem arises if:
	 * Intersect(x_i < c) where c <= 0
	 * Intersect(x_i <= c) where c < 0
	 * Reset(x_i = c) where c < 0
	 */
	bool TPlibMPPMarking::IsEmpty() const{
		return is_bottom(poly);
	}

	void TPlibMPPMarking::Delay(){
		poly_t *delayPoly = delay(poly,0);
		poly_free(poly);
		poly = delayPoly;
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
		/*TODO*//*ignoring this function until implemented in the TPlib */
	}

	size_t TPlibMPPMarking::HashKey() const{
		return VerifyTAPN::hash()(dp);
	}

	relation TPlibMPPMarking::Relation(const StoredMarking& other) const{
		const TPlibMPPMarking &mpp = static_cast<const TPlibMPPMarking&>(other);
		bool sub = is_leq(poly,mpp.poly);
		bool sup = is_leq(mpp.poly,poly);
		if(sup&&sup){
			return EQUAL;
		}
		if(sup){
			return SUPERSET;
		}
		if(sub){
			return SUBSET;
		}
		return DIFFERENT;
	}

	void TPlibMPPMarking::AddTokens(const std::list<int>& placeIndices){
		/*TODO*/
	}

	void TPlibMPPMarking::RemoveTokens(const std::set<int>& tokenIndices){
		int remDims[tokenIndices.size()];
		int i = 0;
		for(std::set<int>::const_iterator it = tokenIndices.begin(); it != tokenIndices.end(); ++i){
			remDims[i] = GetClockIndex(*it);
			i++;
		}
		poly_t *remPoly = remove_dimensions(poly,remDims,tokenIndices.size());
		poly_free(poly);
		poly = remPoly;
	}

}
