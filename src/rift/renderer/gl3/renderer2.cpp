#include <renderer2.hpp>
#include <log.hpp>
#include <unordered_map>
#include <map>

// hash specialization for samplerDesc
namespace std {
	template <> struct hash<SamplerDesc>
	{
		size_t operator()(const SamplerDesc& v) const
		{
			auto h = hash<int>();
			return h(static_cast<int>(v.addrU))
				^ h(static_cast<int>(v.addrV))
				^ h(static_cast<int>(v.addrW))
				^ h(static_cast<int>(v.minFilter))
				^ h(static_cast<int>(v.magFilter));
		}
	};
}

// operator== for samplerDesc
bool operator==(const SamplerDesc &lhs, const SamplerDesc &rhs)
{
	return (lhs.addrU == rhs.addrU)
		&& (lhs.addrV == rhs.addrV)
		&& (lhs.addrW == rhs.addrW)
		&& (lhs.minFilter == rhs.minFilter)
		&& (lhs.magFilter == rhs.magFilter);
}

namespace
{

	const std::map<GLenum, std::string> gl_debug_source_names = {
		{ gl::DEBUG_SOURCE_API, "DEBUG_SOURCE_API" },
		{ gl::DEBUG_SOURCE_APPLICATION, "DEBUG_SOURCE_APPLICATION" },
		{ gl::DEBUG_SOURCE_OTHER, "DEBUG_SOURCE_OTHER" },
		{ gl::DEBUG_SOURCE_SHADER_COMPILER, "DEBUG_SOURCE_SHADER_COMPILER" },
		{ gl::DEBUG_SOURCE_THIRD_PARTY, "DEBUG_SOURCE_THIRD_PARTY" },
		{ gl::DEBUG_SOURCE_WINDOW_SYSTEM, "DEBUG_SOURCE_WINDOW_SYSTEM" }
	};

