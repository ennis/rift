#include "skeleton.hpp"
#include <iostream>
#include <fstream>
#include <log.hpp>
#include <bvh.hpp>


std::unique_ptr<Skeleton> Skeleton::createFromFile(std::string filename, std::vector<Mapping> &mappings)
{
	auto root = std::make_unique<Skeleton>();

	LOG << "Loading skeleton and animation from " << filename;

	std::ifstream inputfile(filename.c_str());
	if (inputfile.good()) {
		while (!inputfile.eof()) {
			std::string buf;
			inputfile >> buf;
			if (!buf.compare("HIERARCHY")) {
				readHierarchy(inputfile, *root, mappings);
			}
		}
		inputfile.close();
	}
	else {
		ERROR << "Failed to load the file " << filename.c_str();
		//fflush(stdout);
	}

	LOG << "file loaded" ;

	return root;
}