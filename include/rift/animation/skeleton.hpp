#ifndef SKELETON_HPP
#define SKELETON_HPP

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct CurrentTransform{
	glm::vec3 current_translation;
	glm::vec3 current_rotation;
};

enum DofMapping{
	TRANSX, TRANSY, TRANSZ, ROTX, ROTY, ROTZ
};

struct Mapping{
	int num_joint;
	DofMapping dof_mapping;
	Mapping(int num_joint_, DofMapping dof_mapping_){
		this->num_joint = num_joint_;
		this->dof_mapping = dof_mapping_;
	}
};

struct Joint{
	std::string name;
	glm::vec3 init_offset;

	int parent;
	std::vector<int> children;

	//-------Constructors
	Joint(){
		parent = -1;
		init_offset = glm::vec3();
	}
	Joint(std::string _name){
		parent = -1;
		name = _name;
		init_offset = glm::vec3();
	}
};

class Skeleton
{
public:
	std::vector<Joint> joints;

	static std::unique_ptr<Skeleton> createFromFile(std::string filename, std::vector<Mapping> &mappings);
};


#endif