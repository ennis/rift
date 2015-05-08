#include <gl4/renderer.hpp>
#include <log.hpp>

namespace gl4
{
	namespace
	{
		void linkProgram(GLuint program)
		{
			GLint status = gl::TRUE_;
			GLint logsize = 0;

			gl::LinkProgram(program);
			gl::GetProgramiv(program, gl::LINK_STATUS, &status);
			gl::GetProgramiv(program, gl::INFO_LOG_LENGTH, &logsize);
			if (status != gl::TRUE_) {
				ERROR << "Link error:";
				if (logsize != 0) {
					char *logbuf = new char[logsize];
					gl::GetProgramInfoLog(program, logsize, &logsize, logbuf);
					ERROR << logbuf;
					delete[] logbuf;
				}
				else {
					ERROR << "<no log>";
				}
				throw std::runtime_error("link failed");
			}
		}
	}

	PipelineState::PipelineState(
		const Shader *vertexShader,
		const Shader *geometryShader,
		const Shader *pixelShader,
		const RasterizerDesc &rasterizerState,
		const DepthStencilDesc &depthStencilState,
		const BlendStateRenderTargetDesc &blendState)
	{
		//cache_id = shader_cache_index++;
		ds_state = depthStencilState;
		rs_state = rasterizerState;
		om_state = blendState;

		program = gl::CreateProgram();
		assert(vertexShader && pixelShader);
		gl::AttachShader(program, vertexShader->shader);
		gl::AttachShader(program, pixelShader->shader);
		if (geometryShader)
			gl::AttachShader(program, geometryShader->shader);
		linkProgram(program);
		gl::DetachShader(program, vertexShader->shader);
		gl::DetachShader(program, pixelShader->shader);

		// query some informations
		/*int num_ubo = 0;
		gl::GetProgramInterfaceiv(program, gl::UNIFORM_BLOCK, gl::ACTIVE_RESOURCES, &num_ubo);
		GLenum bufferProps[] = { gl::BUFFER_BINDING, gl::BUFFER_DATA_SIZE };
		for (auto block_index = 0; block_index < num_ubo; ++block_index) {
		int len;
		int val[2];
		gl::GetProgramResourceiv(
		program,
		gl::UNIFORM_BLOCK,
		block_index,
		2,
		bufferProps,
		2 * sizeof(int),
		&len,
		val);
		LOG << "Block index #" << block_index << ": slot=" << val[0] << ", size=" << val[1];
		}

		int num_uniforms = 0;
		gl::GetProgramInterfaceiv(program, gl::UNIFORM, gl::ACTIVE_RESOURCES, &num_uniforms);
		GLenum uniformProps[] = { gl::TYPE, gl::LOCATION };
		for (auto uindex = 0; uindex < num_uniforms; ++uindex) {
		int len;
		int val[2];
		gl::GetProgramResourceiv(
		program,
		gl::UNIFORM,
		uindex,
		2,
		uniformProps,
		2 * sizeof(int),
		&len,
		val);
		LOG << "Uniform index #" << uindex << ": type=" << val[0] << ", location=" << val[1];
		if (isSamplerType(val[0])) {
		int tex_unit;
		gl::GetUniformiv(program, val[1], &tex_unit);
		LOG << " -- Sampler bound to texture unit " << tex_unit;
		}
		}*/
	}
}