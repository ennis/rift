#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include <common.hpp>

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	float distanceFromPointSq(glm::vec3 const &point)
	{
		float dist = 0.0f;

		if (point.x < min.x) {
			float d = point.x - min.x;
			dist += d*d;
		}
		else {
			if (point.x > max.x)
			{
				float d = point.x - max.x;
				dist += d*d;
			}
		}

		if (point.y < min.y) {
			float d = point.y - min.y;
			dist += d*d;
		}
		else {
			if (point.y > max.y)
			{
				float d = point.y - max.y;
				dist += d*d;
			}
		}

		if (point.z < min.z) {
			float d = point.z - min.z;
			dist += d*d;
		}
		else {
			if (point.z > max.z)
			{
				float d = point.z - max.z;
				dist += d*d;
			}
		}

		return dist;
	}
};

#endif