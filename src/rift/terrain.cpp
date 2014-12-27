#include <terrain.hpp>
#include <boundingbox.hpp>
#include <cmath>
#include <algorithm>
#include <effect.hpp>
#include <gl3error.hpp>
#include <glm/gtc/packing.hpp>

//=============================================================================
Terrain::Terrain(Terrain &&rhs) :
mNumSelectedNodes(std::move(rhs.mNumSelectedNodes)),
mPatchGridSize(std::move(rhs.mPatchGridSize)),
mPatchNumVertices(std::move(rhs.mPatchNumVertices)),
mPatchNumIndices(std::move(rhs.mPatchNumIndices)),
mPatchGridVB(std::move(rhs.mPatchGridVB)),
mPatchGridIB(std::move(rhs.mPatchGridIB)),
mVertexLayout(std::move(rhs.mVertexLayout)),
mHeightmapData(std::move(rhs.mHeightmapData)),
mHeightmapView(std::move(rhs.mHeightmapView)),
mHeightmapTexture(std::move(rhs.mHeightmapTexture)),
mHeightmapVerticalScale(std::move(rhs.mHeightmapVerticalScale)),
mHeightmapSize(std::move(rhs.mHeightmapSize)),
mLog2HeightmapSize(std::move(rhs.mLog2HeightmapSize)),
mNumLodLevels(std::move(rhs.mNumLodLevels)),
mLodRanges(std::move(rhs.mLodRanges)),
mEffect(std::move(rhs.mEffect))
{
}

//=============================================================================
Terrain::Terrain(
	Renderer &renderer, 
	Image &&heightmapData,
	Texture2D *slopeTexture,
	Texture2D *flatTexture) :
mNumSelectedNodes(0),
mPatchGridSize(32),
mPatchGridVB(nullptr),
mPatchGridIB(nullptr),
mHeightmapData(heightmapData),
mHeightmapTexture(nullptr),	
mSlopeTexture(slopeTexture),
mFlatTexture(flatTexture),
mHeightmapVerticalScale(100.f),
mHeightmapSize(heightmapData.getImageView(0, 0).size()),
mNumLodLevels(0)
{
	initHeightmap(renderer);
	initEffect(renderer);
	initGrid(renderer);
	calculateLodRanges();
}


//=============================================================================
Terrain::~Terrain()
{
	mHeightmapTexture->release();
	mPatchGridVB->release();
	mPatchGridIB->release();
}

//=============================================================================
float Terrain::getHeight(const glm::ivec2 &position)
{
	auto p = glm::clamp(position, glm::ivec2{ 0, 0 }, glm::ivec2{ mHeightmapSize.x - 1, mHeightmapSize.y - 1 });
	return mHeightmapVerticalScale * float(mHeightmapView(p.x, p.y)) / UINT16_MAX;
}

//=============================================================================
void Terrain::initHeightmap(Renderer &renderer)
{
	mHeightmapView = mHeightmapData.getImageView(0, 0).viewAs<uint16_t>();
	mHeightmapTexture = renderer.createTexture2D(
		mHeightmapSize,
		1,
		ElementFormat::Unorm16,
		mHeightmapSize.x * mHeightmapSize.y * sizeof(uint16_t),
		mHeightmapView.data());
	mLog2HeightmapSize = std::max(1, int(std::log2(std::max(mHeightmapSize.x / mPatchGridSize , 1))) + 1);

	// compute normals
	// TODO more compact format
	mHeightmapNormals.allocate(ElementFormat::Float3, glm::ivec3(mHeightmapSize, 1.f), 1);
	mHeightmapNormalsView = mHeightmapNormals.getImageView().viewAs<glm::vec3>();

	LOG << "Calulating terrain normal map...";
	for (int x = 0; x < mHeightmapSize.x; ++x) {
		for (int y = 0; y < mHeightmapSize.y; ++y) {
			// skewed 5-point stencil
			float h00 = getHeight({ x, y });
			float h01 = getHeight({ x + 1 , y });
			float h10 = getHeight({ x , y + 1 });
			float h11 = getHeight({ x + 1, y + 1 });

			float gx = 0.5f * (h11 - h00 + h01 - h10);
			float gy = 0.5f * (h00 - h11 + h01 - h10);

			glm::vec3 n{ -gx, 1.414213f, -gy };	// sqrt(2)/2 ???
			n = glm::normalize(n);
			mHeightmapNormalsView(x, y) = n;
		}
	}

	// create texture
	mHeightmapNormalTexture = mHeightmapNormals.convertToTexture2D(renderer);
}

