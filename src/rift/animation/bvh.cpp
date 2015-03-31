#include <skeleton.hpp>

#include <string>
#include <log.hpp>

namespace
{
	void readOffset(
		std::istream &inputfile, 
		Joint &joint)
	{
		std::string buf;
		inputfile >> buf; //doit contenir OFFSET
		if (buf.compare("OFFSET") != 0)
			ERROR << "missing OFFSET: " << joint.name;

		inputfile >> joint.init_offset.x;
		inputfile >> joint.init_offset.y;
		inputfile >> joint.init_offset.z;
	}

	void readChannels(
		std::istream &streamIn, 
		std::vector<Joint> &joints, 
		std::vector<BVHMapping> &mappings)
	{
		auto &joint = joints.back();
		std::string buf;
		streamIn >> buf; //doit contenir CHANNELS
		if (buf.compare("CHANNELS") != 0)
			ERROR << "missing CHANNELS: " << joint.name;

		int nb_dofs; 
		streamIn >> nb_dofs;
		int channel_id = joints.size() - 1;
		for (int i = 0; i < nb_dofs; i++)
		{
			streamIn >> buf;
			if (buf == "Xposition")
				mappings.push_back(BVHMapping(channel_id, DOF::TranslationX));
			else if (buf == "Yposition")
				mappings.push_back(BVHMapping(channel_id, DOF::TranslationY));
			else if (buf == "Zposition")
				mappings.push_back(BVHMapping(channel_id, DOF::TranslationZ));
			else if (buf == "Xrotation")
				mappings.push_back(BVHMapping(channel_id, DOF::RotationX));
			else if (buf == "Yrotation")
				mappings.push_back(BVHMapping(channel_id, DOF::RotationY));
			else if (buf == "Zrotation")
				mappings.push_back(BVHMapping(channel_id, DOF::RotationZ));
		}
	}

	void readEndSite(
		std::istream &streamIn,
		std::vector<Joint> &joints)
	{
		std::string buf;
		auto &joint = joints.back();
		streamIn >> buf;
		if (buf != "{")
			ERROR << "missing {  in readEndSite: " << joint.name;
		readOffset(streamIn, joint);
		streamIn >> buf;
		if (buf != "}")
			ERROR << "missing } in readEndSite: " << joint.name;
	}

	void readJoint(
		std::istream &streamIn,
		std::vector<Joint> &joints,
		std::vector<BVHMapping> &mappings)
	{
		auto cur_joint_index = joints.size()-1;
		std::string buf;
		streamIn >> buf;
		if (buf.compare("{") != 0)
			ERROR << "missing { in readJoint: " << joints[cur_joint_index].name;

		int index_parent = joints.size() - 1;
		readOffset(streamIn, joints[cur_joint_index]);
		readChannels(streamIn, joints, mappings);
		streamIn >> buf; // peut contenir JOINT ou End

		if (buf.compare("End") == 0)
		{ // Cas d'arret
			streamIn >> buf;
			if (buf.compare("Site") != 0)
				ERROR << "missing Site: " << joints.back().name;

			Joint joint("EndSite");
			joint.parent = index_parent;
			joints[cur_joint_index].children.push_back(joints.size());
			joints.push_back(joint);
			readEndSite(streamIn, joints);
		}
		else if (buf.compare("JOINT") == 0)
		{ // 1 ou plus Joint successifs
			streamIn >> buf; //contient le nom
			Joint joint(buf);
			joint.parent = index_parent;
			joints[cur_joint_index].children.push_back(joints.size());
			joints.push_back(joint);
			readJoint(streamIn, joints, mappings);

			while ((streamIn >> std::ws).peek() != '}')
			{
				streamIn >> buf; //doit contenir JOINT
				if (buf != "JOINT")
					ERROR << "missing JOINT: " << joints.back().name;
				streamIn >> buf; //contient le nom
				Joint joint(buf);
				joint.parent = index_parent;
				joints[cur_joint_index].children.push_back(joints.size());
				joints.push_back(joint);
				readJoint(streamIn, joints, mappings);
			}
		}
		else { // impossible
			ERROR << "Syntax error: unexpected symbol: " << joints.back().name;
		}

		streamIn >> buf;
		if (buf.compare("}") != 0)
			ERROR << "missing } in readJoint: " << joints.back().name;
	}

	void readHierarchy(
		std::istream &streamIn,
		std::vector<Joint> &joints, 
		std::vector<BVHMapping> &mappings)
	{
		std::string buf;
		streamIn >> buf; //doit contenir ROOT
		if (!buf.compare("ROOT")) 
		{
			streamIn >> buf; //doit contenir le nom "root"
			Joint joint(buf);
			joint.init_offset = glm::vec3();
			joints.push_back(joint);
			readJoint(streamIn, joints, mappings);
		}
	}
}


Skeleton::Ptr Skeleton::loadFromBVH(
	std::istream &streamIn,
	std::vector<BVHMapping> &bvhMappings)
{
	auto ptr = std::make_unique<Skeleton>();

	while (!streamIn.eof()) {
		std::string buf;
		streamIn >> buf;
		if (!buf.compare("HIERARCHY")) {
			readHierarchy(streamIn, ptr->joints, bvhMappings);
		}
	}

	return ptr;
}