#include <renderer.hpp>
#include <engine.hpp>

namespace gl4
{
	namespace
	{
		struct Command
		{
			enum Opcode
			{
				Draw,
				DrawIndexed,
				DrawProcedural,
				ClearColor,
				ClearDepth,
				Blit,
				SetVertexBuffers,
				SetConstantBuffers,
				SetTextures,
				SetRenderTarget,
				SetScreenRenderTarget,
				SetStencilRef,
				SetPipelineState
			};

			Command(Opcode op_) : op(op_)
			{}

			Opcode op;
		};

		struct CommandContext
		{
			bool updateStencilState = true;
			DepthStencilDesc lastDepthStencilState;
			int stencilRef = 0;

			void prepareDrawCommand()
			{
				// stencil test
				if (lastDepthStencilState.stencilTestEnable)
				{
					gl::Enable(gl::STENCIL_TEST);
					gl::StencilFuncSeparate(
						gl::FRONT, 
						stencilFuncToGLenum(lastDepthStencilState.stencilOpFront.comparisonFunc), 
						stencilRef, 
						lastDepthStencilState.stencilMask);
					gl::StencilFuncSeparate(
						gl::BACK,
						stencilFuncToGLenum(lastDepthStencilState.stencilOpBack.comparisonFunc),
						stencilRef,
						lastDepthStencilState.stencilMask);
					gl::StencilOpSeparate(
						gl::FRONT,
						stencilOpToGLenum(lastDepthStencilState.stencilOpFront.stencilFailOp),
						stencilOpToGLenum(lastDepthStencilState.stencilOpFront.depthFailOp),
						stencilOpToGLenum(lastDepthStencilState.stencilOpFront.passOp));
					gl::StencilOpSeparate(
						gl::BACK,
						stencilOpToGLenum(lastDepthStencilState.stencilOpBack.stencilFailOp),
						stencilOpToGLenum(lastDepthStencilState.stencilOpBack.depthFailOp),
						stencilOpToGLenum(lastDepthStencilState.stencilOpBack.passOp));
				}
				else
				{
					gl::Disable(gl::STENCIL_TEST);
				}
				updateStencilState = false;
			}
		};

		template <Command::Opcode Op>
		struct TCommand : public Command
		{
			TCommand() : Command(Op)
			{}
		};

		struct VertexBufferStateGroup
		{
			GLuint vertex_buffers[kMaxVertexBufferBindings];
			GLintptr vertex_buffers_offsets[kMaxVertexBufferBindings];
			GLsizei vertex_buffers_strides[kMaxVertexBufferBindings];
			const InputLayout *input_layout;
			unsigned num_vertex_buffers;
		};

		struct ConstantBufferStateGroup
		{
			GLuint uniform_buffers[kMaxUniformBufferBindings];
			GLintptr uniform_buffers_offsets[kMaxUniformBufferBindings];
			GLsizeiptr uniform_buffers_sizes[kMaxUniformBufferBindings];
			unsigned num_uniform_buffers;
		};

		struct TextureStateGroup
		{
			GLuint textures[kMaxTextureUnits];
			GLuint samplers[kMaxTextureUnits];
			unsigned num_textures;
		};

		struct CmdSetRenderTarget : public TCommand<Command::SetRenderTarget>
		{
			CmdSetRenderTarget()
			{

			}

			const RenderTarget *render_target;

			void execute(CommandContext& ctx) const
			{
				gl::BindFramebuffer(gl::FRAMEBUFFER, render_target->fbo);
				gl::Viewport(0, 0, render_target->size.x, render_target->size.y);
			}
		};

		struct CmdSetScreenRenderTarget : public TCommand<Command::SetScreenRenderTarget>
		{
			void execute(CommandContext& ctx) const
			{
				gl::BindFramebuffer(gl::FRAMEBUFFER, 0);
				auto screen_size = Engine::instance().getWindow().size();
				gl::Viewport(0, 0, screen_size.x, screen_size.y);
			}
		};

		struct CmdSetVertexBuffers : public TCommand<Command::SetVertexBuffers>
		{
			VertexBufferStateGroup state;
			