	const std::map<GLenum, std::string> gl_debug_type_names = {
		{ gl::DEBUG_TYPE_DEPRECATED_BEHAVIOR, "DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
		{ gl::DEBUG_TYPE_ERROR, "DEBUG_TYPE_ERROR" },
		{ gl::DEBUG_TYPE_MARKER, "DEBUG_TYPE_MARKER" },
		{ gl::DEBUG_TYPE_OTHER, "DEBUG_TYPE_OTHER" },
		{ gl::DEBUG_TYPE_PERFORMANCE, "DEBUG_TYPE_PERFORMANCE" },
		{ gl::DEBUG_TYPE_POP_GROUP, "DEBUG_TYPE_POP_GROUP" },
		{ gl::DEBUG_TYPE_PORTABILITY, "DEBUG_TYPE_PORTABILITY" },
		{ gl::DEBUG_TYPE_PUSH_GROUP, "DEBUG_TYPE_PUSH_GROUP" },
		{ gl::DEBUG_TYPE_UNDEFINED_BEHAVIOR, "DEBUG_TYPE_UNDEFINED_BEHAVIOR" }
	};

	const std::map<GLenum, std::string> gl_debug_severity_names = {
		{ gl::DEBUG_SEVERITY_HIGH, "DEBUG_SEVERITY_HIGH" },
		{ gl::DEBUG_SEVERITY_LOW, "DEBUG_SEVERITY_LOW" },
		{ gl::DEBUG_SEVERITY_MEDIUM, "DEBUG_SEVERITY_MEDIUM" },
		{ gl::DEBUG_SEVERITY_NOTIFICATION, "DEBUG_SEVERITY_NOTIFICATION" }
	};

	void APIENTRY debugCallback(
		GLenum source, 
		GLenum type, 
		GLuint id, 
		GLenum severity, 
		GLsizei length, 
		const GLchar *msg, 
		const void *data)
	{
		std::string src_str = "Unknown";
		if (gl_debug_source_names.count(source)) src_str = gl_debug_source_names.at(source);
		std::string type_str = "Unknown";
		if (gl_debug_type_names.count(type)) type_str = gl_debug_type_names.at(type);
		std::string sev_str = "Unknown";
		if (gl_debug_severity_names.count(severity)) sev_str = gl_debug_severity_names.at(severity);

		LOG << "(GL debug: " << id << ", " << src_str << ", " << type_str << ", " << sev_str << ") " << msg;
	}

	GLenum cullModeToGLenum_[static_cast<int>(CullMode::Max)] = {
		/*None        */ gl::FRONT,	// dummy
		/*Front       */ gl::FRONT,
		/*Back        */ gl::BACK,
		/*FrontAndBack*/ gl::FRONT_AND_BACK
	};

	GLenum cullModeToGLenum(CullMode mode)
	{
		return cullModeToGLenum_[static_cast<int>(mode)];
	}

	GLenum fillModeToGLenum(PolygonFillMode fillMode)
	{
		if (fillMode == PolygonFillMode::Wireframe) {
			return gl::LINE;
		}
		else {
			return gl::FILL;
		}
	}

	GLenum primitiveTypeToGLenum_[static_cast<int>(PrimitiveType::Max)] =
	{
		/*Point        */ gl::POINTS,
		/*Line         */ gl::LINES,
		/*Triangle     */ gl::TRIANGLES,
		/*TriangleStrip*/ gl::TRIANGLE_STRIP
	};
	
	GLenum primitiveTypeToGLenum(PrimitiveType type)
	{
		return primitiveTypeToGLenum_[static_cast<int>(type)];
	}


	enum class DrawCommand
	{
		Draw,
		DrawIndexed,
		DrawIndexedInstanced,
	};

	const int kMaxVertexBufferBindings = 16;
	const int kMaxTextureUnits = 16;
	const int kMaxConstantBufferBindings = 16;

	//
	// Une 'Submission' est une commande de rendu que l'on passe au Renderer par l'intermédiaire d'une RenderQueue.
	// Ces commandes ne sont pas exécutées immédiatement. Elles sont stockées dans des listes (les RenderQueues)
	// puis triées pour optimiser le rendu (par exemple: tri par shader, par profondeur, etc.)
	// Ces objets contiennent tous les états nécessaires pour lancer une commande de rendu, à savoir:
	//	- l'effect (les shaders)
	//  - les paramètres (matériaux)
	//  - la transformation (model->world)
	//  - les vertex/index buffers à binder
	//  - le vertex layout
	//  - la submesh (start index & vertex)
	//  - le type de primitives (tri, lines, tristrip, etc.)
	//  - ...
	//
	// ISSUE: how many constant buffers?
	// how many textures?
	// how many sampler states?
	//
	// ISSUE: 
	// 856 bytes per submission
	// -> one millon submissions = 856 000 000 bytes = 856 MB
	struct Submission
	{
		// default-constructible, moveable, copyable, etc...
		std::array<const Buffer*, kMaxVertexBufferBindings> VB;
		std::array<int, kMaxVertexBufferBindings> offsets;
		std::array<int, kMaxVertexBufferBindings> strides;
		const Buffer *IB;
		const VertexLayout *vertexLayout;
		std::array<const Buffer*, kMaxConstantBufferBindings> CB;
		std::array<const Texture*, kMaxTextureUnits> textures;
		std::array<SamplerDesc, kMaxTextureUnits> samplers;
		const Shader *shader;
		RasterizerDesc rasterizerState;
		DepthStencilDesc depthStencilState;
		DrawCommand cmd;
		PrimitiveType primitiveType;
		int instanceCount;
		int startVertex;
		int numVertices;
		int startIndex;
		int numIndices;
	};
}

class Renderer::RendererImpl
{
public:
	RendererImpl(Window &window_) : window(window_)
	{
		setDefaultState();
		setDebugCallback();
	}

