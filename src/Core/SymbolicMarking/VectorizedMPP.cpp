/*
 * VectorizedMPP.cpp
 *
 *  Created on: Jun 26, 2012
 *      Author: ravn
 */

#include "VectorizedMPP.hpp"

namespace VerifyTAPN {
	void VectorizedMPP::InitZero(int numberOfTokens) {
		n = numberOfTokens + 1;
		gens = 1;
		G = std::vector<int>(n, 0);
	}

	/* Function to calculate whether 1 arbitrary point is contained in (can be
	 * generated by) the polyhedron
	 * x is the vector containing the point
	 * skipGen is used when calculating whether a generator of a polyhedron is redundant. In this case the index of the concerned generator
	 * is passed to indicate that it is not a part of the polyhedron without it
	 * skipGen defaults to -1 which does not macth any generator index
	 */
	bool VectorizedMPP::ContainsPoint(const std::vector<int>& x, int skipGen) const {
		std::vector<int> y = std::vector<int>(gens, INF);
		std::vector<int> z = std::vector<int>(n, INT_MIN);

		for (unsigned int i = 0; i < gens; i++) {
			if (i != skipGen) {
				for (unsigned int j = 0; j < n; j++) {
					if (G.at(i * n + j) == INT_MIN) {
						// y.at(i) does not change since we get MIN(y_val, x_val - -INF) => MIN(y_val, INF)
					} else if (x.at(j) == INT_MIN) {
						y.at(i) = INT_MIN;
					} else {
						y.at(i) = MIN(y.at(i), x.at(j) - G.at(i*n+j));
					}
				}
			}
		}
		for (unsigned int j = 0; j < n; j++) {
			for (unsigned int i = 0; i < gens; i++) {
				if (i != skipGen) {
					if (y.at(i) >= INF || G.at(i * n + j) >= INF || z.at(j) >= INF) {
						//the generator x we check for containment cannot have the value INF in any dimension
					} else if (y.at(i) == INT_MIN || G.at(i * n + j) == INT_MIN) {
						//z.at(j) should not change since we're interested in MAX and the result of y_val + G_val will be -INFINITY
						//and z is for all coordinates initialized to be INT_MIN
					} else {
						z.at(j) = MAX(z.at(j), y.at(i)+G.at(i*n+j));
					}
				}
			}
			if (x.at(j) != z.at(j)) {
				return false;
			}
		}
		return true;
	}

	/* Function to determine whether a given polyhedron is contained in this
	 *
	 */
	bool VectorizedMPP::Contains(const VectorizedMPP& mpp) const {
		std::vector<int> x = std::vector<int>(n);
		for (unsigned int i = 0; i < mpp.gens; i++) {
			std::memcpy(&x.at(0), &mpp.G.at(i * n), sizeof(int) * n); //extracting one generator from mpp copying it to x
			//for (unsigned int j = 0; j < n; j++) {
			//	x.at(j) = mpp.G.at(i * mpp.gens + j);
			//}

			if (!ContainsPoint(x)) {
				return false;
			}
		}
		return true;
	}