			void execute(CommandContext& ctx) const
			{
				gl::BindVertexArray(state.input_layout->vao);
				gl::BindVertexBuffers(0,
					state.num_vertex_buffers,
					state.vertex_buffers,
					state.vertex_buffers_offsets,
					state.vertex_buffers_strides);
			}
		};

		struct CmdSetConstantBuffers : public TCommand<Command::SetConstantBuffers>
		{
			ConstantBufferStateGroup state;

			void execute(CommandContext& ctx) const
			{
				if (state.num_uniform_buffers)
				{
					gl::BindBuffersRange(
						gl::UNIFORM_BUFFER,
						0,
						state.num_uniform_buffers,
						state.uniform_buffers,
						state.uniform_buffers_offsets,
						state.uniform_buffers_sizes
						);
				}
			}
		};

		struct CmdSetTextures : public TCommand<Command::SetTextures>
		{
			TextureStateGroup state;

			void execute(CommandContext& ctx) const
			{
				if (state.num_textures)
				{
					gl::BindTextures(0, state.num_textures, state.textures);
					gl::BindSamplers(0, state.num_textures, state.samplers);
				}
			}
		};

		struct CmdSetPipelineState : public TCommand<Command::SetPipelineState>
		{
			const PipelineState *pipeline_state;

			void execute(CommandContext& ctx) const
			{
				gl::UseProgram(pipeline_state->program);
				if (pipeline_state->rs_state.cullMode == CullMode::None) {
					gl::Disable(gl::CULL_FACE);
				}
				else {
					gl::Enable(gl::CULL_FACE);
					gl::CullFace(cullModeToGLenum(pipeline_state->rs_state.cullMode));
				}
				gl::PolygonMode(gl::FRONT_AND_BACK, fillModeToGLenum(pipeline_state->rs_state.fillMode));
				gl::Enable(gl::DEPTH_TEST);
				if (!pipeline_state->ds_state.depthTestEnable)
					gl::DepthFunc(gl::ALWAYS);
				else
					gl::DepthFunc(gl::LEQUAL);
				if (pipeline_state->ds_state.depthWriteEnable)
					gl::DepthMask(gl::TRUE_);
				else
					gl::DepthMask(gl::FALSE_);

				ctx.lastDepthStencilState = pipeline_state->ds_state;
				ctx.updateStencilState = true;

				// OM / blend state
				// XXX this ain't cheap
				// TODO blend state per color buffer
				gl::Enable(gl::BLEND);
				gl::BlendEquationSeparatei(
					0,
					blendOpToGL(pipeline_state->om_state.rgbOp),
					blendOpToGL(pipeline_state->om_state.alphaOp));
				gl::BlendFuncSeparatei(
					0,
					blendFactorToGL(pipeline_state->om_state.rgbSrcFactor),
					blendFactorToGL(pipeline_state->om_state.rgbDestFactor),
					blendFactorToGL(pipeline_state->om_state.alphaSrcFactor),
					blendFactorToGL(pipeline_state->om_state.alphaDestFactor));
			}
		};

		struct CmdDrawIndexed : public TCommand<Command::DrawIndexed>
		{
			GLuint index_buffer;
			GLintptr index_buffer_offset;
			unsigned first_vertex;
			unsigned first_index;
			unsigned index_count;
			unsigned first_instance;
			unsigned instance_count;
			GLenum mode;

			void execute(CommandContext& ctx) const
			{
				ctx.prepareDrawCommand();
				gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, index_buffer);
				gl::DrawElementsInstancedBaseVertexBaseInstance(
					mode,
					index_count,
					gl::UNSIGNED_SHORT,
					reinterpret_cast<void*>(index_buffer_offset + first_index * 2),
					instance_count,
					first_vertex,
					first_instance);
			}
		};

		struct CmdDraw : public TCommand<Command::Draw>
		{
			unsigned first_vertex;
			unsigned vertex_count;
			unsigned first_instance;
			unsigned instance_count;
			GLenum mode;