//=============================================================================
void Terrain::initEffect(Renderer &renderer)
{
	mEffect.loadFromFile("resources/shaders/terrain.glsl");
	mShader = mEffect.compileShader(renderer);
}

//=============================================================================
void Terrain::render(RenderContext const &renderContext)
{
	// TODO
	Node rootNode;
	rootNode.lod = mLog2HeightmapSize - 1;
	rootNode.size = mHeightmapSize.x;
	rootNode.x = 0;
	rootNode.y = 0;
	clearNodeSelection();
	nodeLodSelect(
		rootNode, 
		renderContext.camera->getEntity()->getTransform().position,
		mLog2HeightmapSize-1);
	renderSelection(renderContext);
}

//=============================================================================
void Terrain::initGrid(Renderer &renderer)
{
	// number of vertices 
	int gs = mPatchGridSize+1;
	mPatchNumVertices = gs*gs;
	mPatchNumIndices = mPatchGridSize * mPatchGridSize * 6;
	// create patch grid
	float *vertices = new float[mPatchNumVertices*2];
	uint16_t *indices = new uint16_t[mPatchNumIndices];
	int p = 0;
	float xp=0.f,yp=0.f;
	float dd=1.f/mPatchGridSize;
	for (int i=0;i<gs;++i) {
		for (int j=0;j<gs;++j) {
			vertices[(i*gs+j)*2+0]=i*dd;
			vertices[(i*gs+j)*2+1]=j*dd;
			if ((i<mPatchGridSize)&&(j<mPatchGridSize)) {
				indices[p++]=i*gs+j+1;
				indices[p++]=i*gs+j;
				indices[p++]=(i+1)*gs+j;
				indices[p++]=i*gs+j+1;
				indices[p++]=(i+1)*gs+j;
				indices[p++]=(i+1)*gs+j+1;
			}
		}
	}
	// create VB and IB
	mPatchGridVB = renderer.createVertexBuffer(
		2*sizeof(float), 
		mPatchNumVertices,
		ResourceUsage::Static, 
		vertices);
	mPatchGridIB = renderer.createIndexBuffer(
		sizeof(uint16_t),
		mPatchNumIndices,
		ResourceUsage::Static,
		indices);

	const VertexElement elem_v2f[1] = {
		VertexElement(0, 0, 0, 2 * sizeof(float), ElementFormat::Float2)
	};
	mVertexLayout = renderer.createVertexLayout(1, elem_v2f);

	delete[] vertices;
	delete[] indices;
}

