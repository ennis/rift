#include <renderer.hpp>
#include <cassert>
#include <log.hpp>

namespace gl4
{
	//namespace
	//{
		// OpenGL buffer pool
		struct BufferPool
		{
			BufferPool() = default;

			GLenum target = 0;
			GLuint obj = 0;
			// block size (multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT)
			size_t block_size = 0;
			// number of blocks
			size_t num_blocks = 0;
			// persistently mapped pointer
			void *mapped_ptr = nullptr;
		};


		// pools
		// 256 x 4096, 512 x 2048, 1024 x 1024, 2048 x 512, 4096 x 256, 8192 x 128,
		// 16384 x 64, 32768 x 32, 65536 x 16
		constexpr auto kNumPools = 9u;
		constexpr auto kMinBlockSizeLog = 8u;
		constexpr auto kMinBlockSize = 256u;
		constexpr auto kMaxBlockSize = kMinBlockSize << (kNumPools - 1);
		constexpr auto kPoolPageSize = 1024 * 1024ul;

		std::vector<BufferPool> pools[kNumPools];
		std::vector<PoolBlockIndex> free_list[kNumPools];

		size_t blockSizeFromPoolIndex(unsigned index)
		{
			return 1ull << (index + kMinBlockSizeLog);
		}

		unsigned nextLog2(size_t v)
		{
			unsigned r = 0;
			v = v - 1;
			while (v >>= 1) {
				r++;
			}
			return r+1;
		}

		// buffers[frame_index]: current frame
		// buffers[(frame_index+1)%3]: frame n-1
		// buffers[(frame_index+2)%3]: frame n-2
		unsigned frame_index;
		std::vector<Buffer::Ptr> transient_buffers[3];
		// fence syncs for transient buffers
		GLsync syncs[3];

		std::vector<Buffer::Ptr> static_buffers;

		// VBOs
		/*struct VertexBuffer
		{
		GLuint obj;
		};

		std::vector<VertexBuffer> dynamic_vbos;*/

		unsigned previousFrame(unsigned k)
		{
			return (frame_index + 3 - k) % 3;
		}
	//}


	void deleteBuffer(Buffer &buf)
	{
		if (buf.pool_index != (unsigned)-1)
		{
			free_list[buf.pool_index].push_back(buf.block_index);
		}
		else
		{
			gl::DeleteBuffers(1, &buf.obj);
		}
	}

	GLuint allocBuffer(GLenum target, size_t size, GLbitfield flags, const void *initialData)
	{
		GLuint obj;
		gl::GenBuffers(1, &obj);
		gl::BindBuffer(target, obj);
		// allocate immutable storage
		gl::BufferStorage(target, size, initialData, flags);
		gl::BindBuffer(target, 0);
		return obj;
	}

	Buffer::Ptr allocLargeBuffer(GLenum target, size_t size, GLbitfield flags_, const void *initialData)
	{
		auto ptr = std::make_unique<Buffer>();
		auto &buf = *ptr;
		buf.target = target;
		buf.size = size;
		buf.pool_index = (unsigned)-1;
		buf.block_index = { (unsigned)-1, (unsigned)-1 };
		buf.offset = 0;
		buf.obj = allocBuffer(
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

	Buffer &allocLargeTransientBuffer(GLenum target, size_t size, const void *initialData)
	{
		auto ptr = allocLargeBuffer(
			target,
			size,
			gl::DYNAMIC_STORAGE_BIT,
			initialData);
		auto &buf = *ptr;
		transient_buffers[previousFrame(0)].push_back(std::move(ptr));
		return buf;
	}

	void newPool(unsigned pool_index)
	{
		assert(pool_index < kNumPools);
		pools[pool_index].push_back(BufferPool());
		auto &pool = pools[pool_index].back();
		pool.target = gl::UNIFORM_BUFFER;
		pool.obj = allocBuffer(
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
		LOG << "newPool size=" << pool.block_size << " page_index=" << page_index;
	}

	PoolBlockIndex allocFromPool(unsigned pool_index)
	{
		assert(pool_index < kNumPools);
		auto &free = free_list[pool_index];
		if (free.empty())
		{
			newPool(pool_index);
			// free shouldn't be empty now
		}

		auto block = free.back();
		free.pop_back();
		return block;
	}

	void reclaimTransientBuffers()
	{
		// reclaim buffers from frame N-2
		auto prev = previousFrame(0);
		//LOG << "Number of transient buffers on frame N-2: " << transient_buffers[prev].size();
		if (gl::ClientWaitSync(syncs[prev], gl::SYNC_FLUSH_COMMANDS_BIT, 1000000)
			== gl::TIMEOUT_EXPIRED)
		{
			//assert(false && "Timeout expired while waiting for frame N-2 to finish");
			LOG << "Timeout expired while waiting for frame N-2 to finish";
		}
		gl::DeleteSync(syncs[prev]);
		transient_buffers[prev].clear();
	}

	void syncTransientBuffers()
	{
		// fence current frame
		auto curFrame = previousFrame(0);
		//LOG << "Number of transient buffers allocated this frame: " << transient_buffers[curFrame].size();
		syncs[curFrame] = gl::FenceSync(gl::SYNC_GPU_COMMANDS_COMPLETE, 0);
		frame_index = (frame_index + 1) % 3;
	}
}

namespace Renderer
{
	Buffer::Ptr allocBuffer(BufferUsage bufferUsage, size_t size, const void *initialData)
	{
		using namespace gl4;
		unsigned pool_index;
		GLenum target = bufferUsageToBindingPoint(bufferUsage);
		if (size < gl4::kMinBlockSize) {
			pool_index = 0;
		}
		else if (size > kMaxBlockSize) {
			// do not allocate in a pool
			return allocLargeBuffer(
				target,
				size,
				gl::DYNAMIC_STORAGE_BIT,
				initialData);
		}
		else
		{
			pool_index = nextLog2(size) - kMinBlockSizeLog;
		}

		auto block = allocFromPool(pool_index);
		auto ptr = std::make_unique<gl4::Buffer>();
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

	Buffer &allocTransientBuffer(BufferUsage bufferUsage, size_t size, const void *initialData)
	{
		using namespace gl4;
		auto ptr = allocBuffer(bufferUsage, size, initialData);
		auto &buf = *ptr;
		transient_buffers[previousFrame(0)].push_back(std::move(ptr));
		return buf;
	}
}