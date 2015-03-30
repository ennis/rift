#include <skeletonanimation.hpp>

SkeletonAnimation SkeletonAnimation::loadFromBVH(std::istream &streamIn, const Skeleton &skel, util::array_ref<BVHMapping> mappings)
{
	SkeletonAnimation result;
	result.animation_curves = AnimationCurve<float>::loadCurvesFromBVH(streamIn, mappings.size());
	result.mappings = mappings.vec();
	return result;
}


void SkeletonAnimationSampler::nextFrame()
{
	current_time += frame_rate; 
	
	for (auto mapping_index = 0u; mapping_index < anim.mappings.size(); ++mapping_index)
	{
		const auto &mapping = anim.mappings[mapping_index];
		auto joint_index = mapping.joint_index;
		switch (mapping.dof)
		{
		case DOF::TranslationX:
			translations[joint_index].x = anim.animation_curves[mapping_index].evaluate(current_time);
			break;
		case DOF::TranslationY:
			translations[joint_index].y = anim.animation_curves[mapping_index].evaluate(current_time);
			break;
		case DOF::TranslationZ:
			translations[joint_index].z = anim.animation_curves[mapping_index].evaluate(current_time);
			break;
		// XXX transform to quaternion before interpolation?
		case DOF::RotationX:
			rotations[joint_index].x = anim.animation_curves[mapping_index].evaluate(current_time);
			break;
		case DOF::RotationY:
			rotations[joint_index].y = anim.animation_curves[mapping_index].evaluate(current_time);
			break;
		case DOF::RotationZ:
			rotations[joint_index].z = anim.animation_curves[mapping_index].evaluate(current_time);
		default:
			break;
		}
	}
}

namespace
{
	void calculateTransformsRec(
		const std::vector<Joint> &joints,
		const std::vector<glm::vec3> &translations,
		const std::vector<glm::vec3> &rotations,
		std::vector<glm::mat4> &out_transforms,
		const glm::mat4 &current_transform,
		int joint_index)
	{
		auto joint = joints[joint_index];
		Transform t;
		t.rotation = glm::quat(rotations[joint_index]);
		t.position = translations[joint_index];
		out_transforms[joint_index] = current_transform * t.toMatrix();
		for (auto child_index : joint.children) {
			auto child_tf = current_transform * glm::translate(joints[child_index].init_offset);
			calculateTransformsRec(joints, translations, rotations, out_transforms, child_tf, child_index);
		}
	}
}

std::vector<glm::mat4> SkeletonAnimationSampler::getPose(const Skeleton &skeleton, const glm::mat4 &root_transform) const
{
	// compute transforms
	assert(translations.size() == rotations.size());
	std::vector<glm::mat4> transforms(skeleton.joints.size());
	// 0 must be root joint
	calculateTransformsRec(skeleton.joints, translations, rotations, transforms, root_transform, 0);
	return transforms;
}
