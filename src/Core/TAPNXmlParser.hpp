#ifndef VERIFYTAPN_TAPNXMLPARSER_HPP_
#define VERIFYTAPN_TAPNXMLPARSER_HPP_

#include "../TAPN/TAPN.hpp"
#include "boost/smart_ptr.hpp"
#include "../../lib/rapidxml-1.13/rapidxml.hpp"

namespace VerifyTAPN {
	using namespace VerifyTAPN::TAPN;

	class TAPNXmlParser
	{
	private:
		struct ArcCollections{
			boost::shared_ptr<TimedInputArc::Vector> inputArcs;
			boost::shared_ptr<OutputArc::Vector> outputArcs;

		public:
			ArcCollections(const boost::shared_ptr<TimedInputArc::Vector>& inputArcs, const boost::shared_ptr<OutputArc::Vector>& outputArcs) : inputArcs(inputArcs), outputArcs(outputArcs) {};
		};
	public: // construction
		TAPNXmlParser() {};
		virtual ~TAPNXmlParser() { /* empty */ };

	public:
		boost::shared_ptr<TimedArcPetriNet> Parse(const std::string & filename) const;
	private:
		boost::shared_ptr<TimedArcPetriNet> ParseTAPN(const rapidxml::xml_node<> & root) const;
		const std::string ReadFile(const std::string & filename) const;

		void ParsePlaces(const rapidxml::xml_node<>& root, TimedArcPetriNet& tapn) const;
		TimedPlace* ParsePlace(const rapidxml::xml_node<>& placeNode) const;

		void ParseTransitions(const rapidxml::xml_node<>& root, TimedArcPetriNet& tapn) const;
		TimedTransition* ParseTransition(const rapidxml::xml_node<>& transitionNode) const;

		void ParseArcs(const rapidxml::xml_node<>& root, TimedArcPetriNet& tapn) const;
		TimedInputArc* ParseInputArc(const rapidxml::xml_node<>& arcNode, TimedArcPetriNet& tapn) const;
		OutputArc* ParseOutputArc(const rapidxml::xml_node<>& arcNode, TimedArcPetriNet& tapn) const;
	};
}

#endif /* VERIFYTAPN_TAPNXMLPARSER_HPP_ */
