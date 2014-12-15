#ifndef ANIMATIONTRACK_HPP
#define ANIMATIONTRACK_HPP

#include <common.hpp>
#include <string>

//
// Base class for all kinds of animation tracks (list of keyframes)
class AnimationTrack
{
public:
	enum class RepeatMode 
	{
		// TODO
		Repeat,
		Once
	};

	~AnimationTrack()
	{}

protected:
	AnimationTrack() = default;

	AnimationTrack(
		const char *name,
		unsigned int numKeys,
		float duration,
		RepeatMode repeatMode) : 
	mName(name),
	mNumKeys(numKeys),
	mDuration(duration),
	mRepeatMode(repeatMode)
	{}

	AnimationTrack(AnimationTrack &&rhs) : 
		mName(std::move(rhs.mName)),
		mNumKeys(rhs.mNumKeys),
		mDuration(rhs.mDuration),
		mRepeatMode(rhs.mRepeatMode)
	{}

private:
	std::string mName;
	unsigned int mNumKeys = 0;
	float mDuration = 0.0f;
	RepeatMode mRepeatMode = RepeatMode::Once;
};

 
#endif /* end of include guard: ANIMATIONTRACK_HPP */