			void execute(CommandContext& ctx) const
			{
				ctx.prepareDrawCommand();
				gl::DrawArraysInstancedBaseInstance(
					mode,
					first_vertex,
					vertex_count,
					instance_count,
					first_instance);
			}
		};

		struct CmdDrawProcedural : public TCommand<Command::DrawProcedural>
		{
			unsigned first_vertex;
			unsigned vertex_count;
			unsigned first_instance;
			unsigned instance_count;
			GLenum mode;

			void execute(CommandContext& ctx) const
			{
				ctx.prepareDrawCommand();
				gl::BindVertexArray(dummy_vao);
				gl::DrawArraysInstancedBaseInstance(
					mode,
					first_vertex,
					vertex_count,
					instance_count,
					first_instance);
			}
		};

		struct CmdClearColor : public TCommand<Command::ClearColor>
		{
			float color[4];

			void execute(CommandContext& ctx) const
			{
				gl::ClearColor(color[0], color[1], color[2], color[3]);
				gl::Clear(gl::COLOR_BUFFER_BIT);
			}
		};

		struct CmdClearDepth : public TCommand<Command::ClearDepth>
		{
			float depth;

			void execute(CommandContext& ctx) const
			{
				gl::DepthMask(gl::TRUE_);
				gl::ClearDepth(depth);
				gl::Clear(gl::DEPTH_BUFFER_BIT);
			}
		};

		struct CmdSetStencilRef : public TCommand<Command::SetStencilRef>
		{
			int8_t ref;