//=============================================================================
void Terrain::renderSelection(RenderContext const &renderContext)
{
	// TODO support renderstates
	auto &renderer = *renderContext.renderer;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	renderer.setVertexBuffer(0, mPatchGridVB);
	renderer.setIndexBuffer(mPatchGridIB);
	renderer.setVertexLayout(mVertexLayout);
	mShader->setup(renderer);
	renderer.setConstantBuffer(0, renderContext.perFrameShaderParameters);
	renderer.setNamedConstantMatrix4("modelMatrix",
		Transform().scale(1.f).toMatrix());
	renderer.setNamedConstantFloat2("heightmapSize", glm::vec2(mHeightmapSize.x, mHeightmapSize.y));
	renderer.setNamedConstantFloat("heightmapScale", mHeightmapVerticalScale);
	renderer.setTexture(0, mHeightmapTexture);
	renderer.setTexture(1, mHeightmapNormalTexture);
	renderer.setTexture(2, mSlopeTexture);
	renderer.setTexture(3, mFlatTexture);
	for (int i = 0; i < mSelectedNodes.size(); ++i) {
		Node const &node = mSelectedNodes[i];
		renderNode(renderContext, node);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//=============================================================================
void Terrain::renderNode(RenderContext const &renderContext, Node const &node) 
{
	// just update the uniforms and do the draw call:
	// the resources are already bound
	// patch offset in world coordinates
	auto &renderer = *renderContext.renderer;
	renderer.setNamedConstantFloat2("patchOffset", glm::vec2(node.x, node.y));
	renderer.setNamedConstantFloat("patchScale", float(node.size));
	renderer.setNamedConstantInt("lodLevel", node.lod);
	renderer.drawIndexed(
		PrimitiveType::Triangle, 
		0, 
		mPatchNumVertices, 
		0, 
		mPatchNumIndices);
}

//=============================================================================
void Terrain::nodeLodSelect(
	Node const &node, 
	glm::vec3 const &eye, 
	int currentLodLevel)
{
	// TODO: do frustrum culling here
	if (currentLodLevel == 0) {
		addNodeToSelection(node);
		return;
	}
	// if the current LOD level is higher that the maximum LOD 
	// the node must be subdivided anyway
	if (currentLodLevel < mNumLodLevels) {
		if (!nodeIntersectLodRange(node, eye, currentLodLevel)) {
			addNodeToSelection(node);
			return;
		}
	}
	currentLodLevel--;
	// subdiv 4x
	int half = node.size / 2;
	Node NW, NE, SW, SE;
	NW.x = node.x;
	NW.y = node.y;
	NW.size = half;
	NW.lod = currentLodLevel;
	NE.x = node.x + half;
	NE.y = node.y;
	NE.size = half;
	NE.lod = currentLodLevel;
	SW.x = node.x;
	SW.y = node.y + half;
	SW.size = half;
	SW.lod = currentLodLevel;
	SE.x = node.x + half;
	SE.y = node.y + half;
	SE.size = half;
	SE.lod = currentLodLevel;
	nodeLodSelect(NW, eye, currentLodLevel);
	nodeLodSelect(NE, eye, currentLodLevel);
	nodeLodSelect(SW, eye, currentLodLevel);
	nodeLodSelect(SE, eye, currentLodLevel);
}

//=============================================================================
bool Terrain::nodeIntersectLodRange(
	Node const &node, 
	glm::vec3 const &eye, 
	int lod)
{
	// check all corners for intersection
	AABB aabb;
	aabb.min.x = float(node.x);
	aabb.min.y = 0.f;	// TODO heightmap Y bounds
	aabb.min.z = float(node.y);
	aabb.max.x = float(node.x + node.size);
	aabb.max.y = 0.f;
	aabb.max.z = float(node.y + node.size);
	float range = mLodRanges[lod];
	return aabb.distanceFromPointSq(eye) <= range*range;
}

//=============================================================================
void Terrain::addNodeToSelection(Node const &node)
{
	mSelectedNodes.push_back(node);
}

//=============================================================================
void Terrain::clearNodeSelection()
{
	mSelectedNodes.clear();
}

//=============================================================================
void Terrain::calculateLodRanges()
{
	// TODO
	float lodRange = 2.5;
	mNumLodLevels = 
		std::min(
			int((1.f / log2(lodRange))
				 * log2(std::max(mHeightmapSize.x / mPatchGridSize, 1))) + 1, 
			kMaxLodLevel);
	LOG << "Terrain: num LOD levels = " << mNumLodLevels;
	for (int i = 0; i < mNumLodLevels; ++i) {
		mLodRanges[i] = float(mPatchGridSize) * powf(lodRange, float(i));
		LOG << "LOD " << i << " upto " << mLodRanges[i];
	}
}
