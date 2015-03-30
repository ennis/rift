#ifndef SKELETON_HPP
#define SKELETON_HPP

#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

struct CurrentTransform {
	glm::vec3 current_translation;
	glm::vec3 current_rotation;
};

enum class DOF
{
	TranslationX,
	TranslationY,
	TranslationZ,
	RotationX,
	RotationY,
	RotationZ
};

struct BVHMapping
{
	DOF dof;
	int joint_index;

	BVHMapping() = default;
	BVHMapping(int joint_index_, DOF dof_) :
		dof(dof_),
		joint_index(joint_index_)
	{}
};

struct Joint
{
	//-------Constructors
	Joint() = default;

	Joint(std::string name_) :
		name(std::move(name_)),
		parent(-1)
	{
	}

	std::string name = "root";
	glm::vec3 init_offset;
	int parent = -1;
	// TODO smallvector?
	std::vector<int> children;
};

class Skeleton
{
public:
	using Ptr = std::unique_ptr<Skeleton>;

	static Ptr loadFromBVH(
		std::istream &streamIn, 
		std::vector<BVHMapping> &bvhMappings);

	std::vector<Joint> joints;
};


#endif