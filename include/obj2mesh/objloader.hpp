#ifndef OBJLOADER_HPP
#define OBJLOADER_HPP

#include <common.hpp>
#include <vector>
#include <list>
#include <istream>

struct OBJModel
{
	struct float3 {
		float x, y, z;
	};

	struct float2 {
		float u, v;
	};

	// triangular faces only
	struct Face {
		int vertices[3];
		int normals[3];
		int texcoords[3];
	};

	struct Group
	{
		std::string name;
		std::string usemtl;
		std::vector<Face> faces;
	};

	void load(std::istream &input);

	

	std::vector<float3> mPositions;
	std::vector<float3> mNormals;
	std::vector<float2> mTexCoords;
	std::list<Group> mGroups;
};


#endif