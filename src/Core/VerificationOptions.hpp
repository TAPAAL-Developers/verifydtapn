#ifndef VERIFICATIONOPTIONS_HPP_

#define VERIFICATIONOPTIONS_HPP_


#include <string>


namespace VerifyTAPN {
	enum Trace { NONE, SOME };
	enum SearchType { BREADTHFIRST, DEPTHFIRST };
	enum Factory { DEFAULT, DISCRETE_INCLUSION, MAXPLUS, MAXPLUS_COW, LOAD_ONLY };

	class VerificationOptions {
		private:
			VerificationOptions(
				const std::string& inputFile,
				const std::string& queryFile,
				SearchType searchType,
				unsigned int k_bound,
				bool symmetry,
				Trace trace,
				bool useUntimedPlaces,
				bool useGlobalMaxConstants,
				Factory factory,
				bool debugOutput,
				bool overApproximate
			) :
				inputFile(inputFile),
				queryFile(queryFile),
				searchType(searchType),
				k_bound(k_bound),
				symmetry(symmetry),
				trace(trace),
				useUntimedPlaces(useUntimedPlaces),
				useGlobalMaxConstants(useGlobalMaxConstants),
				factory(factory),
				debugOutput(debugOutput),
				overApproximate(overApproximate)
			{};

		public: // static
			static VerificationOptions ParseVerificationOptions(int argc, char* argv[]);

		public: // inspectors
			const std::string GetInputFile() const { return inputFile; }
			const std::string QueryFile() const {return queryFile; }
			inline const unsigned int GetKBound() const { return k_bound; }
			inline const Trace GetTrace() const { return trace; };
			inline const bool GetSymmetryEnabled() const { return symmetry; }
			inline const bool GetUntimedPlacesEnabled() const { return useUntimedPlaces; }
			inline const bool GetGlobalMaxConstantsEnabled() const { return useGlobalMaxConstants; }
			inline const SearchType GetSearchType() const { return searchType; }
			inline Factory GetFactory() const { return factory; };
			inline const bool UseDebugOutput() const { return debugOutput; }
			inline const bool UseOverApprox() const { return overApproximate;	}
		private:
			std::string inputFile;
			std::string queryFile;
			SearchType searchType;
			unsigned int k_bound;
			bool symmetry;
			Trace trace;
			bool useUntimedPlaces;
			bool useGlobalMaxConstants;
			Factory factory;
			bool debugOutput;
			bool overApproximate;
	};

}

#endif /* VERIFICATIONOPTIONS_HPP_ */
