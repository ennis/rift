#ifndef SKELETONANIMATION_HPP
#define SKELETONANIMATION_HPP

#include <skeleton.hpp>
#include <animationcurve.hpp>
#include <transform.hpp>
#include <array_ref.hpp>

class SkeletonAnimation
{
public:
	friend class SkeletonAnimationSampler;

	SkeletonAnimation()
	{}

	static SkeletonAnimation loadFromBVH(std::istream &streamIn, const Skeleton &skel, util::array_ref<BVHMapping> mappings);

private:
	std::vector<AnimationCurve<float> > animation_curves;
	std::vector<BVHMapping> mappings;
};


// represents an animation sampling state
// linked to a skeleton animation (and a skeleton)
class SkeletonAnimationSampler
{
public:
	SkeletonAnimationSampler(const Skeleton &skeleton_, SkeletonAnimation &anim_, float frameRate) : 
	anim(anim_),
	current_time(0.f),
	frame_rate(frameRate)
	{
		auto numjoints = skeleton_.joints.size();
		translations.resize(numjoints);
		rotations.resize(numjoints);
	}

	// advance to next frame
	void nextFrame();
	std::vector<glm::mat4> getPose(const Skeleton &skeleton, const glm::mat4 &root_transform) const;

	// compute pose at time X:
	// array (vector) of transforms: one per joint
	// transform: translation + rotation
	// -> initialized to default bone position in skeleton
	// loop over each channel
	//		sample curve

private:
	const SkeletonAnimation &anim;
	float frame_rate;
	float current_time;
	// joint transforms
	// initialized to zero
	std::vector<glm::vec3> translations;
	std::vector<glm::vec3> rotations;
	// TODO cache key indices / sample in advance
};

 
#endif /* end of include guard: SKELETONANIMATION_HPP */