#include <bvh.hpp>

#include <string>
#include <log.hpp>

void readOffset(std::ifstream &inputfile, Skeleton &skel){
	std::string buf;
	inputfile >> buf; //doit contenir OFFSET
	if (buf.compare("OFFSET") != 0)
		ERROR << "missing OFFSET: " << skel.joints.back().name;

	inputfile >> skel.joints.back().init_offset.x;
	inputfile >> skel.joints.back().init_offset.y;
	inputfile >> skel.joints.back().init_offset.z;
}

void readChannels(std::ifstream &inputfile, Skeleton &skel, std::vector<Mapping> &mappings){
	std::string buf;
	inputfile >> buf; //doit contenir CHANNELS
	if (buf.compare("CHANNELS") != 0)
		ERROR << "missing CHANNELS: " << skel.joints.back().name;

	int nb_dofs; inputfile >> nb_dofs;

	int skelSize = skel.joints.size() - 1;

	for (int i = 0; i < nb_dofs; i++){
		inputfile >> buf;

		if (!buf.compare("Xposition"))
		{
			mappings.push_back(Mapping(skelSize, TRANSX)); continue;
		}
		if (!buf.compare("Yposition"))
		{
			mappings.push_back(Mapping(skelSize, TRANSY)); continue;
		}
		if (!buf.compare("Zposition"))
		{
			mappings.push_back(Mapping(skelSize, TRANSX)); continue;
		}
		if (!buf.compare("Xrotation"))
		{
			mappings.push_back(Mapping(skelSize, ROTX)); continue;
		}
		if (!buf.compare("Yrotation"))
		{
			mappings.push_back(Mapping(skelSize, ROTY)); continue;
		}
		if (!buf.compare("Zrotation"))
		{
			mappings.push_back(Mapping(skelSize, ROTZ)); continue;
		}
	}
}

void readEndSite(std::ifstream &inputfile, Skeleton &skel){
	std::string buf;

	inputfile >> buf;
	if (buf.compare("{") != 0)
		ERROR << "missing {  in readEndSite: " << skel.joints.back().name;

	readOffset(inputfile, skel);

	inputfile >> buf;
	if (buf.compare("}") != 0)
		ERROR << "missing } in readEndSite: " << skel.joints.back().name;
}



void readJoint(std::ifstream &inputfile, Skeleton& skel, std::vector<Mapping> &mappings){
	std::string buf;
	inputfile >> buf;
	if (buf.compare("{") != 0)
		ERROR << "missing { in readJoint: " << skel.joints.back().name;

	int index_parent = skel.joints.size()-1;

	readOffset(inputfile, skel);
	readChannels(inputfile, skel, mappings);
	inputfile >> buf; // peut contenir JOINT ou End

	if (buf.compare("End") == 0){ // Cas d'arret
		inputfile >> buf;
		if (buf.compare("Site") != 0)
			ERROR << "missing Site: " << skel.joints.back().name;

		Joint joint("EndSite");
		skel.joints.push_back(joint);
		readEndSite(inputfile, skel);
		skel.joints[skel.joints.size()-1].parent = index_parent;
		skel.joints[index_parent].children.push_back(skel.joints.size()-1);
	}
	else if (buf.compare("JOINT") == 0){ // 1 ou plus Joint successifs
		inputfile >> buf; //contient le nom
		
		Joint joint(buf);
		skel.joints.push_back(joint);

		readJoint(inputfile, skel,mappings);
		skel.joints[skel.joints.size()-1].parent = index_parent;
		skel.joints[index_parent].children.push_back(skel.joints.size()-1);

		while ((inputfile >> std::ws).peek() != '}'){
			inputfile >> buf; //doit contenir JOINT
			if (buf.compare("JOINT") != 0)
				ERROR << "missing JOINT: " << skel.joints.back().name;

			inputfile >> buf; //contient le nom
			Joint joint(buf);
			skel.joints.push_back(joint);
			readJoint(inputfile, skel, mappings);
			skel.joints[skel.joints.size()-1].parent = index_parent;
			skel.joints[index_parent].children.push_back(skel.joints.size()-1);
		}
	}
	else{ // impossible
		ERROR << "Syntax error: unexpected symbol: " << skel.joints.back().name;
	}

	inputfile >> buf;
	if (buf.compare("}") != 0)
		ERROR << "missing } in readJoint: " << skel.joints.back().name;
}

void readHierarchy(std::ifstream &inputfile,
	Skeleton &skel, std::vector<Mapping> &mappings){
	std::string buf;
	inputfile >> buf; //doit contenir ROOT
	if (!buf.compare("ROOT")) {
		inputfile >> buf; //doit contenir le nom "root"
		Joint joint(buf);
		joint.init_offset = glm::vec3();
		skel.joints.push_back(joint);

		readJoint(inputfile, skel, mappings);
	}
}