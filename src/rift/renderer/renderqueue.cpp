#include <renderqueue.hpp>
#include <algorithm>	// std::sort
#include <log.hpp>

namespace
{
	inline uint64_t makeSortKey(unsigned int submission, unsigned int bucket)
	{
		return (static_cast<uint64_t>(bucket) << 32) | submission;
	}

	inline unsigned int getBucketId(uint64_t sortKey)
	{
		return static_cast<unsigned int>(sortKey >> 32);
	}

	void debugViewport(const Viewport2& vp)
	{
		LOG << "\tx       =" << vp.topLeftX << "\n"
			<< "\ty       =" << vp.topLeftY << "\n"
			<< "\tw       =" << vp.width << "\n"
			<< "\th       =" << vp.height << "\n"
			<< "\tmindepth=" << vp.minDepth << "\n"
			<< "\tmaxdepth=" << vp.maxDepth;
	}
}

RenderQueue::RenderQueue(Renderer &renderer_) : renderer(&renderer_)
{

}

RenderQueue &RenderQueue::submit(unsigned int submission, unsigned int bucket)
{
	auto key = makeSortKey(submission, bucket);
	renderItems.push_back({ key, submission });
	return *this;
}

// issue a clear color command at the beginning of the bucket
RenderQueue &RenderQueue::clearColor(
	unsigned int bucket,
	float r,
	float g,
	float b,
	float a
	)
{
	assert(bucket < kMaxBuckets);
	buckets[bucket].flags |= Bucket::ClearColor;
	buckets[bucket].clearColor[0] = r;
	buckets[bucket].clearColor[1] = g;
	buckets[bucket].clearColor[2] = b;
	buckets[bucket].clearColor[3] = a;
	return *this;
}

// issue a clear depth command at the beginning of the bucket
RenderQueue &RenderQueue::clearDepth(
	unsigned int bucket,
	float z
	)
{
	assert(bucket < kMaxBuckets);
	buckets[bucket].flags |= Bucket::ClearDepth;
	buckets[bucket].clearDepth = z;
	return *this;
}

// set the color & depth render targets
RenderQueue &RenderQueue::setRenderTargets(
	unsigned int bucket,
	std::array_ref<const RenderTarget*> colorTargets,
	const RenderTarget *depthStencilTarget
	)
{
	assert(bucket < kMaxBuckets);
	assert(colorTargets.size() < kMaxColorRenderTargets);
	for (int i = 0; i < colorTargets.size(); ++i) 
		buckets[bucket].colorTargets[i] = colorTargets[i];
	buckets[bucket].depthStencilTarget = depthStencilTarget;
	return *this;
}

RenderQueue &RenderQueue::setViewports(
	unsigned int bucket,
	std::array_ref<Viewport2> viewports
	)
{
	assert(bucket < kMaxBuckets);
	assert(viewports.size() < kMaxViewports);
	for (int i = 0; i < viewports.size(); ++i)
		buckets[bucket].viewports[i] = viewports[i];
	return *this;
}

void RenderQueue::debugPrint()
{
	LOG << "RenderQueue " << this;
	for (unsigned int i = 0; i < kMaxBuckets; ++i) {
		const auto &b = buckets[i];
		LOG << "Bucket #" << i << "\n"
			<< "color=" << b.clearColor << "\n"
			<< "depth=" << b.clearDepth << "\n"
			<< "rt[0]=" << b.colorTargets[0] << "\n"
			<< "rt[1]=" << b.colorTargets[1] << "\n"
			<< "rt[2]=" << b.colorTargets[2] << "\n"
			<< "rt[3]=" << b.colorTargets[3] << "\n";
		for (int i = 0; i < kMaxViewports; ++i) {
			LOG << "Viewport #" << i << ':';
			debugViewport(b.viewports[i]);
		}
	}

	for (const auto &ri : renderItems) 
	{
		LOG << "RenderItem SK=" << ri.sortKey << " SB=" << ri.submissionId;
	}
}

RenderQueue &RenderQueue::flush()
{
	// sort the command list
	std::sort(renderItems.begin(), renderItems.end(), [](const RenderItem &i1, const RenderItem &i2) {
		return i1.sortKey < i2.sortKey;
	});

	int currentBucket = -1;
	for (const auto &item : renderItems) {
		int nextBucket = getBucketId(item.sortKey);
		if (nextBucket != currentBucket) {
			currentBucket = nextBucket;
			const auto &b = buckets[currentBucket];
			renderer->setRenderTargets(
				std::make_array_ref(b.colorTargets.data(), b.colorTargets.size()), 
				b.depthStencilTarget);
			renderer->setViewports(std::make_array_ref(b.viewports.data(), b.viewports.size()));
			if (b.flags & Bucket::ClearColor) 
				renderer->clearColor(b.clearColor[0], b.clearColor[1], b.clearColor[2], b.clearColor[3]);
			if (b.flags & Bucket::ClearDepth)
				renderer->clearDepth(b.clearDepth);
		}
		renderer->submit(item.submissionId);
	}

	renderItems.clear();
	return *this;
}