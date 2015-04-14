#include <gl4/renderer.hpp>
#include <log.hpp>

namespace gl4
{
	Stream::Stream(BufferUsage usage_, size_t size_, unsigned num_buffers) 
	{
		// align size
		buffer_size = (size_ + 256u - 1) & ~((size_t)256u - 1);
		auto total_size = buffer_size * num_buffers;
		ranges.resize(num_buffers);
		LOG << "Allocating stream of size " << buffer_size << "x" << num_buffers;
		gl::GenBuffers(1, &buffer_object);
		buffer_target = gl4::detail::bufferUsageToBindingPoint(usage_);
		gl::BindBuffer(buffer_target, buffer_object);
		gl::BufferStorage(buffer_target, total_size, nullptr, gl::MAP_WRITE_BIT | gl::MAP_PERSISTENT_BIT | gl::MAP_COHERENT_BIT);
		gl::BindBuffer(buffer_target, 0);
		// init ranges
		for (auto i = 0u; i < num_buffers; ++i) 
		{
			ranges[i].sync = nullptr;
		}
		current_offset = 0;
		current_size = 0;
		current_range = 0; 
		mapped_ptr = gl::MapNamedBufferRangeEXT(buffer_object, 0, buffer_size,
			gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
			gl::MAP_PERSISTENT_BIT | // persistent mapping
			gl::MAP_COHERENT_BIT | // coherent
			gl::MAP_WRITE_BIT | // write-only (no readback)
			gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
			);
	}

	/*void Stream::remap()
	{
	mapped_ptr = gl::MapNamedBufferRangeEXT(buffer_object, 0, buffer_size,
	gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
	gl::MAP_PERSISTENT_BIT | // persistent mapping
	gl::MAP_COHERENT_BIT | // coherent
	gl::MAP_WRITE_BIT | // write-only (no readback)
	gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
	);
	}*/

	void *Stream::reserve(size_t size)
	{
		assert(size <= buffer_size);

		if (size == 0) {
			return nullptr;
		}

		// still have a fence?
		if (ranges[current_range].sync != nullptr) {
			// yes, synchronize
			auto result = gl::ClientWaitSync(ranges[current_range].sync, gl::SYNC_FLUSH_COMMANDS_BIT, 0);
			if (result == gl::TIMEOUT_EXPIRED) {
				// We want absolutely no stalls
				// TODO handle this?
				assert(!"Buffer stall");
			}
			// Ok, the buffer is unused
			ranges[current_range].sync = nullptr;
		}

		// ewww
		auto ptr = reinterpret_cast<void*>(current_offset + current_size + 256);
		auto space = buffer_size - (current_offset + current_size);
		if (!std::align(256, size, ptr, space)) {
			ERROR << "Out of space in current buffer range";
			assert(false);
		}
		// buffer is unlocked and has enough space: reserve
		current_offset = buffer_size - space;
		// TODO align allocated space!!
		current_size = size;
		return reinterpret_cast<char*>(mapped_ptr) + current_range * buffer_size + current_offset;
	}

	void Stream::fence(RenderQueue2 &renderQueue)
	{
		// go to the next buffer
		renderQueue.fenceSync(ranges[current_range].sync);
		current_range = (current_range + 1) % ranges.size();
		current_offset = 0;
		current_size = 0;
	}

	void RenderQueue2::beginCommand()
	{
		std::memset(&state, 0, sizeof(state));
	}

	void RenderQueue2::setVertexBuffers(
		util::array_ref<BufferDesc> vertex_buffers, 
		const InputLayout &layout)
	{
		for (auto i = 0u; i < vertex_buffers.size(); ++i)
		{
			auto index = state.u.drawCommand.num_vertex_buffers++;
			auto &dc = state.u.drawCommand;
			dc.vertex_buffers[index] = vertex_buffers[i].buffer;
			dc.vertex_buffers_offsets[index] = vertex_buffers[i].offset;
			dc.vertex_buffers_strides[index] = layout.strides[i];
		}
		setInputLayout(layout);
	}

	void RenderQueue2::setIndexBuffer(const BufferDesc &index_buffer)
	{
		state.u.drawCommand.index_buffer = index_buffer.buffer;
		state.u.drawCommand.index_buffer_offset = index_buffer.offset;
	}

	void RenderQueue2::setUniformBuffers(util::array_ref<BufferDesc> uniform_buffers)
	{
		for (auto i = 0u; i < uniform_buffers.size(); ++i)
		{
			auto index = state.u.drawCommand.num_uniform_buffers++;
			state.u.drawCommand.uniform_buffers[index] = uniform_buffers[i].buffer;
			state.u.drawCommand.uniform_buffers_offsets[index] = uniform_buffers[i].offset;
			state.u.drawCommand.uniform_buffers_sizes[index] = uniform_buffers[i].size;
		}
	}

	void RenderQueue2::setTexture2D(int unit, const Texture2D &tex, const SamplerDesc &samplerDesc)
	{
		auto index = state.u.drawCommand.num_textures++;
		state.u.drawCommand.textures[index] = tex.id;
		state.u.drawCommand.samplers[index] = Renderer::getInstance().getSampler(samplerDesc);
	}		
	
	void RenderQueue2::setTextureCubeMap(int unit, const TextureCubeMap &tex, const SamplerDesc &samplerDesc)
	{
		auto index = state.u.drawCommand.num_textures++;
		state.u.drawCommand.textures[index] = tex.id;
		state.u.drawCommand.samplers[index] = Renderer::getInstance().getSampler(samplerDesc);
	}

