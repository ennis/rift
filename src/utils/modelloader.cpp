#include <modelloader.hpp>
#include <log.hpp>

class OBJModel
{
public:
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

	void load(char const *filename);

	std::vector<float3> const &getPositions();
	std::vector<float3> const &getNormals();
	std::vector<float2> const &getTexCoords();
	std::vector<Face> const &getFaces();

	void convertToGLMesh(
		std::vector<float> &positions, 
		std::vector<float> &normals, 
		std::vector<float> &texcoords);

private:
	std::vector<float3> mPositions;
	std::vector<float3> mNormals;
	std::vector<float2> mTexCoords;
	std::vector<Face> mFaces;
};

void OBJModel::load(const char *filename)
{
	// open stream
	std::ifstream is(filename);

	if (is.fail()) {
		throw std::runtime_error("OBJFile::load: could not open file");
	}

	// open success
	LOG << "loading " << filename << "...";

	std::string line;
	std::stringstream ss;
	std::string token;
	int numline = 0;
	while (!is.eof()) {
		// read a line
		std::getline(is, line);
		++numline;
		// skip empty lines
		if (line.size() == 0) {
			continue;
		}
		ss.clear();
		ss.str(line);
		// get token
		ss >> token;
		//std::cout << token.c_str() << '\n';
		if (token.compare("v") == 0) {
			// parse vertex declaration
			float3 vertex;
			ss >> vertex.x >> vertex.y >> vertex.z;
			// ignore end of line
			mPositions.push_back(vertex);
		}
		else if (token.compare("vn") == 0) {
			// parse vertex normal declaration
			float3 normal;
			ss >> normal.x >> normal.y >> normal.z;
			mNormals.push_back(normal);
		}
		else if (token.compare("vt") == 0) {
			// parse texcoord declaration
			float2 texcoord;
			ss >> texcoord.u >> texcoord.v;
			mTexCoords.push_back(texcoord);
		}
		else if (token.compare("f") == 0) {
			// parse face declaration
			char delim;
			Face f;
			for (int i = 0; i < 3; ++i) {
				ss >> f.vertices[i] >> delim;
				if (delim != '/') {
					ERROR << "syntax error at line " << numline;
				}
				if (ss.peek() == '/') {
					f.texcoords[i] = 0;
					ss >> delim >> f.normals[i];
				} else {
					ss >> f.texcoords[i] >> delim;
					if (delim != '/') {
						ERROR << "syntax error at line " << numline;
					}
					ss >> f.normals[i];
				}
			}
			mFaces.push_back(f);
		}
		else if (token[0] == '#') {
			// comment line, ignored
		}
		else {
			// ignored
		}

		// check for failbit or eofbit after parsing a declaration
		if (ss.fail()) {
			ERROR << "syntax error at line " << numline;
		}
	}

	LOG << "vertices: " << mPositions.size()
		<< "\nnormals: " << mNormals.size()
		<< "\ntexcoords: " << mTexCoords.size()
		<< "\nfaces: " << mFaces.size();
}

void OBJModel::convertToGLMesh(
	std::vector<float> &positions,
	std::vector<float> &normals,
	std::vector<float> &texcoords)
{
	for (auto &f : mFaces) {
		// for each vertex (3)
		for (int i = 0; i < 3; ++i) {
			positions.push_back(mPositions[f.vertices[i] - 1].x);
			positions.push_back(mPositions[f.vertices[i] - 1].y);
			positions.push_back(mPositions[f.vertices[i] - 1].z);

			normals.push_back(mNormals[f.normals[i] - 1].x);
			normals.push_back(mNormals[f.normals[i] - 1].y);
			normals.push_back(mNormals[f.normals[i] - 1].z);

			if (mTexCoords.size()) {
				texcoords.push_back(mTexCoords[f.texcoords[i] - 1].u);
				texcoords.push_back(mTexCoords[f.texcoords[i] - 1].v);
			}
			else {
				texcoords.push_back(0.f);
				texcoords.push_back(0.f);
			}
		}
	}

	LOG << "OBJ: " << positions.size() << " unique vertices";
}

CMeshBuffer *loadMeshFromOBJ(CRenderer &renderer, const char *path)
{
	OBJModel mesh;
	mesh.load(path);
	std::vector<float> positions;
	std::vector<float> normals;
	std::vector<float> texcoords;
	mesh.convertToGLMesh(positions, normals, texcoords);

	MeshBufferInit mbi;
	mbi.desc.numVertices = positions.size()/3;
	mbi.desc.numIndices = 0;
	mbi.desc.layoutType = Layout_Full;
	mbi.desc.primitiveType = PrimitiveType::Triangles;
	mbi.positions = positions.data();
	mbi.normals = normals.data();
	mbi.texcoords = texcoords.data();

	return renderer.getImpl().createMeshBuffer(mbi);
}

CModel *loadModelFromOBJ(CRenderer &renderer, const char *path)
{
	CModel *model = renderer.createModel();
	CMeshBuffer *buf = loadMeshFromOBJ(renderer, path);
	model->addMeshPart(buf, nullptr);
	return model;
}