	void setDefaultState()
	{
		for (int i = 0; i < kMaxConstantBufferBindings; ++i) {
			current.CB[i] = nullptr;
		}
		for (int i = 0; i < kMaxVertexBufferBindings; ++i) {
			current.VB[i] = nullptr;
			current.offsets[i] = 0;
			current.strides[i] = 0;
		}
		current.IB = nullptr;
		current.indexBufferFormat = ElementFormat::Uint16;
		current.vertexLayout = nullptr;
		for (int i = 0; i < kMaxTextureUnits; ++i) {
			current.textures[i] = nullptr;
			current.samplers[i].addrU = TextureAddressMode::Clamp;
			current.samplers[i].addrV = TextureAddressMode::Clamp;
			current.samplers[i].addrW = TextureAddressMode::Clamp;
			current.samplers[i].magFilter = TextureFilter::Nearest;
			current.samplers[i].minFilter = TextureFilter::Nearest;
		}
		current.rasterizerState.cullMode = CullMode::None;
		current.rasterizerState.fillMode = PolygonFillMode::Fill;
		current.rasterizerState.depthClipEnable = true;

		current.depthStencilState.depthTestEnable = true;
		current.depthStencilState.depthWriteEnable = true;

		current.cmd = DrawCommand::Draw;
		current.primitiveType = PrimitiveType::Triangle;
		current.instanceCount = 0;
		current.startVertex = 0;
		current.numVertices = 0;
		current.startIndex = 0;
		current.numIndices = 0;
	}

	const RenderTarget *getScreenRenderTarget() const
	{
		return &screenRenderTarget;
	}

	const RenderTarget *getScreenDepthRenderTarget() const
	{
		return &depthRenderTarget;
	}

	void setVertexBuffer(
		int slot,
		const Buffer *buffer,
		int offset,
		int stride
		)
	{
		current.VB[slot] = buffer;
		current.offsets[slot] = offset;
		current.strides[slot] = stride;
	}

	/*void setVertexBuffers(
		int firstInputSlot,
		std::array_ref<const Buffer*> buffers,
		std::array_ref<int> offsets,
		std::array_ref<int> strides
		)
	{

	}*/

	void setIndexBuffer(
		const Buffer *indexBuffer,
		ElementFormat format
		)
	{
		current.IB = indexBuffer;
		current.indexBufferFormat = format;
	}

	void setInputLayout(
		const VertexLayout *layout
		)
	{
		current.vertexLayout = layout;
	}

	void setConstantBuffer(
		int slot,
		const Buffer* buffer
		)
	{
		assert(slot < kMaxConstantBufferBindings);
		current.CB[slot] = buffer;
	}

	/*void setConstantBuffers(
		int firstInputSlot,
		std::array_ref<const Buffer*> buffers
		)
	{

	}*/

	void setTexture(
		int texunit,
		const Texture *texture
		)
	{
		current.textures[texunit] = texture;
	}

	void setShader(
		const Shader *shader
		)
	{
		current.shader = shader;
	}

	void setRasterizerState(
		RasterizerDesc desc
		)
	{
		current.rasterizerState = desc;
	}

	void setDepthStencilState(
		DepthStencilDesc desc
		)
	{
		current.depthStencilState = desc;
	}

	void setSamplerState(
		int texUnit,
		const SamplerDesc &samplerDesc
		)
	{
		current.samplers[texUnit] = samplerDesc;
	}

	//=============  RENDER TARGET COMMANDS =============

	// issue a clear color command
	void clearColor(
		float r,
		float g,
		float b,
		float a
		)
	{
		gl::ClearColor(r, g, b, a);
		gl::Clear(gl::COLOR_BUFFER_BIT);
	}

	// issue a clear depth command
	void clearDepth(
		float z
		)
	{
		gl::ClearDepth(z);
		gl::Clear(gl::DEPTH_BUFFER_BIT);
	}

