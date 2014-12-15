#include <spatialanimationtrack.hpp>
#include <serialization.hpp>

// load spatial animation from stream
SpatialAnimationTrack SpatialAnimationTrack::loadFromStream(std::istream &streamIn)
{
	using namespace rift::serialization;
	Unpacker u(streamIn);
	// number of keys
	unsigned int numKeys;
	u.unpack(numKeys);
	// extract keys
	std::vector<float> times;
	std::vector<Keyframe> keyframes;
	times.reserve(numKeys);
	keyframes.reserve(numKeys);
	for (unsigned int i = 0; i < numKeys; ++i) {
		float t, px, py, pz, qx, qy, qz, qw, sx, sy, sz;
		u.unpack(t);
		u.unpack(px); u.unpack(py); u.unpack(pz);
		u.unpack(qx); u.unpack(qy); u.unpack(qz); u.unpack(qw);
		u.unpack(sx); u.unpack(sy); u.unpack(sz);
		times.push_back(t);
		keyframes.push_back(
			Keyframe{
				glm::vec3(px, py, pz), 
				glm::quat(qx, qy, qz, qw), 
				glm::vec3(sx, sy, sz)});
	}
	return SpatialAnimationTrack(
		std::move(times), 
		std::move(keyframes));
}
