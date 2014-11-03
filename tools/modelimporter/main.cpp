#include <yaml-cpp/yaml.h>
#include <log.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/ProgressHandler.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/packing.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <utils/meshdata.hpp>

class ImportProgressHandler : public Assimp::ProgressHandler
{
public:
	bool Update(float percentage) {
		LOG << percentage << "%...";
		return true;
	}
};

// convert vertex data imported by assimp  
void convertVertexFull(
	const aiScene *scene, 
	int numTotalVertices, 
	int numTotalIndices, 
	char *&outVertexData,
	std::size_t &outVertexSize,
	char *&outIndexData,
	std::size_t &outIndexSize)
{
	auto vertexData = new MDAT_VertexLayoutFull[numTotalVertices];
	auto indexData = new uint16_t[numTotalIndices];
	auto vertexPtr = vertexData;
	auto indexPtr = indexData;

	// process each mesh (or submesh, as we call it)
	for (unsigned int imesh = 0; imesh < scene->mNumMeshes; ++imesh) {
		auto mesh = scene->mMeshes[imesh];
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			vertexPtr->position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertexPtr->normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			vertexPtr->tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertexPtr->bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
			vertexPtr->texcoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			++vertexPtr;
		}
		// just copy the indices
		// TODO more than 65536 indices?
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			*indexPtr++ = mesh->mFaces[i].mIndices[0];
			*indexPtr++ = mesh->mFaces[i].mIndices[1];
			*indexPtr++ = mesh->mFaces[i].mIndices[2];
		}
	}

	outVertexData = reinterpret_cast<char*>(vertexData);
	outVertexSize = numTotalVertices * sizeof(MDAT_VertexLayoutFull);
	outIndexData = reinterpret_cast<char*>(indexData);
	outIndexSize = numTotalIndices * 2;
}


// convert vertex data imported by assimp  
void convertVertexPacked(
	const aiScene *scene,
	int numTotalVertices,
	int numTotalIndices,
	char *&outVertexData,
	std::size_t &outVertexSize,
	char *&outIndexData,
	std::size_t &outIndexSize)
{
	auto vertexData = new MDAT_VertexLayoutPacked[numTotalVertices];
	auto indexData = new uint16_t[numTotalIndices];
	auto vertexPtr = vertexData;
	auto indexPtr = indexData;

	for (unsigned int imesh = 0; imesh < scene->mNumMeshes; ++imesh) {
		auto mesh = scene->mMeshes[imesh];
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			vertexPtr->position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertexPtr->normal = glm::packSnorm3x10_1x2(glm::vec4(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z, 0));
			vertexPtr->tangent = glm::packSnorm3x10_1x2(glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0));
			vertexPtr->bitangent = glm::packSnorm3x10_1x2(glm::vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0));
			vertexPtr->texcoord = glm::packUnorm2x16(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
			++vertexPtr;
		}
		// just copy the indices
		// TODO more than 65536 indices?
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			*indexPtr++ = mesh->mFaces[i].mIndices[0];
			*indexPtr++ = mesh->mFaces[i].mIndices[1];
			*indexPtr++ = mesh->mFaces[i].mIndices[2];
		}
	}

	outVertexData = reinterpret_cast<char*>(vertexData);
	outVertexSize = numTotalVertices * sizeof(MDAT_VertexLayoutPacked);
	outIndexData = reinterpret_cast<char*>(indexData);
	outIndexSize = numTotalIndices * 2;
}

