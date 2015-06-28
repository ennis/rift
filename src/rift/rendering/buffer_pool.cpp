#include <rendering/opengl4.hpp>
#include <cassert>
#include <iomanip>
#include <log.hpp>

namespace
{
	unsigned syncPreviousFrame(unsigned sync_cycle, unsigned k) {
		return (sync_cycle + 3 - k) % 3;
	}

	unsigned nextLog2(size_t v)
	{
		unsigned r = 0;
		v = v - 1;
		while (v >>= 1) {
			r++;
		}
		return r + 1;
	}
}

size_t GraphicsContext::blockSizeFromPoolIndex(unsigned index)
{
	return 1ull << (index + kMinBlockSizeLog);
}

// buffers[frame_index]: current frame
// buffers[(frame_index+1)%3]: frame n-1
// buffers[(frame_index+2)%3]: frame n-2

GLuint allocBufferRaw(GLenum target, size_t size, GLbitfield flags, const void *initialData)
{
	GLuint obj;
	gl::GenBuffers(1, &obj);
	gl::BindBuffer(target, obj);
	// allocate immutable storage
	gl::BufferStorage(target, size, initialData, flags);
	gl::BindBuffer(target, 0);
	return obj;
}

void updateBufferRaw(GLuint buf, GLenum target, int offset, int size, const void *data)
{
	if (gl::exts::var_EXT_direct_state_access) {
		gl::NamedBufferSubDataEXT(buf, offset, size, data);
	}
	else {
		gl::BindBuffer(target, buf);
		gl::BufferSubData(target, offset, size, data);
	}
}

Buffer::Ptr GraphicsContext::allocLargeBuffer(GLenum target, size_t size, GLbitfield flags_, const void *initialData)
{
	auto ptr = std::make_unique<Buffer>(*this);
	auto &buf = *ptr;
	buf.target = target;
	buf.size = size;
	buf.pool_index = (unsigned)-1;
	buf.block_index = { (unsigned)-1, (unsigned)-1 };
	buf.offset = 0;
	buf.obj = allocBufferRaw(
		target,
		size,
		flags_
		| gl::MAP_WRITE_BIT
		| gl::MAP_PERSISTENT_BIT
		| gl::MAP_COHERENT_BIT,
		initialData);
	buf.ptr = gl::MapNamedBufferRangeEXT(
		buf.obj, 0, size,
		gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
		gl::MAP_PERSISTENT_BIT | // persistent mapping
		gl::MAP_COHERENT_BIT | // coherent
		gl::MAP_WRITE_BIT | // write-only (no readback)
		gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
		);
	return std::move(ptr);
}

void GraphicsContext::createPool(unsigned pool_index)
{
	assert(pool_index < kNumPools);
	pools[pool_index].push_back(BufferPool());
	auto &pool = pools[pool_index].back();
	pool.target = gl::UNIFORM_BUFFER;
	pool.obj = allocBufferRaw(
		gl::UNIFORM_BUFFER,
		kPoolPageSize,
		gl::MAP_WRITE_BIT
		| gl::MAP_PERSISTENT_BIT
		| gl::MAP_COHERENT_BIT,
		nullptr);
	pool.block_size = blockSizeFromPoolIndex(pool_index);
	pool.num_blocks = kPoolPageSize / pool.block_size;
	pool.mapped_ptr = gl::MapNamedBufferRangeEXT(
		pool.obj, 0, kPoolPageSize,
		gl::MAP_INVALIDATE_BUFFER_BIT | // discard old contents
		gl::MAP_PERSISTENT_BIT | // persistent mapping
		gl::MAP_COHERENT_BIT | // coherent
		gl::MAP_WRITE_BIT | // write-only (no readback)
		gl::MAP_UNSYNCHRONIZED_BIT // no driver sync (we handle this)
		);
	// add new blocks to free list
	auto page_index = pools[pool_index].size() - 1;
	for (auto i = 0u; i < pool.num_blocks; ++i)
	{
		free_list[pool_index].push_back(PoolBlockIndex{ page_index, i });
	}
	//LOG << "newPool size=" << pool.block_size << " page_index=" << page_index;
}

