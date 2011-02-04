#ifndef VERIFYTAPN_TAPNXMLPARSER_HPP_
#define VERIFYTAPN_TAPNXMLPARSER_HPP_

#include "../TAPN/TAPN.hpp"
#include "boost/smart_ptr.hpp"
#include "../SymbolicMarking/SymMarking.hpp"
#include "../../../lib/rapidxml-1.13/rapidxml.hpp"

namespace VerifyTAPN {
	using namespace VerifyTAPN::TAPN;

	class TAPNXmlParser
	{
	private:
		struct ArcCollections{
			TimedInputArc::Vector inputArcs;
			OutputArc::Vector outputArcs;

		public:
			ArcCollections(const TimedInputArc::Vector& inputArcs, const OutputArc::Vector& outputArcs) : inputArcs(inputArcs), outputArcs(outputArcs) {};
		};
	public: // construction
		TAPNXmlParser() {};
		virtual ~TAPNXmlParser() { /* empty */ };

	public:
		boost::shared_ptr<TimedArcPetriNet> Parse(const std::string & filename) const;
		SymMarking* ParseMarking(const std::string & filename, const TimedArcPetriNet& tapn) const;
	private:
		boost::shared_ptr<TimedArcPetriNet> ParseTAPN(const rapidxml::xml_node<> & root) const;

		TimedPlace::Vector ParsePlaces(const rapidxml::xml_node<>& root) const;
		boost::shared_ptr<TimedPlace> ParsePlace(const rapidxml::xml_node<>& placeNode) const;

		TimedTransition::Vector ParseTransitions(const rapidxml::xml_node<>& root) const;
		boost::shared_ptr<TimedTransition> ParseTransition(const rapidxml::xml_node<>& transitionNode) const;

		ArcCollections ParseArcs(const rapidxml::xml_node<>& root, const TimedPlace::Vector& places, const TimedTransition::Vector& transitions) const;
		boost::shared_ptr<TimedInputArc> ParseInputArc(const rapidxml::xml_node<>& arcNode, const TimedPlace::Vector& places, const TimedTransition::Vector& transitions) const;
		boost::shared_ptr<OutputArc> ParseOutputArc(const rapidxml::xml_node<>& arcNode, const TimedPlace::Vector& places, const TimedTransition::Vector& transitions) const;
		SymMarking* ParseInitialMarking(const rapidxml::xml_node<>& root, const TimedArcPetriNet& tapn) const;
	};
}

#endif /* VERIFYTAPN_TAPNXMLPARSER_HPP_ */
