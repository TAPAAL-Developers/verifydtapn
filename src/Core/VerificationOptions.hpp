#ifndef VERIFICATIONOPTIONS_HPP_
#define VERIFICATIONOPTIONS_HPP_


#include <string>


namespace VerifyTAPN {
	class VerificationOptions {
		private:
			VerificationOptions(std::string inputFile, std::string queryFile, int k_bound, bool symmetry, bool useInfinityPlaces, bool useGlobalMaxConstants) : inputFile(inputFile), queryFile(queryFile), k_bound(k_bound), symmetry(symmetry), useInfinityPlaces(useInfinityPlaces), useGlobalMaxConstants(useGlobalMaxConstants) {};

		public: // static
			static VerificationOptions ParseVerificationOptions(int argc, char* argv[]);

		public: // inspectors
			const std::string GetInputFile() const { return inputFile; }
			const std::string QueryFile() const {return queryFile; }
			inline const int GetKBound() const { return k_bound; }
			inline const bool GetSymmetryEnabled() const { return symmetry; }
			inline const bool GetInfinityPlacesEnabled() const { return useInfinityPlaces; }
			inline const bool GetGlobalMaxConstantsEnabled() const { return useGlobalMaxConstants; }
		private:
			std::string inputFile;
			std::string queryFile;
			int k_bound;
			bool symmetry;
			bool useInfinityPlaces;
			bool useGlobalMaxConstants;
	};

}

#endif /* VERIFICATIONOPTIONS_HPP_ */