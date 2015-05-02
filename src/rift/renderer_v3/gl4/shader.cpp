#include <renderer.hpp>
#include <log.hpp>

namespace gl4
{
	//=========================================================================
	//=========================================================================
	// GLSL
	//=========================================================================
	//=========================================================================
	GLuint glslCompileShader(const char *shaderSource, GLenum type)
	{
		GLuint obj = gl::CreateShader(type);
		const char *shaderSources[1] = { shaderSource };
		gl::ShaderSource(obj, 1, shaderSources, NULL);
		gl::CompileShader(obj);

		GLint status = gl::TRUE_;
		GLint logsize = 0;

		gl::GetShaderiv(obj, gl::COMPILE_STATUS, &status);
		gl::GetShaderiv(obj, gl::INFO_LOG_LENGTH, &logsize);
		if (status != gl::TRUE_) {
			ERROR << "Compile error:";
			if (logsize != 0) {
				char *logbuf = new char[logsize];
				gl::GetShaderInfoLog(obj, logsize, &logsize, logbuf);
				ERROR << logbuf;
				delete[] logbuf;
				gl::DeleteShader(obj);
			}
			else {
				ERROR << "<no log>";
			}
			throw std::runtime_error("shader compilation failed");
		}

		return obj;
	}

	void glslLinkProgram(GLuint program)
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

	// creates a shader program from vertex and fragment shader source files
	GLuint glslCreateProgram(const char *vertexShaderSource, const char *fragmentShaderSource)
	{
		GLuint vs_obj = glslCompileShader(vertexShaderSource, gl::VERTEX_SHADER);
		GLuint fs_obj = glslCompileShader(fragmentShaderSource, gl::FRAGMENT_SHADER);
		GLuint program_obj = gl::CreateProgram();
		gl::AttachShader(program_obj, vs_obj);
		gl::AttachShader(program_obj, fs_obj);
		glslLinkProgram(program_obj);
		// once the program is linked, no need to keep the shader objects
		gl::DetachShader(program_obj, vs_obj);
		gl::DetachShader(program_obj, fs_obj);
		gl::DeleteShader(vs_obj);
		gl::DeleteShader(fs_obj);
		return program_obj;
	}

	int shader_cache_index = 0;


	Shader::Shader(
		const char *vsSource,
		const char *psSource,
		const RasterizerDesc &rasterizerState,
		const DepthStencilDesc &depthStencilState,
		const BlendDesc &blendState)
	{
		cache_id = shader_cache_index++;
		ds_state = depthStencilState;
		rs_state = rasterizerState;
		om_state = blendState;
		// preprocess source
		program = glslCreateProgram(vsSource, psSource);

		// query some informations
		int num_ubo = 0;
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
		}
	}

}