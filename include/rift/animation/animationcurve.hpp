#ifndef ANIMATIONCURVE_HPP
#define ANIMATIONCURVE_HPP

#include <string>
#include <vector>
#include <istream>
#include <skeleton.hpp>
#include <bvh.hpp>

//
// Base class for all kinds of animation tracks (list of keyframes)
template <typename T>
class AnimationCurve
{
public:
	enum class RepeatMode 
	{
		// TODO
		Repeat,
		Once
	};

	struct Key
	{
		Key(T time_, T value_) : time(time_), value(value_)
		{}

		T time;
		T value;
		// TODO tangents
	};

	// empty animation curve
	AnimationCurve() = default;
	// build from vector of keys
	AnimationCurve(std::vector<Key> keys_) : keys(std::move(keys_))
	{
	}

	std::pair<int, int>
	findKeys(T time) const
	{
		// dichotomy
		int l = 0;
		int u = keys.size();
		int p = l + (u - l) / 2;
		while (p != l) 
		{
			if (time < keys[p].time) {
				u = p;
			}
			else if (time > keys[p].time) {
				l = p;
			}
			else {
				break;
			}
			p = l + (u - l) / 2;
		}

		return std::pair<int,int>(p,std::min<int>(p+1,keys.size()-1));
	}

	T evaluate(T time) const
	{
		// hermite interpolation
		auto pair = findKeys(time);
		auto p0 = keys[pair.first].value;
		auto x0 = keys[pair.first].time;
		auto p1 = keys[pair.second].value;
		auto x1 = keys[pair.second].time;

		auto dx = x1 - x0;
		auto tt = (time - x0) / dx;
		auto tt2 = tt*tt;
		auto tt3 = tt2*tt;
		auto h00 = 2 * tt3 - 3 * tt2 + 1;
		auto h10 = tt3 - 2 * tt2 + tt;
		auto h01 = -2 * tt3 + 3 * tt2;
		auto h11 = tt3 - tt2;

		// TODO non-flat tangents
		auto m0 = T(0.0);
		auto m1 = T(0.0);

		return h00*p0 + h10*dx*m0 + h01*p1 + h11*dx*m1;
	}

	// BVH loader
	static std::vector<AnimationCurve> loadCurvesFromBVH(std::istream &streamIn, int numChannels)
	{
		std::vector<AnimationCurve> curves(numChannels);
		// TODO this is ugly
		int numFrames;
		T frameTime;
		std::string word;
		streamIn >> word;
		assert(word == "MOTION");
		streamIn >> word;
		assert(word == "Frames:");
		streamIn >> numFrames;
		streamIn >> word;
		assert(word == "Frame");
		streamIn >> word;
		assert(word == "Time:");
		streamIn >> frameTime;

		int index = 0;
		T current_time = 0.0;

		for (int frame = 0; frame < numFrames; ++frame) {
			current_time = frameTime * frame;
			for (int i = 0; i < numChannels; ++i) {
				T v;
				streamIn >> v;
				curves[i].keys.push_back(Key{ current_time, v });
			}
		}
	
		return curves;
	}

private:
	RepeatMode repeatMode = RepeatMode::Repeat;
	// TODO flat_set
	std::vector<Key> keys;
};
 
#endif /* end of include guard: ANIMATIONCURVE_HPP */