	void RenderQueue2::setShader(const Shader &shader)
	{
		// TODO rename to pipeline state
		state.u.drawCommand.shader = &shader;
	}

	void RenderQueue2::draw(
		PrimitiveType primitiveType,
		unsigned firstVertex,
		unsigned vertexCount,
		unsigned firstInstance,
		unsigned instanceCount)
	{
		// TODO clean this (be type-safe)
		// => flexible command buffer
		state.u.drawCommand.first_vertex = firstVertex;
		state.u.drawCommand.vertex_count = vertexCount;
		state.u.drawCommand.index_count = 0;
		state.u.drawCommand.first_instance = firstInstance;
		state.u.drawCommand.instance_count = instanceCount;
		state.u.drawCommand.mode = detail::primitiveTypeToGLenum(primitiveType);
		state.type = RenderItem2::Type::DrawCommand;
		// end command
		// TODO no-copy
		render_items.push_back(state);
	}

	void RenderQueue2::drawIndexed(
		PrimitiveType primitiveType,
		unsigned firstIndex,
		unsigned indexCount,
		int vertexOffset,
		unsigned firstInstance,
		unsigned instanceCount)
	{
		state.u.drawCommand.first_index = firstIndex;
		state.u.drawCommand.first_vertex = vertexOffset;
		state.u.drawCommand.index_count = indexCount;
		state.u.drawCommand.vertex_count = 0;
		state.u.drawCommand.first_instance = firstInstance;
		state.u.drawCommand.instance_count = instanceCount;
		state.u.drawCommand.mode = detail::primitiveTypeToGLenum(primitiveType);
		state.type = RenderItem2::Type::DrawCommand;
		render_items.push_back(state);
	}

	void RenderQueue2::fenceSync(GLsync &out_sync)
	{
		state.u.fence.sync = &out_sync;
		state.type = RenderItem2::Type::FenceSync;
		render_items.push_back(state);
	}

	void RenderQueue2::setInputLayout(const InputLayout &layout)
	{
		state.u.drawCommand.input_layout = &layout;
	}

	void Renderer::commit(
		RenderQueue2 &renderQueue2
		)
	{
		for (const auto &ri : renderQueue2.render_items) {
			drawItem2(ri);
		}
	}

	void Renderer::drawItem2(const RenderItem2 &item)
	{

		if (item.type == RenderItem2::Type::DrawCommand)
		{
			auto &dc = item.u.drawCommand;
			if (item.u.drawCommand.input_layout != nullptr) 
			{
				gl::BindVertexArray(dc.input_layout->vao);
				gl::BindVertexBuffers(0, 
					dc.num_vertex_buffers, 
					dc.vertex_buffers, 
					dc.vertex_buffers_offsets, 
					dc.vertex_buffers_strides);
			}
			else {
				// fully procedural (no VBO)
				gl::BindVertexArray(dummy_vao);
			}

			gl::BindBuffersRange(
				gl::UNIFORM_BUFFER,
				0,
				dc.num_uniform_buffers,
				dc.uniform_buffers,
				dc.uniform_buffers_offsets,
				dc.uniform_buffers_sizes
				);

			if (dc.num_textures)
			{
				gl::BindTextures(0, dc.num_textures, dc.textures);
				gl::BindSamplers(0, dc.num_textures, dc.samplers);
			}

			// shaders
			gl::UseProgram(dc.shader->program);
			// Rasterizer
			if (dc.shader->rs_state.cullMode == CullMode::None) {
				gl::Disable(gl::CULL_FACE);
			}
			else {
				gl::Enable(gl::CULL_FACE);
				gl::CullFace(detail::cullModeToGLenum(dc.shader->rs_state.cullMode));
			}
			gl::PolygonMode(gl::FRONT_AND_BACK, detail::fillModeToGLenum(dc.shader->rs_state.fillMode));
			gl::Enable(gl::DEPTH_TEST);
			if (!dc.shader->ds_state.depthTestEnable)
				gl::DepthFunc(gl::ALWAYS);
			else
				gl::DepthFunc(gl::LEQUAL);
			if (dc.shader->ds_state.depthWriteEnable)
				gl::DepthMask(gl::TRUE_);
			else
				gl::DepthMask(gl::FALSE_);

			// OM / blend state
			// XXX this ain't cheap
			// TODO blend state per color buffer
			gl::Enable(gl::BLEND);
			gl::BlendEquationSeparatei(
				0,
				detail::blendOpToGL(dc.shader->om_state.rgbOp),
				detail::blendOpToGL(dc.shader->om_state.alphaOp));
			gl::BlendFuncSeparatei(
				0,
				detail::blendFactorToGL(dc.shader->om_state.rgbSrcFactor),
				detail::blendFactorToGL(dc.shader->om_state.rgbDestFactor),
				detail::blendFactorToGL(dc.shader->om_state.alphaSrcFactor),
				detail::blendFactorToGL(dc.shader->om_state.alphaDestFactor));

			if (dc.index_count) {
				gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, dc.index_buffer);
				gl::DrawElementsInstancedBaseVertexBaseInstance(
					dc.mode,
					dc.index_count,
					gl::UNSIGNED_SHORT,
					reinterpret_cast<void*>(dc.index_buffer_offset + dc.first_index * 2),
					dc.instance_count,
					dc.first_vertex,
					dc.first_instance);
			}
			else {
				gl::DrawArraysInstancedBaseInstance(
					dc.mode,
					dc.first_vertex,
					dc.vertex_count,
					dc.instance_count,
					dc.first_instance);
			}
		}
	}

}