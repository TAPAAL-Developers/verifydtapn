#include <iostream>
#include "Core/TAPN/TAPN.hpp"
#include "boost/smart_ptr.hpp"
#include "Core/TAPNParser/TAPNXmlParser.hpp"
#include "Core/VerificationOptions.hpp"
#include "Core/QueryParser/AST.hpp"
#include "Core/QueryParser/TAPNQueryParser.hpp"
#include "ReachabilityChecker/SearchStrategy.hpp"

#include "Core/SymbolicMarking/UppaalDBMMarkingFactory.hpp"
#include "Core/SymbolicMarking/DiscreteInclusionMarkingFactory.hpp"
#include "Core/SymbolicMarking/MPPMarkingFactory.hpp"
#include "Core/SymbolicMarking/MPVector.hpp"

#include "ReachabilityChecker/Trace/trace_exception.hpp"

double elapsedTime;
#ifdef __MINGW32__
#include <windows.h>
LARGE_INTEGER frequency, t1, t2;
#define START_TIME QueryPerformanceFrequency(&frequency); QueryPerformanceCounter(&t1);
#define END_TIME QueryPerformanceCounter(&t2); elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
#else
#include <sys/time.h>
timeval t1, t2;
double elapsedTime;
#define START_TIME gettimeofday(&t1, NULL);
#define END_TIME gettimeofday(&t2, NULL); elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;
#endif

using namespace std;
using namespace VerifyTAPN;
using namespace VerifyTAPN::TAPN;
using namespace boost;

MarkingFactory* CreateFactory(const VerificationOptions& options, const boost::shared_ptr<TAPN::TimedArcPetriNet>& tapn)
{
	switch(options.GetFactory())
	{
	case DISCRETE_INCLUSION:
		return new DiscreteInclusionMarkingFactory(tapn);
	case MAXPLUS:
		return new MPPMarkingFactory<MPVector>(tapn);
	case MAXPLUS_COW:
		return new MPPMarkingFactory<MPVectorCOW>(tapn);
	default:
		return new UppaalDBMMarkingFactory(tapn);
	}
}

int main(int argc, char* argv[])
{
	VerificationOptions options = VerificationOptions::ParseVerificationOptions(argc, argv);
	DiscreteMarking::debug = options.UseDebugOutput();

	TAPNXmlParser modelParser;
	boost::shared_ptr<TAPN::TimedArcPetriNet> tapn;

	try{
		tapn = modelParser.Parse(options.GetInputFile());
	}catch(const std::string& e){
		std::cout << "There was an error parsing the model file: " << e << std::endl;
		return 1;
	}

	tapn->Initialize(options.GetUntimedPlacesEnabled());
	MarkingFactory* factory = CreateFactory(options, tapn);

	std::vector<int> initialPlacement(modelParser.ParseMarking(options.GetInputFile(), *tapn));
	SymbolicMarking* initialMarking(factory->InitialMarking(initialPlacement));
	if(initialMarking->NumberOfTokens() > options.GetKBound())
	{
		std::cout << "The specified k-bound is less than the number of tokens in the initial markings.";
		return 1;
	}

	AST::Query* query;
	try{
		TAPNQueryParser queryParser(*tapn);
		queryParser.parse(options.QueryFile());
		query = queryParser.GetAST();
	}catch(...){
		std::cout << "There was an error parsing the query file." << std::endl;
		return 1;
	}
	SearchStrategy* strategy = new DefaultSearchStrategy(*tapn, initialMarking, query, options, factory);

	START_TIME;
	bool result = strategy->Verify();
	END_TIME;
	std::cout << strategy->GetStats() << std::endl;
	std::cout << "Query is " << (result ? "satisfied" : "NOT satisfied") << ". Time spent: " << elapsedTime << "ms." << std::endl;

	try{
		strategy->PrintTraceIfAny(result);
	}catch(const trace_exception& e){
		std::cout << "There was an error generating a trace. This is a bug. Please report this on launchpad and attach your TAPN model and this error message: ";
		std::cout << e.what() << std::endl;
		return 1;
	}
	delete strategy;
	delete factory;

	return 0;
}


