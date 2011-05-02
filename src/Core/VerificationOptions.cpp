#include "VerificationOptions.hpp"
#include <iostream>
#include "boost/program_options.hpp"
//#include "boost/filesystem.hpp"

namespace VerifyTAPN {
	std::string enumToString(Trace trace){
		switch(trace)
		{
		case SOME:
			return "some";
		default:
			return "no";
		}
	}

	Trace intToEnum(int i){
		switch(i){
		case 1:
			return SOME;
		default:
			return NONE;
		}
	}

	std::string SearchTypeEnumToString(SearchType s){
		switch(s){
		case DEPTHFIRST:
			return "Depth-First Search";
		default:
			return "Breadth-First Search";
		}
	}

	SearchType intToSearchTypeEnum(int i) {
		switch(i){
		case 1:
			return DEPTHFIRST;
		default:
			return BREADTHFIRST;
		}
	}

	Factory intToFactory(unsigned int i) {
		switch(i)
		{
		case 1:
			return DISCRETE_INCLUSION;
		case 2:
			return MAXPLUS;
		default:
			return DEFAULT;
		}
	}

	std::string FactoryEnumToString(Factory factory){
		switch(factory)
		{
		case DISCRETE_INCLUSION:
			return "discrete inclusion";
		case MAXPLUS:
			return "max-plus polyhedra";
		case DEFAULT:
		default:
			return "default";
		}
	}

	VerificationOptions VerificationOptions::ParseVerificationOptions(int argc, char* argv[])
	{
		bool error = false;

		boost::program_options::options_description desc("Usage: verifytapn -k <number> [-t <number>] model-file query-file \nAllowed Options:");
		desc.add_options()
				("help,h", "Produce help message")
				("k-bound,k", boost::program_options::value<unsigned int>(), "Specify the bound of the TAPN model")
				("search-type,o", boost::program_options::value<unsigned int>()->default_value(0), "Specify the desired search strategy. \n - 0: BFS\n - 1: DFS" )
				("trace,t", boost::program_options::value<unsigned int>()->default_value(0), "Specify the desired trace option. \n - 0: none\n - 1: Some")
				("global-max-constant,g", "Use a global max constant for extrapolation (as opposed to local constants)")
				("untimed-places,u", "Use the untimed place optimization")
				("symmetry,s", "Use symmetry reduction")
				("factory,f", boost::program_options::value<unsigned int>()->default_value(0), "Specify marking factory.\n - 0: default\n - 1: discrete-inclusion\n - 2: max-plus polyhedra")
				("model-file", boost::program_options::value<std::string>(), "model file")
				("query-file", boost::program_options::value<std::string>(), "query file")
				("debug-output,d", "Print debug output (debug builds only)")
				("overapproximate,a", "Use overapproximation.")
		;

		boost::program_options::positional_options_description p;
		p.add("model-file", 1);
		p.add("query-file", 2);

		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::command_line_parser(argc,argv).options(desc).positional(p).run(), vm);
		boost::program_options::notify(vm);

		if(argc == 1 || vm.count("help")) {
			std::cout << desc << "\n";
			exit(0);
		}

		SearchType search = intToSearchTypeEnum(vm["search-type"].as<unsigned int>());
		std::cout << "Using " << SearchTypeEnumToString(search) << "\n";

		if(vm.count("k-bound")) {
			std::cout << "k-bound is: " << vm["k-bound"].as<unsigned int>() << "\n";
		}
		else {
			std::cout << "Error! You must specify a bound k on the TAPN model" << "\n\n";
			error = true;
		}

		Trace trace = intToEnum(vm["trace"].as<unsigned int>());
		std::cout << "Generating " << enumToString(trace) << " trace \n";



		bool symmetry = false;
		if(vm.count("symmetry")) {
			std::cout << "Symmetry Reduction is ON\n";
			symmetry = true;
		}
		else
		{
			std::cout << "Symmetry Reduction is OFF\n";
		}

		bool untimedPlaces = false;
		if(vm.count("untimed-places")) {
			std::cout << "Untimed place optimization is ON\n";
			untimedPlaces = true;
		}
		else
		{
			std::cout << "Untimed place optimization is OFF\n";
		}

		bool globalConstants = false;
		if(vm.count("global-max-constant"))
		{
			std::cout << "Using global maximum constant for extrapolation\n";
			globalConstants = true;
		}
		else
		{
			std::cout << "Using local maximum constants for extrapolation\n";
			globalConstants = false;
		}

		Factory factory = intToFactory(vm["factory"].as<unsigned int>());
		std::cout << "Using " << FactoryEnumToString(factory) << " marking factory" << std::endl;

		if(vm.count("model-file")) {
			std::cout << "model file is: " << vm["model-file"].as<std::string>() << "\n";
		}
		else{
			std::cout << "Error! You must specify an model file" << "\n\n";
			error = true;
		}

		if(vm.count("query-file")) {
			std::cout << "query file is: " << vm["query-file"].as<std::string>() << "\n";
		}
		else{
			std::cout << "Error! You must specify an query file" << "\n\n";
			error = true;
		}

		bool debugOutput = false;
		if (vm.count("debug-output"))
			debugOutput = true;

		bool overApproximate = false;
		if (vm.count("overapproximate")) {
			std::cout << "Using overapproximation\n";
			overApproximate = true;
		}

		if(error) {
			std::cout << desc << "\n";
			exit(0);
		}

		return VerificationOptions(vm["model-file"].as<std::string>(), vm["query-file"].as<std::string>(), search, vm["k-bound"].as<unsigned int>(), symmetry, trace, untimedPlaces, globalConstants, factory, debugOutput, overApproximate);
	}
}