PoolBlockIndex GraphicsContext::allocFromPool(unsigned pool_index)
{
	assert(pool_index < kNumPools);
	auto &free = free_list[pool_index];
	if (free.empty())
		createPool(pool_index);
	auto block = free.back();
	free.pop_back();
	return block;
}

void GraphicsContext::reclaimTransientBuffers()
{
	// reclaim buffers from frame N-2
	auto prev = syncPreviousFrame(sync_cycle, 0);
	//LOG << "Number of transient buffers on frame N-2: " << transient_buffers[prev].size();
	if (gl::ClientWaitSync(transient_syncs[prev], gl::SYNC_FLUSH_COMMANDS_BIT, 1000000)
		== gl::TIMEOUT_EXPIRED)
	{
		//assert(false && "Timeout expired while waiting for frame N-2 to finish");
		LOG << "Timeout expired while waiting for frame N-2 to finish";
	}
	gl::DeleteSync(transient_syncs[prev]);
	transient_buffers[prev].clear();
}

void GraphicsContext::showPoolDebugInfo()
{
	for (auto i = 0u; i < kNumPools; ++i)
	{
		auto size = blockSizeFromPoolIndex(i);
		auto &pool = pools[i];
		auto &free = free_list[i];
		std::ostringstream os;
		os << std::left << std::setw(8) << size << std::setw(0) << ": " << free_list[i].size() << "/" << pools[i].size() * (kPoolPageSize / size);
		Logging::screenMessage(os.str());
	}
}

void GraphicsContext::syncTransientBuffers()
{
	// fence current frame
	auto curFrame = syncPreviousFrame(sync_cycle, 0);
	//LOG << "Number of transient buffers allocated this frame: " << transient_buffers[curFrame].size();
	GraphicsContext::showPoolDebugInfo();
	Logging::screenMessage("TBUF    : " 
		+ std::to_string(transient_buffers[curFrame].size())
		+ "," + std::to_string(transient_buffers[syncPreviousFrame(sync_cycle, 1)].size())
		+ "," + std::to_string(transient_buffers[syncPreviousFrame(sync_cycle, 2)].size()));
	transient_syncs[curFrame] = gl::FenceSync(gl::SYNC_GPU_COMMANDS_COMPLETE, 0);
	sync_cycle = (sync_cycle + 1) % 3;
}

Buffer::Ptr GraphicsContext::createBuffer(GLenum target, size_t size, const void *initialData)
{
	unsigned pool_index;
	if (size < kMinBlockSize)
		pool_index = 0;
	else if (size > kMaxBlockSize) {
		return allocLargeBuffer(
			target,
			size,
			gl::DYNAMIC_STORAGE_BIT,
			initialData);
	}
	else
		pool_index = nextLog2(size) - kMinBlockSizeLog;
	auto block = allocFromPool(pool_index);
	auto ptr = std::make_unique<Buffer>(*this);
	auto &buf = *ptr;
	buf.obj = pools[pool_index][block.page_index].obj;
	buf.offset = block.block_index *  blockSizeFromPoolIndex(pool_index);
	buf.ptr = reinterpret_cast<char*>(pools[pool_index][block.page_index].mapped_ptr) + buf.offset;
	if (initialData)
		memcpy(buf.ptr, initialData, size);
	buf.size = size;
	buf.block_index = block;
	buf.pool_index = pool_index;
	return std::move(ptr);
}

Buffer *GraphicsContext::createTransientBuffer(GLenum target, size_t size, const void *initialData)
{
	auto ptr = createBuffer(target, size, initialData);
	auto buf = ptr.get();
	transient_buffers[sync_cycle].push_back(std::move(ptr));
	return buf;
}

void GraphicsContext::deleteBuffer(Buffer &buf)
{
	if (buf.pool_index != (unsigned)-1)
		free_list[buf.pool_index].push_back(buf.block_index);
	else
		gl::DeleteBuffers(1, &buf.obj);
}