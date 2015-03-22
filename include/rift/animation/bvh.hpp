#ifndef BVH_HPP
#define BVH_HPP

#include <memory>
#include "skeleton.hpp"

void readHierarchy(std::ifstream &inputfile,
	Skeleton &skel, std::vector<Mapping> &mappings);
void readJoint(std::ifstream &inputfile, Skeleton& parent,std::vector<Mapping> &mappings);
void readMotion(std::ifstream &inputfile, Skeleton& root);
void readKeyFrame(std::ifstream &inputfile, Skeleton& skel);
void defineRotateOrder(Skeleton &skel);

//void printSkeleton(Skeleton & skel);


#endif