	/* Function to intersect this with a half-space
	 * a max-plus half-space is here used to represent a difference constraint
	 * a and b are generators defining the half-space
	 */
	void VectorizedMPP::IntersectHalfspace(std::vector<int>& a, std::vector<int>& b) {
		std::vector<int> Ggt; //"set" G^greather_than
		Ggt.reserve(G.size() - n); //reserve space to potentially hold all generators of G but one. In case all generators goes to Ggt the intersection is empty
		int GgtGens = 0; // number of generators in Ggt

		for (unsigned int i = 0; i < gens; i++) {
			int ag = INT_MIN; //to contain max-plus dot products (a * G^i)
			int bg = INT_MIN; //and (b * G^i)
			for (unsigned int j = 0; j < n; j++) {
				if (a.at(j) == INT_MIN || G.at(i * n + j) == INT_MIN) {
					//ag is either itself or INT_MIN which it was initialized as
					//a_j + G^i_j = INT_MIN in either case
				} else {
					ag = MAX(ag, a.at(j) + G.at(i*n+j));
				}
				if (b.at(j) == INT_MIN || G.at(i * n + j) == INT_MIN) {
					//same as above (ag) for bg
				} else {
					bg = MAX(bg, b.at(j) + G.at(i*n+j));
				}
			}
			if (ag > bg) {
				Ggt.resize(Ggt.size() + n); //resizing Ggt to have space for another generator
				std::memcpy(&Ggt.at(GgtGens * n), &G.at(i * n), sizeof(int) * n); //copying generator to Ggt "set"
				GgtGens++;
				if (i < gens - 1) {
					std::memcpy(&G.at(i * n), &G.at((gens - 1) * n), sizeof(int) * n); //moving last generator to empty space just created from removing a generator
					i--; //loop need to handle just moved generator
				}
				gens--;
				G.resize(gens * n); //resizing g to drop last generator which was just moved (might not be necessary as we keep count of generators and do a additional resize just below)
			}
		}
		if (gens == 0 || GgtGens == 0) {
			return;
		}

		int GleqGens = gens;
		G.resize(gens * n + GleqGens * GgtGens * n); //resize to fit the extra generators to be added

		for (int g = 0; g < GleqGens; g++) {
			for (int h = 0; h < GgtGens; h++) {
				std::vector<int> generator = std::vector<int>(n, INT_MIN);
				int ah = INT_MIN; // to hold dotproduct of a and h
				int bg = INT_MIN; // dot product for b and g
				for (unsigned int j = 0; j < n; j++) {
					if (a.at(j) == INT_MIN || Ggt.at(h * n + j) == INT_MIN) {
						//nothing, result = INT_MIN
					} else {
						ah = MAX(ah, a.at(j)+Ggt.at(h*n+j));
					}
					if (b.at(j) == INT_MIN || G.at(g * n + j) == INT_MIN) {
						//nothing, result = INT_MIN
					} else {
						bg = MAX(bg, b.at(j)+G.at(g*n+j));
					}
				}
				for (unsigned int j = 0; j < n; j++) {
					if (ah == INT_MIN || G.at(g * n + j) == INT_MIN) {
						if (bg == INT_MIN || Ggt.at(h * n + j) == INT_MIN) {
							//generator.at(j) = INT_MIN which it already is initialised to
						} else {
							generator.at(j) = bg + Ggt.at(h * n + j);
						}
					} else if (bg == INT_MIN || Ggt.at(h * n + j) == INT_MIN) {
						generator.at(j) = ah + G.at(g * n + j);
					} else {
						generator.at(j) = MAX(ah+G.at(g*n+j), bg+Ggt.at(h*n+j));
					}
				}
				std::memcpy(&G.at(gens * n), &generator.at(0), sizeof(int) * n); //copying newly created generator into the end of G
				gens++;
			}
		}
	}

	/*
	 * function to remove redundant generators
	 */
	void VectorizedMPP::Cleanup() {
		//	std::cout<<"Gens before cleanup: " << gens;
		std::vector<int> g = std::vector<int>(n);
		for (int i = gens - 1; i >= 0; --i) {
			std::memcpy(&g.at(0), &G.at(i * n), sizeof(int) * n); //extracting a generator
			if (ContainsPoint(g, i)) { //passing i to reflect the given generator should be ignored in the containment calculation
				if (i < gens - 1) { //checking whether the given generator is the last one or not. If not the last one has to be moved to keep data compact
					std::memcpy(&G.at(i * n), &G.at((gens - 1) * n), sizeof(int) * n);
				}
				gens--;
				G.resize(gens * n);
			}
		}
		//	std::cout<<" - After cleanup: " << gens << std::endl;
	}

