#include <animationclip.hpp>
#include <serialization.hpp>
#include <fstream>
#include <log.hpp>

namespace
{
	using namespace rift::serialization;
	AnimationClip::Channel loadChannel(Unpacker &unpacker)
	{
		std::string channelName;
		unpacker.unpack(channelName);
		LOG << "-- Loading channel : " << channelName.c_str();
		unsigned int numPositionKeys, numRotationKeys, numScaleKeys;
		std::vector<AnimationClip::PositionKey> positionKeys;
		std::vector<AnimationClip::RotationKey> rotationKeys;
		// ====== Positions ======
		unpacker.unpack(numPositionKeys);
		positionKeys.reserve(numPositionKeys);
		assert(numPositionKeys < 65536);
		LOG << "  -- " << numPositionKeys << " position keys";
		for (unsigned int i = 0; i < numPositionKeys; ++i) {
			float time;
			float x, y, z;
			unpacker.unpack(time);
			unpacker.unpack(x);
			unpacker.unpack(y);
			unpacker.unpack(z);
			positionKeys.emplace_back(AnimationClip::PositionKey{ time, glm::vec3(x, y, z) });
		}
		// ====== Rotations ======
		unpacker.unpack(numRotationKeys);
		assert(numRotationKeys < 65536);
		LOG << "  -- " << numRotationKeys << " rotation keys";
		rotationKeys.reserve(numRotationKeys);
		for (unsigned int i = 0; i < numRotationKeys; ++i) {
			float time;
			float x, y, z, w;
			unpacker.unpack(time);
			unpacker.unpack(x);
			unpacker.unpack(y);
			unpacker.unpack(z);
			unpacker.unpack(w);
			rotationKeys.emplace_back(AnimationClip::RotationKey{ time, glm::quat(x, y, z, w) });
		}
		// ====== Scale keys (ignored) ======
		unpacker.unpack(numScaleKeys);
		assert(numScaleKeys < 65536);
		LOG << "  -- " << numScaleKeys << " scale keys";
		for (unsigned int i = 0; i < numScaleKeys; ++i) {
			unpacker.skip<float>();	// time
			unpacker.skip<float>();	// X scale 
			unpacker.skip<float>();	// Y scale
			unpacker.skip<float>();	// Z scale
		}
		return AnimationClip::Channel(
			std::move(channelName), 
			std::move(positionKeys), 
			std::move(rotationKeys));
	}

	template <typename T>
	int findNearestKey(const std::vector<T> &keys, float time)
	{
		unsigned int l = 0;
		unsigned int u = static_cast<unsigned int>(keys.size());
		unsigned int p = l + (u - l) / 2;
		// dichotomy
		while (p != l) {
			if (time < keys[p].time) {
				u = p;
			}
			else if (time > keys[p].time) {
				l = p;
			}
			else {
				return p;
			}
			p = l + (u - l) / 2;
		}
		return p;
	}
}

AnimationClip AnimationClip::loadFromFile(const char *fileName)
{
	AnimationClip clip;
	std::ifstream fileIn(fileName, std::ios::in | std::ios::binary);
	assert(fileIn.is_open());
	Unpacker unpacker(fileIn);

	std::string name;
	unpacker.unpack(name);
	LOG << "Loading animation clip: " << name.c_str();
	unsigned int numChannels;
	unpacker.unpack(numChannels);
	for (unsigned int i = 0; i < numChannels; ++i) {
		// load channels
		clip.mChannels.emplace_back(loadChannel(unpacker));
	}

	return clip;
}

// XXX move this into named constructor Pose::fromAnimationClip(float time)
Pose AnimationClip::computePose(float time)
{
	std::vector<glm::vec3> positions;
	std::vector<glm::quat> rotations;

	for (const auto &ch : mChannels) {
		positions.push_back(ch.getPositionKeys()[findNearestKey(ch.getPositionKeys(), time)].pos);
		rotations.push_back(ch.getRotationKeys()[findNearestKey(ch.getRotationKeys(), time)].rotation);
	}

	return Pose(std::move(positions), std::move(rotations));
}