	// set the color & depth render targets
	void setRenderTargets(
		std::array_ref<const RenderTarget*> colorTargets,
		const RenderTarget *depthStencilTarget
		)
	{
		assert(colorTargets.size() != 0);
		// TODO hardcoded
		assert(colorTargets.size() <= 8);

		// hmmm...
		if (colorTargets[0] == &screenRenderTarget)
		{
			gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
		}
		else
		{
			gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);
			for (int i = 0; i < colorTargets.size(); ++i)
			{
				auto rt = colorTargets[i];
				if (rt->layer != -1)
					// bind all layers
					gl::FramebufferTexture(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + i, rt->texture->id, rt->mipLevel);
				else
					// bind a layer (face) of a cubemap or (TODO) a layer of a texture array
					gl::FramebufferTextureLayer(gl::FRAMEBUFFER, gl::COLOR_ATTACHMENT0 + i, rt->texture->id, rt->mipLevel, rt->layer);
			}
			gl::FramebufferTexture(gl::FRAMEBUFFER, gl::DEPTH_ATTACHMENT, depthStencilTarget->texture->id, depthStencilTarget->mipLevel);
			// check fb completeness
			GLenum err;
			err = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
			assert(err == gl::FRAMEBUFFER_COMPLETE);
			// enable draw buffers
			static const GLenum drawBuffers[8] = {
				gl::COLOR_ATTACHMENT0,
				gl::COLOR_ATTACHMENT0 + 1,
				gl::COLOR_ATTACHMENT0 + 2,
				gl::COLOR_ATTACHMENT0 + 3,
				gl::COLOR_ATTACHMENT0 + 4,
				gl::COLOR_ATTACHMENT0 + 5,
				gl::COLOR_ATTACHMENT0 + 6,
				gl::COLOR_ATTACHMENT0 + 7
			};
			gl::DrawBuffers(colorTargets.size(), drawBuffers);
		}
	}

	void setViewports(
		std::array_ref<Viewport2> viewports
		)
	{
		float vp[4] = {
			viewports[0].topLeftX,
			viewports[0].topLeftY,
			viewports[0].width,
			viewports[0].height
		};
		gl::Viewport(viewports[0].topLeftX, viewports[0].topLeftY, viewports[0].width, viewports[0].height);
		//gl::DepthRangeIndexed(0, viewports[0].minDepth, viewports[0].maxDepth);
		//gl::DepthRangeIndexed(0, 0.0f, 1.0f);

		// TODO more than 1 viewport
		/*for (int i = 0; i < viewports.size(); ++i) {
			float vp[4] = {
				viewports[i].topLeftX,
				viewports[i].topLeftY,
				viewports[i].width,
				viewports[i].height
			};
			gl::ViewportIndexedfv(i, vp);
			gl::DepthRangeIndexed(i, viewports[i].minDepth, viewports[i].maxDepth);
		}*/
	}

	//============= DRAW COMMANDS =============

	// submit vertices for rendering
	void draw(
		PrimitiveType primitiveType_,
		int startVertex_,
		int numVertices_
		)
	{
		current.cmd = DrawCommand::Draw;
		current.primitiveType = primitiveType_;
		current.startVertex = startVertex_;
		current.numVertices = numVertices_;
	}

	void drawIndexed(
		PrimitiveType primitiveType_,
		int startIndex_,
		int numIndices_,
		int baseVertex_
		)
	{
		current.cmd = DrawCommand::DrawIndexed;
		current.primitiveType = primitiveType_;
		current.startVertex = baseVertex_;
		current.startIndex = startIndex_;
		current.numIndices = numIndices_;
	}

	void drawIndexedInstanced(
		PrimitiveType primitiveType_,
		int baseInstance_,
		int numInstances_,
		int startIndex_,
		int numIndices_,
		int baseVertex_
		)
	{
		current.cmd = DrawCommand::DrawIndexedInstanced;
		// TODO
	}

	//============= SUBMIT =============

	// return submission index
	int createSubmission()
	{
		Submission sub = {
			current.VB, current.offsets, current.strides, current.IB, current.vertexLayout,
			current.CB, current.textures, current.samplers, current.shader, current.rasterizerState,
			current.depthStencilState, current.cmd, current.primitiveType, current.instanceCount,
			current.startVertex, current.numVertices, current.startIndex, current.numIndices
		};
		submissions.push_back(sub);
		setDefaultState();
		return submissions.size() - 1;
	}