	/*
	 * upper bound between two clocks
	 * return: clock1 - clock2
	 */
	int VectorizedMPP::GetUpperDiffBound(int clock1, int clock2) const {
		int bound = INT_MIN;
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n + clock1) != INT_MIN && G.at(i * n + clock2) != INT_MIN) {
				bound = MAX(bound, G.at(i*n+clock1)-G.at(i*n+clock2));
			} else if (G.at(i * n + clock1) != INT_MIN && G.at(i * n + clock2) == INT_MIN) {
				return INT_MAX;
			}
		}
		return bound;
	}

	/*
	 * output sensitive cleanup
	 * naive implementation - copying around a lot of data, which should be avoided if possible
	 * TODO - need to figure out a clever way to implement -- ALGORITHM NOT COMPLEATE
	 */
	void VectorizedMPP::CleanupOS() {
		std::vector<int> E = std::vector<int>(n); //extreme generators
		int lexmin = Lexmin(G, gens);
		unsigned int Egens = 1;
		memcpy(&E.at(0), &G.at(lexmin * n), sizeof(int) * n); //initializing E with lexicpgraphical minimum
		G.erase(G.begin() + lexmin * n, G.begin() + lexmin * n + n);
		gens--;
		std::vector<int> x = std::vector<int>(n); //generator to check inclusion for
		for (int i = gens - 1; i >= 0; i--) {
			std::memcpy(&x.at(0), &G.at(i * n), sizeof(int) * n);
			if (ExSetContainsPoint(E, Egens, x)) {
				G.erase(G.begin() + (i * n), G.begin() + (i * n) + n);
			} else {
				std::vector<int> y = std::vector<int>(n, INF); //vector holding (E\x)
				std::vector<int> satDim = std::vector<int>(n, 0);
				for (unsigned int e = 0; e < Egens; e++) {
					std::vector<int> sat;
					for (unsigned int j = 0; j < n; j++) {
						if (E.at(e * n + j) == INT_MIN) {
							// y.at(i) does not change since we get MIN(y_val, x_val - -INF) => MIN(y_val, INF)
						} else if (x.at(j) == INT_MIN) {
							if (y.at(e) != INT_MIN) {
								y.at(e) = INT_MIN;
								sat.clear();
							}
							sat.push_back(j);
						} else if (y.at(e) > x.at(j) - E.at(e * n + j)) {
							sat.clear();
							sat.push_back(j);
							y.at(e) = x.at(j) - E.at(e * n + j);
						} else if (y.at(e) == x.at(j) - E.at(e * n + j)) {
							sat.push_back(j);
						}
					}
					for (unsigned int s = 0; s < sat.size(); s++) {
						satDim.at(sat.at(s)) = 1;
					}
				}
				for (unsigned int j = 0; j < n; j++) {
					if (satDim.at(j)) {
						std::vector<int> Z;
						std::vector<int> Zgens;
						int minPsi = INF;
						for (unsigned int g = 0; g < gens; g++) {
							if (minPsi == INT_MIN) {

							}
						}
					}
				}
			}
		}
	}

	/*
	 * calculating ARGMAX_{z € P}(psi(z))
	 * where psi(x) = x_i - w_i - max_{j!=i}(x_j - w_j)
	 * i = dim
	 * Zgens is a vector containing the generator indexes of the generators satisfying the argmax
	 * TODO -- Algorithm is not finished!!
	 */
	std::vector<int> VectorizedMPP::ArgmaxPsi(const std::vector<int>& P, int Pgens, int dim, const std::vector<int> w,
			int skipgen) const {
		int maxPsi = INT_MIN;
		std::vector<int> Zgens;
		for (int i = 0; i < Pgens; i++) {
			if (i != skipgen) {
				int max = INT_MIN; // max_{j!=dim}(p_j - w_j)
				for (unsigned int j = 0; j < n; j++) {
					if (j != dim) {
						if (w.at(j) == INT_MIN) {
							max = INF;
						} else if (P.at(i * n + j) == INT_MIN) {

						} else {

						}
					}
				}
			}
		}
		return Zgens;
	}

	/*
	 * determines whether a point can be generated by a cone C given as a vector and the number of generators of C
	 */
	bool VectorizedMPP::ExSetContainsPoint(const std::vector<int>& C, int Cgens, const std::vector<int>& x) const {
		std::vector<int> y = std::vector<int>(gens, INF);
		std::vector<int> z = std::vector<int>(n, INT_MIN);

		for (unsigned int i = 0; i < Cgens; i++) {
			for (unsigned int j = 0; j < n; j++) {
				if (C.at(i * n + j) == INT_MIN) {
					// y.at(i) does not change since we get MIN(y_val, x_val -(-INF)) => MIN(y_val, INF)
				} else if (x.at(j) == INT_MIN) {
					y.at(i) = INT_MIN;
				} else {
					y.at(i) = MIN(y.at(i), x.at(j) - C.at(i*n+j));
				}
			}
		}
		for (unsigned int j = 0; j < n; j++) {
			for (unsigned int i = 0; i < Cgens; i++) {
				if (y.at(i) >= INF || C.at(i * n + j) >= INF || z.at(j) >= INF) {
					//the generator x we check for containment cannot have the value INF in any dimension
				} else if (y.at(i) == INT_MIN || C.at(i * n + j) == INT_MIN) {
					//z.at(j) should not change since we're interested in MAX and the result of y_val + G_val will be -INFINITY
					//and z is for all coordinates initialized to be INT_MIN
				} else {
					z.at(j) = MAX(z.at(j), y.at(i)+C.at(i*n+j));
				}
			}
			if (x.at(j) != z.at(j)) {
				return false;
			}
		}
		return true;
	}

	/*
	 * normalizing the generators, such that the first coordinate different from
	 * INT_MIN is 0
	 */
	void VectorizedMPP::Norm() {
		for (unsigned int i = 0; i < gens; i++) {
			int temp = INT_MIN;
			for (unsigned int j = 0; j < n; j++) {
				if (temp == INT_MIN && G.at(i * n + j) != INT_MIN) {
					temp = G.at(i * n + j);
				}
				if (temp != INT_MIN && G.at(i * n + j) != INT_MIN) {
					G.at(i * n + j) -= temp;
				}
			}
		}
	}

	/*
	 * returning the index of the Lexicographical minimal generator, to a specified dimension, of the cone C
	 * if no dimension is specified dim 0 is defeault
	 * dim: the dimension we want the lexmin over
	 * C: a vector containing the generators of the cone
	 * Cgens: the numbers of generators of C
	 */
	int VectorizedMPP::Lexmin(const std::vector<int>& C, unsigned int Cgens, unsigned int dim) const {
		std::vector<int> posMins;
		bool initValNegInf = false;
		for (unsigned int i = 0; i < Cgens; i++) {
			if (C.at(i * n + dim) != INT_MIN && !initValNegInf) {
				posMins.push_back(i);
			} else if (C.at(i * n + dim) == INT_MIN && !initValNegInf) {
				posMins.clear();
				posMins.push_back(i);
			} else if (C.at(i * n + dim) == INT_MIN && initValNegInf) {
				posMins.push_back(i);
			}
		}
		if (posMins.size() == 1) {
			return posMins.at(0);
		}
		for (unsigned int j = 0; j < n; j++) {
			if (j != dim) {
				if (initValNegInf) {
					int minVal;
					int normVal = 0;
					if (C.at(posMins.back() * n + j) == INT_MIN) {
						minVal = INT_MIN;
					} else {
						normVal = C.at(posMins.back() * n + j);
						minVal = 0;
					}
					for (int i = posMins.size() - 2; i >= 0; --i) {
						if (C.at(posMins.at(i) * n + j) == INT_MIN && minVal != INT_MIN) {
							minVal = INT_MIN;
							posMins.resize(i + 1);
						} else if (C.at(posMins.at(i) * n + j) != INT_MIN
								&& C.at(posMins.at(i) * n + j) - normVal < minVal) {
							minVal = C.at(posMins.at(i) * n + j) - normVal;
							posMins.resize(i + 1);
						} else if (C.at(posMins.at(i) * n + j) != INT_MIN
								&& C.at(posMins.at(i) * n + j) - normVal > minVal) {
							posMins.erase(posMins.begin() + j);
						}
					}
				} else {
					int minVal = C.at(posMins.back() * n + j) - C.at(posMins.back() * n + dim);
					for (int i = posMins.size() - 2; i >= 0; --i) {
						if (C.at(posMins.at(i) * n + j) - C.at(posMins.at(i) * n + dim) < minVal) {
							posMins.resize(i + 1);
							minVal = C.at(posMins.at(i) * n + j) - C.at(posMins.at(i) * n + dim);
						} else if (C.at(posMins.at(i) * n + j) - C.at(posMins.at(i) * n + dim) > minVal) {
							posMins.erase(posMins.begin() + j);
						}
					}
				}
				if (posMins.size() == 1) {
					return posMins.at(0);
				}
			}
		}
		std::cout
				<< "Something went horribly wrong. It should not have come to this. The algorithm detected no lexicographic minimal generator. So I'm returning -1 which will make the program crash."
				<< std::endl;
		return -1;
	}

	/*
	 * Function to reset a clock to 0.
	 * The function is also able to handle non normalised polyhedra by resetting to the 0-clock offset
	 * resetVal defaults to 0
	 */
	void VectorizedMPP::ResetClock(int clock, int resetVal) {
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n) == INT_MIN) {
				G.at(i * n + clock) = INT_MIN;
			} else {
				G.at(i * n + clock) = G.at(i * n) + resetVal;
			}
		}
