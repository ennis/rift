#ifndef POSE_HPP
#define POSE_HPP

#include <common.hpp>
#include <vector>

class Pose
{
public:
	Pose()
	{}

	Pose(std::vector<glm::vec3> &&positions, std::vector<glm::quat> &&rotations) :
		mPositions(positions),
		mRotations(rotations)
	{
		assert(mPositions.size() == mRotations.size());
	}

	Pose(const Pose &) = delete;

	Pose &operator=(Pose &&rhs) 
	{
		mPositions.clear();
		mPositions.swap(rhs.mPositions);
		mRotations.clear();
		mRotations.swap(rhs.mRotations);
		return *this;
	}

	const std::vector<glm::vec3> &getPositions() const {
		return mPositions;
	}

	const std::vector<glm::quat> &getRotations() const {
		return mRotations;
	}

	unsigned int getNumBones() const {
		return mPositions.size();
	}

private:
	std::vector<glm::vec3> mPositions;
	std::vector<glm::quat> mRotations;
};

#endif