			void execute(CommandContext& ctx) const
			{
				ctx.stencilRef = ref;
				ctx.updateStencilState = true;
			}
		};
	}

	void CommandBuffer::write(const void *data, unsigned size)
	{
		void *base = cmdBuf.data();
		size_t bufsize = cmdBuf.size();
		if (writePtr + size > bufsize)
		{
			cmdBuf.resize((bufsize < 4096) ? 4096 : (bufsize * 2));
			base = cmdBuf.data();
		}
		void *ptr = (char*)base + writePtr;
		memcpy(ptr, data, size);
		writePtr += size;
	}

	void CommandBuffer::setRenderTarget(const RenderTarget &rt)
	{
		CmdSetRenderTarget cmd;
		cmd.render_target = &rt;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setScreenRenderTarget()
	{
		CmdSetScreenRenderTarget cmd;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::clearColor(float color[4])
	{
		CmdClearColor cmd;
		std::copy(color, color + 4, cmd.color);
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::clearDepth(float depth)
	{
		CmdClearDepth cmd;
		cmd.depth = depth;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setStencilRef(int8_t ref)
	{
		CmdSetStencilRef cmd;
		cmd.ref = ref;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setVertexBuffers(
		util::array_ref<const Buffer*> vertexBuffers,
		const InputLayout &layout)
	{
		CmdSetVertexBuffers cmd;
		cmd.state.num_vertex_buffers = vertexBuffers.size();
		for (auto i = 0u; i < vertexBuffers.size(); ++i)
		{
			cmd.state.vertex_buffers[i] = vertexBuffers[i]->obj;
			cmd.state.vertex_buffers_offsets[i] = vertexBuffers[i]->offset;
			cmd.state.vertex_buffers_strides[i] = layout.strides[i];
		}
		cmd.state.input_layout = &layout;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setConstantBuffers(
		util::array_ref<const Buffer*> constantBuffers)
	{
		CmdSetConstantBuffers cmd;
		cmd.state.num_uniform_buffers = constantBuffers.size();
		for (auto i = 0u; i < constantBuffers.size(); ++i)
		{
			cmd.state.uniform_buffers[i] = constantBuffers[i]->obj;
			cmd.state.uniform_buffers_offsets[i] = constantBuffers[i]->offset;
			cmd.state.uniform_buffers_sizes[i] = constantBuffers[i]->size;
		}
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setTextures(
		util::array_ref<const Texture*> textures,
		util::array_ref<const Sampler*> samplers)
	{
		CmdSetTextures cmd;
		assert(textures.size() == samplers.size());
		cmd.state.num_textures = textures.size();
		for (auto i = 0u; i < textures.size(); ++i)
		{
			cmd.state.samplers[i] = samplers[i]->id;
			cmd.state.textures[i] = textures[i]->id;
		}
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::setPipelineState(
		const PipelineState *pipelineState)
	{
		CmdSetPipelineState cmd;
		cmd.pipeline_state = pipelineState;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::draw(
		PrimitiveType primitiveType,
		unsigned firstVertex,
		unsigned vertexCount,
		unsigned firstInstance,
		unsigned instanceCount
		)
	{
		CmdDraw cmd;
		cmd.mode = primitiveTypeToGLenum(primitiveType);
		cmd.first_vertex = firstVertex;
		cmd.vertex_count = vertexCount;
		cmd.first_instance = firstInstance;
		cmd.instance_count = instanceCount;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::drawProcedural(
		PrimitiveType primitiveType,
		unsigned firstVertex,
		unsigned vertexCount,
		unsigned firstInstance,
		unsigned instanceCount
		)
	{
		CmdDrawProcedural cmd;
		cmd.mode = primitiveTypeToGLenum(primitiveType);
		cmd.first_vertex = firstVertex;
		cmd.vertex_count = vertexCount;
		cmd.first_instance = firstInstance;
		cmd.instance_count = instanceCount;
		write(&cmd, sizeof(cmd));
	}

	void CommandBuffer::drawIndexed(
		PrimitiveType primitiveType,
		const Buffer &indexBuffer,
		unsigned firstVertex,
		unsigned firstIndex,
		unsigned indexCount,
		unsigned firstInstance,
		unsigned instanceCount
		)
	{
		CmdDrawIndexed cmd;
		cmd.mode = primitiveTypeToGLenum(primitiveType);
		cmd.index_buffer = indexBuffer.obj;
		cmd.index_buffer_offset = indexBuffer.offset;
		cmd.first_vertex = firstVertex;
		cmd.first_index = firstIndex;
		cmd.index_count = indexCount;
		cmd.first_instance = firstInstance;
		cmd.instance_count = instanceCount;
		write(&cmd, sizeof(cmd));
	}

}

namespace Renderer
{
	template <typename T>
	void executeImpl(gl4::CommandContext &ctx, char *&ptr)
	{
		((const T*)ptr)->execute(ctx);
		ptr += sizeof(T);
	}

	void execute(gl4::CommandBuffer &buffer)
	{
		using namespace gl4;
		char *ptr = buffer.cmdBuf.data();
		char *endptr = ptr + buffer.writePtr;
		gl4::CommandContext ctx;
		while (ptr < endptr)
		{
			auto opcode = *(Command::Opcode*)ptr;
			switch (opcode)
			{
			case Command::Draw:
				executeImpl<CmdDraw>(ctx, ptr);
				break;
			case Command::DrawIndexed:
				executeImpl<CmdDrawIndexed>(ctx, ptr);
				break;
			case Command::DrawProcedural:
				executeImpl<CmdDrawProcedural>(ctx, ptr);
				break;
			case Command::ClearColor:
				executeImpl<CmdClearColor>(ctx, ptr);
				break;
			case Command::ClearDepth:
				executeImpl<CmdClearDepth>(ctx, ptr);
				break;
			case Command::Blit:
				//
				break;
			case Command::SetVertexBuffers:
				executeImpl<CmdSetVertexBuffers>(ctx, ptr);
				break;
			case Command::SetConstantBuffers:
				executeImpl<CmdSetConstantBuffers>(ctx, ptr);
				break;
			case Command::SetTextures:
				executeImpl<CmdSetTextures>(ctx, ptr);
				break;
			case Command::SetRenderTarget:
				executeImpl<CmdSetRenderTarget>(ctx, ptr);
				break;
			case Command::SetScreenRenderTarget:
				executeImpl<CmdSetScreenRenderTarget>(ctx, ptr);
				break;
			case Command::SetPipelineState:
				executeImpl<CmdSetPipelineState>(ctx, ptr);
				break;
			}
		}
	}
}