bool convertModel(
	std::string const &fileName, 
	MDAT_LayoutType outputLayoutType,
	std::string const &outputFileName)
{
	LOG << "Importing " << fileName << "...";

	// Create an instance of the Importer class
	Assimp::Importer importer;

	// set our custom progress handler
	// BAD DOCUMENTATION: the importer assumes ownership of the object and expects it to be allocated with new
	importer.SetProgressHandler(new ImportProgressHandler);

	// import the model
	// we need tangents and triangles (drop points and lines)
	const aiScene* scene = importer.ReadFile(
		fileName,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType |
		aiProcess_PreTransformVertices);


	// If the import failed, report it
	if (!scene)
	{
		ERROR << importer.GetErrorString();
		return false;
	}

	int numVertices = 0;
	int numIndices = 0;
	MDAT_SubMeshDesc *subMeshes = new MDAT_SubMeshDesc[scene->mNumMeshes];

	// initialize the list of submeshes and count the total number of vertices and indices
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) 
	{
		// texture coordinates and tangent space are required
		// if they were not present or could not be computed, we fail
		if (!scene->mMeshes[i]->HasTextureCoords(0)) {
			delete[] subMeshes;
			ERROR << "mesh does not have texture coordinates";
			return 0;
		}

		if (!scene->mMeshes[i]->HasTangentsAndBitangents()) {
			delete[] subMeshes;
			ERROR << "mesh does not have tangent data";
			return 0;
		}

		subMeshes[i].vertexOffset = numVertices;
		subMeshes[i].indexOffset = numIndices;
		subMeshes[i].numVertices = scene->mMeshes[i]->mNumVertices;
		subMeshes[i].numIndices = scene->mMeshes[i]->mNumFaces * 3;
		numVertices += scene->mMeshes[i]->mNumVertices;
		numIndices += scene->mMeshes[i]->mNumFaces * 3;

		// assign default name if the submesh has no name assigned
		aiString &name = scene->mMeshes[i]->mName;
		if (name.length == 0) {
			std::string newName = "mesh_" + std::to_string(i);
			strncpy(subMeshes[i].name, newName.data(), MDAT_MaxSubMeshNameSize);
		}
		else {
			strncpy(subMeshes[i].name, name.data, MDAT_MaxSubMeshNameSize);
		}

		LOG << "submesh[" << i << "] (" << subMeshes[i].name 
			<< ") vertexOffset=" << subMeshes[i].vertexOffset
			<< " numVertices=" << subMeshes[i].numVertices
			<< " indexOffset=" << subMeshes[i].indexOffset
			<< " numIndices=" << subMeshes[i].numIndices;
	}

	LOG << fileName << ": " << scene->mNumMeshes << " subMeshes, " << numVertices << " vertices, " << numIndices << " indices";
	LOG << "Writing " << outputFileName << "...";


	// initialize file header
	MDAT_Header header;
	strncpy(header.sig, "MESH", 4);
	header.dataOffset = sizeof(MDAT_Header) + scene->mNumMeshes * sizeof(MDAT_SubMeshDesc);
	header.vertexSize = numVertices;
	header.indexSize = numIndices;
	header.numSubMeshes = scene->mNumMeshes;

	// buffers for vertex and index data
	char *vertexData;
	char *indexData;
	std::size_t vertexDataSize;
	std::size_t indexDataSize;

	// convert data to target vertex format
	switch (outputLayoutType) {
	case GL3_Layout_Full:
		convertVertexFull(scene, numVertices, numIndices, vertexData, vertexDataSize, indexData, indexDataSize);
		header.vertexFormat = 0;
		break;
	case GL3_Layout_Packed:
		convertVertexPacked(scene, numVertices, numIndices, vertexData, vertexDataSize, indexData, indexDataSize);
		header.vertexFormat = 1;	
		break;
	}

	// handle materials
	if (!scene->HasMaterials()) {
		// this means that the submeshes will be assigned a default material
		WARNING << "Model has no material";
	}
	else {
		// try to get some material parameters
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
			aiMaterial *mat = scene->mMaterials[i];
			aiString name;
			mat->Get(AI_MATKEY_NAME, name);
			LOG << "Material[" << i << "] (" << name.C_Str() << ")";
		}
	}

	// write data to file
	std::ofstream ofs(outputFileName, std::ios::binary);

	// write header 
	// signature, number of submeshes, start offset of vertex/index data, size of vertex data, size of index data
	ofs.write(reinterpret_cast<const char*>(&header), sizeof(header));
	// submesh descriptors
	ofs.write(reinterpret_cast<const char*>(subMeshes), scene->mNumMeshes * sizeof(MDAT_SubMeshDesc));
	// vertex data
	ofs.write(vertexData, vertexDataSize);
	// index data
	ofs.write(indexData, indexDataSize);
	ofs.close();

	// free memory
	delete[] subMeshes;
	delete[] vertexData;
	delete[] indexData;

	return true;
}

void usage()
{
	std::cout << "TODO" << std::endl;
}

int main(int argc, char **argv)
{
	namespace fs = boost::filesystem;
	namespace po = boost::program_options;

	logInit("modelimporter");

	// parse command line
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("layout-type", po::value<std::string>(), "set output layout type")
		("input-files", po::value<std::vector<std::string> >(), "set input files");
	
	po::positional_options_description positional;
	positional.add("input-files", -1);

	po::variables_map vm;
	try {
		po::store(po::command_line_parser(argc, argv).options(desc).positional(positional).run(), vm);
		po::notify(vm);
	}
	catch (std::exception &e) {
		ERROR << e.what();
		return 1;
	}

	MDAT_LayoutType layoutType = GL3_Layout_Full;

	// read layout-type from command line
	if (vm.count("layout-type")) {
		std::string str = vm["layout-type"].as<std::string>();
		if (str == "full") {
			layoutType = GL3_Layout_Full;
		}
		else if (str == "packed") {
			layoutType = GL3_Layout_Packed;
		}
	}

	// convert all models specified on the command line
	auto inputFiles = vm["input-files"].as< std::vector<std::string> >();
	for (auto &inputFile : inputFiles) {
		fs::path inputPath(inputFile);
		fs::path outputPath(inputFile);
		outputPath.replace_extension("mesh");
		convertModel(inputPath.string(), layoutType, outputPath.string());
	}

	return 0;
}