//		Cleanup();
	}

	/*
	 * Frees up a clock from all constraints
	 * resetVal is passed when called fram Extrapolate to reset the polyhedron to maxBound+1 according to the extrapoltaion algorithm
	 * resetVal defaults to 0 for a regular Free.
	 */
	void VectorizedMPP::FreeClock(int clock, int resetVal) {
		ResetClock(clock, resetVal);
		AddUnitVec(clock);
	}

	/*
	 * swapping all internal values of two clocks
	 */
	void VectorizedMPP::SwapClocks(int clock1, int clock2) {
		for (unsigned int i = 0; i < gens; i++) {
			int temp = G.at(i * n + clock1);
			G.at(i * n + clock1) = G.at(i * n + clock2);
			G.at(i * n + clock2) = temp;
		}
	}

	void VectorizedMPP::Print(std::ostream& out) const {
		out << "VectorizedMPP:" << std::endl;
		out << "gens: " << gens << std::endl;
		out << "n: " << n << std::endl;
		out << "G:" << std::endl;
		for (unsigned int i = 0; i < gens; i++) {
			for (unsigned j = 0; j < n; j++) {
				if (j == 0) {
					out << "g" << i << "   ";
				}
				if (G.at(i * n + j) != INT_MIN) {
					out << G.at(i * n + j) << " ";
				} else {
					out << "-inf ";
				}
				if (j == n - 1) {
					out << std::endl;
				}
			}
		}
	}

	bool VectorizedMPP::IsEmpty() const {
#if DEBUG_PRINT
		std::cout << "IsEmpty!" << std::endl << "Marking:" << std::endl;
		Print(std::cout);
#endif
		if (gens <= 0 || G.size() == 0) {
#if DEBUG_PRINT
			std::cout << "is empty? " << true << std::endl;
			std::cout << "-----------------" << std::endl;
#endif
			return true;
		}
		for (unsigned int i = 0; i < gens; ++i) {
			if (G.at(i * n) != INT_MIN) {
#if DEBUG_PRINT
				std::cout << "is empty? " << false << std::endl;
				std::cout << "-----------------" << std::endl;
#endif
				return false;
			}
		}
#if DEBUG_PRINT
		Print(std::cout);
		std::cout << "all tests failed, returning empty\n";
		std::cout << "-----------------" << std::endl;
#endif
		return true;
	}

	void VectorizedMPP::Delay() {
		int addGens = 0;
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n) != INT_MIN) {
				G.resize(G.size() + n);
				addGens++;
				std::memcpy(&G.at((gens - 1 + addGens) * n), &G.at(i * n), sizeof(int) * n);
				G.at((gens - 1 + addGens) * n) = INT_MIN;
			}
		}
		gens += addGens;
	}

	void VectorizedMPP::Constrain(int clock, const TAPN::TimeInterval& interval) {
		if ((interval.IsLowerBoundStrict() && interval.GetLowerBound() >= 0)) {
			std::cout << "lowerbound: " << interval.GetLowerBound() << " - " << interval.IsLowerBoundStrict()
					<< std::endl;
			std::cout
					<< "Model includes strict constraint(s) which is currently not supported by max-plus polyhedra - might incur incorrect behaviour\n"
					<< std::endl;
		}
		if (interval.IsUpperBoundStrict() && (interval.GetUpperBound() != INT_MAX && interval.GetUpperBound() != INF)) {
			std::cout << "upperbound: " << interval.GetUpperBound() << " - " << interval.IsUpperBoundStrict()
					<< std::endl;
			std::cout
					<< "Model includes strict constraint(s) which is currently not supported by max-plus polyhedra - might incur incorrect behaviour\n"
					<< std::endl;
		}
		std::vector<int> a = std::vector<int>(n, INT_MIN);
		std::vector<int> b = std::vector<int>(n, INT_MIN);
		if (interval.GetUpperBound() != INT_MAX) {
			a.at(clock) = 0;
			b.at(0) = interval.GetUpperBound();
			IntersectHalfspace(a, b);
//			Cleanup();
		}
		if (interval.GetLowerBound() > 0) {
			a.at(clock) = INT_MIN;
			b.at(0) = INT_MIN;
			a.at(0) = 0;
			b.at(clock) = -interval.GetLowerBound();
			IntersectHalfspace(a, b);
//			Cleanup();
		}
	}

	void VectorizedMPP::Constrain(int clock, const TAPN::TimeInvariant& invariant) {
		if (invariant.GetBound() != INT_MAX && invariant.GetBound() != INF) {
			if (invariant.IsBoundStrict()) {
				std::cout
						<< "Model contains strictness which is not supported by Max-Plus Polyhedra.\nBound converted to non-strict equivalent which may incur incorrect behaviour.\n"
						<< std::endl;
			}
			std::vector<int> a = std::vector<int>(n, INT_MIN);
			std::vector<int> b = std::vector<int>(n, INT_MIN);
			a.at(clock) = 0;
			b.at(0) = invariant.GetBound();
			IntersectHalfspace(a, b);
		}
	}

	bool VectorizedMPP::PotentiallySatisfies(int clock, const TAPN::TimeInterval& interval) const {
		bool lowerSat = false, upperSat = false;
		if (interval.GetLowerBound() <= 0) {
			if (interval.IsLowerBoundStrict() && interval.GetLowerBound() == 0) {
				std::cout << "Strict bound might incur incorrect result\n";
			}
			lowerSat = true;
		}
		if (interval.GetUpperBound() >= INF) {
			upperSat = true;
		} else if (interval.IsUpperBoundStrict()) {
			std::cout << "Strict bound might incur incorrect result\n";
		}
		if (upperSat && lowerSat) {
			return true;
		}
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n) != INT_MIN && G.at(i * n + clock) != INT_MIN) { // second condition should never be able to be false given first is true
				lowerSat = lowerSat || G.at(i * n + clock) - G.at(i * n) >= interval.GetLowerBound();
				upperSat = upperSat || G.at(i * n + clock) - G.at(i * n) <= interval.GetUpperBound();
			} else if (G.at(i * n) == INT_MIN && G.at(i * n + clock) != INT_MIN) {
				lowerSat = true; //generator is delayed and thus always will satisfy a lower bound
			}
			if (upperSat && lowerSat) {
				return true;
			}
		}
		return false;
	}

	/*
	 * extrapolation - not complete but sound
	 * principles: freeing unbounded clocks, and theorems 4.9 and 4.11
	 */
	void VectorizedMPP::Extrapolate(const int* maxConstants) {
		for (unsigned int j = 1; j < n; ++j) {
			if (maxConstants[j] == -INF) {
				FreeClock(j);
			}/*else if (maxConstants[j] >= 0) {
			 bool addUnitVec = false; //should we add a new unit vector for the given dimension  (j)?
			 bool resetExtra = true; //is all generators above maxConstant[j] thus we should apply 4.9?
			 for (unsigned int i = 0; i < gens; i++) {
			 if (resetExtra) {
			 if (G.at(i * n) != INT_MIN && G.at(i * n + j) - G.at(i * n) <= maxConstants[j]) {
			 resetExtra = false;
			 }
			 if (i == gens - 1 && resetExtra) {
			 FreeClock(j, maxConstants[j] + 1);
			 break;
			 }
			 }
			 if (!addUnitVec && G.at(i * n + j) != INT_MIN) { //generator has an actual value for concerned dimension
			 bool unitVec = true; //is the given generator a unit vector?
			 for (unsigned int k = 1; k < n; k++) {
			 if (k != j && G.at(i * n + k) != INT_MIN) {
			 unitVec = false;
			 if (G.at(i * n + j) - G.at(i * n + k) <= maxConstants[j]) {
			 break;
			 }
			 }
			 if (k == n - 1 && !unitVec) {
			 addUnitVec = true;
			 AddUnitVec(j);
			 }
			 }
			 }
			 }
			 }*/
		}
		Cleanup();
		//ExtrapolateClaim(maxConstants);
		Extrapolate49(maxConstants);
		//Extrapolate49_2(maxConstants);
		Extrapolate411(maxConstants);
		//Extrapolate413(maxConstants);
	}

	void VectorizedMPP::ExtrapolateClaim(const int* maxConstants) {
		std::vector<int> topCorner = std::vector<int>(n, INT_MIN);
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n) != INT_MIN) {
				for (unsigned int j = 1; j < n; j++) {
					if (topCorner.at(j) != INT_MAX) {
						if (G.at(i * n + j) - G.at(i * n) > maxConstants[j]) {
							topCorner.at(j) = INT_MAX;
						} else {
							topCorner.at(j) = MAX(topCorner.at(j), G.at(i*n+j)-G.at(i*n));
						}
					}
				}
			} else {
				for (unsigned int j = 1; j < n; j++) {
					if (topCorner.at(j) != INT_MAX && G.at(i * n + j) != INT_MIN) {
						topCorner.at(j) = INT_MAX;
					}
				}
			}
		}
		unsigned int oldGens = gens;
		for (unsigned int i = 0; i < oldGens; i++) {
			for (unsigned int j = 1; j < n; j++) {
				if (G.at(i * n) != INT_MIN && G.at(i * n + j) - G.at(i * n) > maxConstants[j] && maxConstants[j] >= 0) {
					int minDiff = INT_MAX;
					for (unsigned int k = 1; k < n; k++) {
						if (k != j) {
							if (topCorner.at(j) != INT_MAX) {
								minDiff = MIN(minDiff, G.at(i*n+k)-G.at(i*n)-topCorner.at(k));
							}
						}
					}
					if (minDiff < 0) {
						std::cout << "what the heck!!" << std::endl;
					} else if (minDiff == INT_MAX) {
						//std::cout << "adding delay extra" << std::endl;
						G.resize(G.size() + n * sizeof(int));
						memcpy(&G.at(gens * n), &G.at(i * n), sizeof(int) * n);
						G.at(gens * n) = INT_MIN;
						gens++;
						for (unsigned int k = 1; k < n; k++) {
							topCorner.at(k) = INT_MAX;
						}
					} else {
						//std::cout << "adding modify extra" << std::endl;
						G.resize(G.size() + n * sizeof(int));
						memcpy(&G.at(gens * n), &G.at(i * n), sizeof(int) * n);
						for (unsigned int k = 1; k < n; k++) {
							G.at(gens * n + k) += minDiff;
							if (topCorner.at(k) != INT_MAX) {
								topCorner.at(k) = MAX(topCorner.at(k), G.at(gens*n+k)+minDiff);
							}
						}
						gens++;
					}
				}
			}
		}
		Cleanup();
	}

	void VectorizedMPP::Extrapolate411(const int* maxConstants) {
		unsigned int oldGens = gens;
		for (unsigned int j = 1; j < n; j++) {
			if (maxConstants[j] >= 0) {
				for (unsigned int i = 0; i < oldGens; i++) {
					if (G.at(i * n) != INT_MIN && G.at(i * n + j) - G.at(i * n) <= maxConstants[j]) {
						break;
					}
					if (i == oldGens - 1) {
						FreeClock(j, maxConstants[j] + 1);
					}
				}
			}
		}
		Cleanup();
	}

	void VectorizedMPP::Extrapolate49(const int* maxConstants) {
		for (unsigned int j = 1; j < n; j++) {
			if (maxConstants[j] >= 0) {
				bool addUnitVec = false;
				for (unsigned int i = 0; i < gens; i++) {
					if (!addUnitVec && G.at(i * n + j) != INT_MIN) { //generator has an actual value for concerned dimension
						bool unitVec = true;
						for (unsigned int k = 1; k < n; k++) {
							if (k != j && G.at(i * n + k) != INT_MIN) {
								unitVec = false;
								if (G.at(i * n + j) - G.at(i * n + k) <= maxConstants[j]) {
									break;
								}
							}
							if (k == n - 1 && !unitVec) {
								addUnitVec = true;
							}
						}
						if (addUnitVec) {
							AddUnitVec(j);
							break;
						}
					}
				}
			}
		}
		Cleanup();
	}

	void VectorizedMPP::Extrapolate49_2(const int* maxConstants) {
		//std::cout << "Extrapolate49_2" << std::endl;
		for (unsigned int j = 1; j < n; j++) {
			if (maxConstants[j] >= 0) {
				bool aboveDia = true;
				for (unsigned int k = 1; k < n; k++) {
					if (k != j) {
						for (unsigned int i = 0; i < gens; i++) {
							if (G.at(i * n + j) != INT_MIN) {
								if (G.at(i * n + k) == INT_MIN) {
									break;
								} else if (G.at(i * n + j) - G.at(i * n + k) > maxConstants[j]) {
									break;
								} else if (i == gens - 1) {
									aboveDia = false;
								}
							}
						}
						if(!aboveDia){
							break;
						}
					}
				}
				if (aboveDia) {
					//std::cout << "aboveDia - j = " << j << std::endl;
					AddUnitVec(j);
				}
			}
		}
		Cleanup();
	}

	void VectorizedMPP::Extrapolate413(const int* maxConstants) {
		for (unsigned int j = 1; j < n; j++) {
			if (maxConstants[j] >= 0) {
				break;
			}
			if (j == n - 1) {
				return;
			}
		}
		for (unsigned int i = 0; i < gens; i++) {
			if (G.at(i * n) != INT_MIN) {
				for (unsigned int j = 1; j < n; j++) {
					if (G.at(i * n + j) - G.at(i * n) <= maxConstants[j]) {
						break;
					}
					if (j == n - 1) {
						G.resize(G.size() + n);
						memcpy(&G.at(gens * n), &G.at(i * n), sizeof(int) * n);
						G.at(gens * n) = INT_MIN;
						gens++;
					}
				}
			}
		}
		Cleanup();
	}
	/*
	 * adds a unit vector for a specific dimension to the end of G.
	 * Unit vectors e are defined as:
	 * e_j = 0, j == dim
	 * e_j = NegInf, j!= dim
	 */
	void VectorizedMPP::AddUnitVec(unsigned int dim) {
		gens++;
		G.resize(gens * n);
		for (unsigned int j = 0; j < n; j++) {
			if (j == dim) {
				G.at((gens - 1) * n + j) = 0;
			} else {
				G.at((gens - 1) * n + j) = INT_MIN;
			}
		}
	}

	void VectorizedMPP::ConvexHullUnion(VectorizedMPP* mpp) {
		G.resize(G.size() + mpp->G.size());
		memcpy(&G.at(n * gens), &mpp->G.at(0), sizeof(int) * n * mpp->gens);
		gens += mpp->gens;
	}

	relation VectorizedMPP::Relation(const VectorizedMPP& other) const {
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "VectorizedMPP::Relation" << std::endl << "relating..." << std::endl;
#endif
		bool sup = Contains(other);
		bool sub = other.Contains(*this);
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "Contains(other) = " << sup << " and other.Contains(*this) = " << sub << std::endl;
#endif
		if (sub && sup) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "relation = EQUAL" << std::endl;
#endif
			return EQUAL;
		}
		if (sub) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "relation = SUBSET" << std::endl;
#endif
			return SUBSET;
		}
		if (sup) {
#if DEBUG_PRINT || DEBUG_RELATION
			std::cout << "relation = SUPERSET" << std::endl;
#endif
			return SUPERSET;
		}
#if DEBUG_PRINT || DEBUG_RELATION
		std::cout << "relation = DIFFERENT" << std::endl;
#endif
		return DIFFERENT;
	}

	void VectorizedMPP::AddClock() {
		for (int i = gens - 1; i >= 0; i--) {
			G.insert(G.begin() + i * n + n, G.at(i * n));
		}
		n++;
	}

	void VectorizedMPP::RemoveClocks(const std::vector<unsigned int> removeClocks) {
		for (unsigned int j = 0; j < removeClocks.size(); j++) {
			for (int i = gens - 1; i >= 0; i--) {
				G.erase(G.begin() + n * i + removeClocks.at(j));
			}
			n--;
		}
	}
}

