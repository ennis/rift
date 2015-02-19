#ifndef RENDERQUEUE_HPP
#define RENDERQUEUE_HPP

#include <renderer2.hpp>
#include <vector>

const unsigned int kMaxBuckets = 16;
const unsigned int kMaxColorRenderTargets = 8;
const unsigned int kMaxViewports = 16;

// class RenderQueue
// Gère les listes de Submissions (tri)
class RenderQueue
{
public:
	RenderQueue(Renderer &renderer);
	// noncopyable
	RenderQueue(const RenderQueue&) = delete;
	RenderQueue &operator=(const RenderQueue &) = delete;
	// move-constructible
	RenderQueue(RenderQueue &&rhs) = delete;
	// moveable
	RenderQueue &operator=(RenderQueue &&rhs) = delete;

	// clear the list of submissions (without rendering them)
	void clear();


	//============= BUCKET COMMANDS =============

	// issue a clear color command at the beginning of the bucket
	RenderQueue &clearColor(
		unsigned int bucket,
		float r, 
		float g, 
		float b, 
		float a
	);

	// issue a clear depth command at the beginning of the bucket
	RenderQueue &clearDepth(
		unsigned int bucket,
		float z
	);

	// set the color & depth render targets
	RenderQueue &setRenderTargets(
		unsigned int bucket,
		std::array_ref<const RenderTarget*> colorTargets,
		const RenderTarget *depthStencilTarget
		);

	RenderQueue &setViewports(
		unsigned int bucket,
		std::array_ref<Viewport2> viewports
		);

	//============= SUBMIT =============

	// submit the command to specified bucket
	RenderQueue &submit(
		unsigned int submission, 
		unsigned int bucket
		);

	// flush the render queue (execute the commands)
	RenderQueue &flush(); 
	
	void debugPrint();

private:
	struct RenderItem
	{
		uint64_t sortKey;
		unsigned int submissionId;
	};

	struct Bucket
	{
		Bucket() 
		{
			for (unsigned int i = 0; i < colorTargets.size(); ++i)
				colorTargets[i] = nullptr;
			depthStencilTarget = nullptr;
			flags = 0;
		}

		enum Flags {
			ClearColor = (1 << 0),
			ClearDepth = (1 << 1)
		};

		std::array<const RenderTarget*, kMaxColorRenderTargets> colorTargets;
		std::array<Viewport2, kMaxViewports> viewports;
		const RenderTarget *depthStencilTarget;
		unsigned long flags;
		float clearColor[4];
		float clearDepth;
	};

	Renderer *renderer;
	// array of (sorted) render items
	std::vector<RenderItem> renderItems;

	std::array<Bucket, kMaxBuckets> buckets;
};

 
#endif /* end of include guard: RENDERQUEUE_HPP */