	// submit
	void submit(int submissionId)
	{
		const auto &item = submissions[submissionId];

		// vertex layout
		gl::BindVertexArray(item.vertexLayout->vao);

		// TODO multi-bind
		// bind vertex buffers
		for (int i = 0; i < item.VB.size(); ++i) {
			if (item.VB[i])
				gl::BindVertexBuffer(i, item.VB[i]->id, item.offsets[i], item.strides[i]);
			else
				gl::BindVertexBuffer(i, 0, 0, 0);
		}

		// IB
		if (item.IB) {
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, item.IB->id);
		}

		// TODO multi-bind
		// TODO cache sampler in submission
		// set sampler states
		// set texture bindings
		for (int i = 0; i < item.samplers.size(); ++i) {
			if (item.textures[i]) {
				auto sam = getSampler(item.samplers[i]);
				gl::BindSampler(i, sam->id);
				gl::BindTexture(gl::TEXTURE0 + i, item.textures[i]->id);
			}
		}

		// shaders
		gl::UseProgram(item.shader->id);

		// TODO multi-bind
		// Constant buffers
		for (int i = 0; i < item.CB.size(); ++i) {
			if (item.CB[i]) 
				gl::BindBufferRange(gl::UNIFORM_BUFFER, i, item.CB[i]->id, 0, item.CB[i]->size);
			else 
				gl::BindBufferRange(gl::UNIFORM_BUFFER, i, 0, 0, 0);
		}

		// Rasterizer
		if (item.rasterizerState.cullMode == CullMode::None) {
			gl::Disable(gl::CULL_FACE);
		}
		else {
			gl::Enable(gl::CULL_FACE);
			gl::CullFace(cullModeToGLenum(item.rasterizerState.cullMode));
		}
		//gl::PolygonMode(gl::FRONT_AND_BACK, fillModeToGLenum(item.rasterizerState.fillMode)); 
		gl::PolygonMode(gl::FRONT_AND_BACK, gl::LINE);
		// TODO depth clip enable

		// TODO Depth stencil

		// draw command
		GLenum mode = primitiveTypeToGLenum(item.primitiveType);
		switch (item.cmd)
		{
		case DrawCommand::Draw:
			gl::DrawArrays(mode, item.startVertex, item.numVertices);
			break;
		case DrawCommand::DrawIndexed:
			gl::DrawElementsBaseVertex(mode, item.numIndices, gl::UNSIGNED_SHORT, reinterpret_cast<void*>(item.startIndex), item.startVertex);
			break;
		case DrawCommand::DrawIndexedInstanced:
			// TODO
			break;
		}
	}

private:
	Sampler *getSampler(SamplerDesc desc)
	{
		auto &ins = samplerCache.insert(std::pair<SamplerDesc, std::unique_ptr<Sampler> >(desc, nullptr));
		auto &res = ins.first;
		if (ins.second) {
			res->second = std::make_unique<Sampler>(desc);
		}
		return res->second.get();
	}

	void setDebugCallback()
	{
		gl::DebugMessageCallback(debugCallback, nullptr);
		gl::DebugMessageControl(gl::DONT_CARE, gl::DONT_CARE, gl::DONT_CARE, 0, nullptr, true);
		gl::DebugMessageInsert(
			gl::DEBUG_SOURCE_APPLICATION,
			gl::DEBUG_TYPE_MARKER,
			1111,
			gl::DEBUG_SEVERITY_NOTIFICATION, -1,
			"Started logging OpenGL messages");
	}

	RenderTarget screenRenderTarget;
	RenderTarget depthRenderTarget;
	GLuint fbo;

	//-----------------------------
	// current state
	struct {
		std::array<const Buffer*, kMaxVertexBufferBindings> VB;
		std::array<int, kMaxVertexBufferBindings> offsets;
		std::array<int, kMaxVertexBufferBindings> strides;
		const Buffer *IB;
		ElementFormat indexBufferFormat;
		const VertexLayout *vertexLayout;
		std::array<const Buffer*, kMaxConstantBufferBindings> CB;
		std::array<const Texture*, kMaxTextureUnits> textures;
		std::array<SamplerDesc, kMaxTextureUnits> samplers;
		const Shader *shader;
		RasterizerDesc rasterizerState;
		DepthStencilDesc depthStencilState;
		DrawCommand cmd;
		PrimitiveType primitiveType;
		int instanceCount;
		int startVertex;
		int numVertices;
		int startIndex;
		int numIndices;
	} current;

	//-----------------------------
	// submission pool
	std::vector<Submission> submissions;

	//-----------------------------
	// sampler state cache
	std::unordered_map<SamplerDesc, std::unique_ptr<Sampler> > samplerCache;

	//-----------------------------
	// window
	Window &window;
};

