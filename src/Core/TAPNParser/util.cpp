#include "util.hpp"
#include <fstream>

namespace VerifyTAPN
{
	const std::string ReadFile(const std::string& filename)
	{
		std::ifstream file (filename.c_str());
		if(!file){
			throw new std::exception();
		}

		std::string storage((std::istreambuf_iterator<char>(file)),
							 std::istreambuf_iterator<char>());


//		std::string contents;
//		std::string line;
//		while(std::getline(file, line)){
//			contents += line;
//		}
		return storage;
	};
}
