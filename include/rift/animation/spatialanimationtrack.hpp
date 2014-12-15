#ifndef SPATIALANIMATIONTRACK_HPP
#define SPATIALANIMATIONTRACK_HPP

#include <animationtrack.hpp>
#include <vector>
#include <ostream>

// spatial keyframe animation (position, rotation, scale)
class SpatialAnimationTrack : public AnimationTrack
{
public:
	struct Keyframe
	{
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
	};

	SpatialAnimationTrack(
		std::vector<float> &&times, 
		std::vector<Keyframe> &&keyframes) :
	mTimes(std::move(times)),
	mKeyframes(std::move(keyframes))
	{}

	~SpatialAnimationTrack()
	{}

	static SpatialAnimationTrack loadFromStream(std::istream &streamIn);

private:
	SpatialAnimationTrack() = default;

	std::vector<float> mTimes;
	std::vector<Keyframe> mKeyframes;
};

 
#endif /* end of include guard: SPATIALANIMATIONTRACK_HPP */