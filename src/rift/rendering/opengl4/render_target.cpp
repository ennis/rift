#include <rendering/opengl4/opengl4.hpp>

namespace gl4
{

	RenderTarget::RenderTarget() :
		fbo(0),
		depth_target(nullptr)
	{
	}

	RenderTarget::RenderTarget(
		glm::ivec2 size_,
		util::array_ref<ElementFormat> colorTargetFormats) :
		size(size_)
	{
		assert(colorTargetFormats.size() < 8);
		for (auto format : colorTargetFormats) {
			checkForUnusualColorFormats(format);
			color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
		}
		init();
	}

	RenderTarget::RenderTarget(
		glm::ivec2 size_,
		util::array_ref<ElementFormat> colorTargetFormats,
		ElementFormat depthTargetFormat) :
		size(size_),
		depth_target(nullptr)
	{
		assert(colorTargetFormats.size() < 8);
		for (auto format : colorTargetFormats) {
			checkForUnusualColorFormats(format);
			color_targets.push_back(Texture2D::create(size, 1, format, nullptr));
		}
		depth_target = Texture2D::create(size, 1, depthTargetFormat, nullptr);
		init();
	}

	RenderTarget::Ptr RenderTarget::create(
		glm::ivec2 size,
		util::array_ref<ElementFormat> colorTargetFormats,
		ElementFormat depthTargetFormat)
	{
		auto ptr = std::make_unique<RenderTarget>(size, colorTargetFormats, depthTargetFormat);
		return ptr;
	}

	RenderTarget::Ptr RenderTarget::createNoDepth(
		glm::ivec2 size,
		util::array_ref<ElementFormat> colorTargetFormats)
	{
		auto ptr = std::make_unique<RenderTarget>(size, colorTargetFormats);
		return ptr;
	}

	void RenderTarget::init()
	{
		gl::GenFramebuffers(1, &fbo);
		gl::BindFramebuffer(gl::FRAMEBUFFER, fbo);

		for (int i = 0; i < color_targets.size(); ++i)
		{
			auto &&target = color_targets[i];
			gl::FramebufferTexture(
				gl::FRAMEBUFFER,
				gl::COLOR_ATTACHMENT0 + i,
				target->id,
				0);
		}
		gl::FramebufferTexture(
			gl::FRAMEBUFFER,
			gl::DEPTH_ATTACHMENT,
			depth_target->id,
			0);

		// check fb completeness
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

		gl::DrawBuffers(color_targets.size(), drawBuffers);

		GLenum err;
		err = gl::CheckFramebufferStatus(gl::FRAMEBUFFER);
		assert(err == gl::FRAMEBUFFER_COMPLETE);
	}
}