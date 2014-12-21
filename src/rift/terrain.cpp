#include <terrain.hpp>
#include <engine.hpp>
#include <boundingbox.hpp>
#include <cmath>
#include <algorithm>

//=============================================================================
Terrain::Terrain(Image *heightmapData) :
mNumSelectedNodes(0), 
mPatchGridSize(16),
mPatchGridVB(nullptr),
mPatchGridIB(nullptr),
mHeightmapData(heightmapData),
mHeightmapTexture(nullptr),
mHeightmapVerticalScale(200.f),
mHeightmapSize(heightmapData->imageView(0, 0).size()),
mNumLodLevels(0),
mShader(nullptr)
{
	init();
}

//=============================================================================
Terrain::~Terrain()
{
	mHeightmapData->release();
	mHeightmapTexture->release();
	mPatchGridVB->release();
	mPatchGridIB->release();
}

//=============================================================================
void Terrain::init()
{
	initShader();
	initGrid();
	initHeightmap();
	calculateLodRanges();
}

//=============================================================================
void Terrain::initHeightmap()
{
	auto &renderer = Engine::instance().getRenderer();

	mHeightmapView = mHeightmapData->imageView(0, 0).viewAs<uint16_t>();
	mHeightmapTexture = renderer.createTexture2D(
		mHeightmapSize,
		1,
		ElementFormat::Unorm16,
		mHeightmapSize.x * mHeightmapSize.y * sizeof(uint16_t),
		mHeightmapView.data());
	mLog2HeightmapSize = std::max(1, int(log2(std::max(mHeightmapSize.x / mPatchGridSize, 1))) + 1);
}

//=============================================================================
void Terrain::initShader()
{
	auto &renderer = Engine::instance().getRenderer();

	mShader = renderer.createShader(
		loadShaderSource("resources/shaders/terrain/vert.glsl").c_str(),
		loadShaderSource("resources/shaders/terrain/frag.glsl").c_str());
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
void Terrain::initGrid()
{
	auto &renderer = Engine::instance().getRenderer();

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
	auto &renderer = Engine::instance().getRenderer();
	// TODO support renderstates
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	renderer.setVertexBuffer(0, mPatchGridVB);
	renderer.setIndexBuffer(mPatchGridIB);
	renderer.setVertexLayout(mVertexLayout);
	renderer.setShader(mShader);
	renderer.setConstantBuffer(0, renderContext.perFrameShaderParameters);
	renderer.setNamedConstantMatrix4("modelMatrix",
		Transform().scale(1.f).toMatrix());
	renderer.setNamedConstantInt2("heightmapSize", mHeightmapSize);
	renderer.setNamedConstantFloat("heightmapScale", mHeightmapVerticalScale);
	renderer.setTexture(0, mHeightmapTexture);
	for (int i = 0; i < mNumSelectedNodes; ++i) {
		Node const &node = mSelectedNodes[i];
		renderNode(renderContext, node);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//=============================================================================
void Terrain::renderNode(RenderContext const &renderContext, Node const &node) 
{
	auto &renderer = Engine::instance().getRenderer();
	// just update the uniforms and do the draw call:
	// the resources are already bound
	// patch offset in world coordinates
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
	if (mNumSelectedNodes >= kMaxNodes) {
		return;
	}
	mSelectedNodes[mNumSelectedNodes++] = node;
}

//=============================================================================
void Terrain::clearNodeSelection()
{
	mNumSelectedNodes = 0;
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
