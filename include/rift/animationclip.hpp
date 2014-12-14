#pragma once
#include <common.hpp>
#include <vector>
#include <string>
#include <pose.hpp>


class AnimationClip
{
public:
	struct PositionKey
	{
		float time;
		glm::vec3 pos;
	};

	struct RotationKey
	{
		float time;
		glm::quat rotation;
	};

	class Channel
	{
	public:
		Channel(
			std::string &&nodeName, 
			std::vector<PositionKey> &&positionKeys, 
			std::vector<RotationKey> &&rotationKeys) :
			mNodeName(nodeName),
			mPositionKeys(positionKeys),
			mRotationKeys(rotationKeys)
		{}

		Channel(Channel &&rhs) : 
			mNodeName(std::move(rhs.mNodeName)), 
			mPositionKeys(std::move(rhs.mPositionKeys)), 
			mRotationKeys(std::move(rhs.mRotationKeys))
		{
		}

		Channel(Channel const &rhs) = delete;

		std::string const &getNodeName() const {
			return mNodeName;
		}

		std::vector<PositionKey> const &getPositionKeys() const {
			return mPositionKeys;
		}

		std::vector<RotationKey> const &getRotationKeys() const {
			return mRotationKeys;
		}

	private:
		std::string mNodeName;
		std::vector<PositionKey> mPositionKeys;
		std::vector<RotationKey> mRotationKeys;
	};

	AnimationClip(AnimationClip &&rhs) :
		mChannels(std::move(rhs.mChannels))
	{}

	AnimationClip(AnimationClip const &rhs) = delete;

	std::vector<AnimationClip::Channel> const &getChannels() const {
		return mChannels;
	}

	Pose computePose( float time);

	static AnimationClip loadFromFile(const char *fileName);

private:
	AnimationClip()
	{}

	std::vector<AnimationClip::Channel> mChannels;
};