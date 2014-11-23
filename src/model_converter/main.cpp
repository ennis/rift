#include "model.hpp"
#include <fstream>

int main(int argc, const char **argv)
{
	assert(argc > 1);
	auto fileName = argv[1];
	Importer::Model model(fileName);
	std::ofstream streamOut(std::string(fileName) + ".mesh", std::ios::out | std::ios::binary);
	model.exports(streamOut);
}