//=============================================================================
Renderer::Renderer(Window &window_) : impl(std::make_unique<RendererImpl>(window_))
{
}

Renderer::~Renderer()
{
}

const RenderTarget *Renderer::getScreenRenderTarget() const
{
	return impl->getScreenRenderTarget();
}

const RenderTarget *Renderer::getScreenDepthRenderTarget() const
{
	return impl->getScreenDepthRenderTarget();
}

void Renderer::setVertexBuffer(
	int slot,
	const Buffer *buffer,
	int offset,
	int strides
	)
{
	impl->setVertexBuffer(slot, buffer, offset, strides);
}

void Renderer::setIndexBuffer(
	const Buffer *indexBuffer,
	ElementFormat format
	)
{
	impl->setIndexBuffer(indexBuffer, format);
}

void Renderer::setInputLayout(
	const VertexLayout *layout
	)
{
	impl->setInputLayout(layout);
}

void Renderer::setConstantBuffer(
	int slot,
	const Buffer* buffer
	)
{
	impl->setConstantBuffer(slot, buffer);
}

void Renderer::setRasterizerState(
	RasterizerDesc desc
	)
{
	impl->setRasterizerState(desc);
}

void Renderer::setDepthStencilState(
	DepthStencilDesc desc
	)
{
	impl->setDepthStencilState(desc);
}

void Renderer::setTexture(
	int texunit,
	const Texture *texture
	)
{
	impl->setTexture(texunit, texture);
}

void Renderer::setShader(
	const Shader *shader
	)
{
	impl->setShader(shader);
}

void Renderer::setSamplerState(
	int texUnit,
	const SamplerDesc &samplerDesc
	)
{
	impl->setSamplerState(texUnit, samplerDesc);
}

void Renderer::draw(
	PrimitiveType primitiveType,
	int startVertex,
	int numVertices
	)
{
	impl->draw(primitiveType, startVertex, numVertices);
}

void Renderer::drawIndexed(
	PrimitiveType primitiveType,
	int startIndex,
	int numIndices,
	int baseVertex
	)
{
	impl->drawIndexed(primitiveType, startIndex, numIndices, baseVertex);
}

void Renderer::drawIndexedInstanced(
	PrimitiveType primitiveType,
	int baseInstance,
	int numInstances,
	int startIndex,
	int numIndices,
	int baseVertex
	)
{
	impl->drawIndexedInstanced(primitiveType, baseInstance, numInstances, startIndex, numIndices, baseVertex);
}

void Renderer::clearColor(
	float r,
	float g,
	float b,
	float a
	)
{
	impl->clearColor(r, g, b, a);
}

void Renderer::clearDepth(
	float z
	)
{
	impl->clearDepth(z);
}

void Renderer::setRenderTargets(
	std::array_ref<const RenderTarget*> colorTargets,
	const RenderTarget *depthStencilTarget
	)
{
	impl->setRenderTargets(colorTargets, depthStencilTarget);
}

void Renderer::setViewports(
	std::array_ref<Viewport2> viewports
	)
{
	impl->setViewports(viewports);
}

int Renderer::createSubmission()
{
	return impl->createSubmission();
}

void Renderer::submit(int submissionId)
{
	impl->submit(submissionId);
}
