#include <Memory/MemoryChunkManager.hpp>
#include <Logger/Logger.hpp>
#include <new>

namespace CoreEngine
{
    // -------------------------------------------------------------------------
    // MemoryChunk
    // -------------------------------------------------------------------------

    MemoryChunkManager::MemoryChunk::MemoryChunk(size_t blockSize, size_t blockCount, uint8_t alignment)
        : mPoolAllocator(blockSize, blockCount, alignment)
        , mAlignment(alignment)
    {
    }

    void* MemoryChunkManager::MemoryChunk::Allocate()
    {
        return mPoolAllocator.Allocate(mPoolAllocator.GetBlockSize(), mAlignment);
    }

    void MemoryChunkManager::MemoryChunk::Free(void* memory)
    {
        mPoolAllocator.Free(memory);
    }

    void MemoryChunkManager::MemoryChunk::Clear()
    {
        mPoolAllocator.Clear();
    }

    bool MemoryChunkManager::MemoryChunk::Empty() const
    {
        return mPoolAllocator.GetFreeBlockCount() == 0;
    }

    bool MemoryChunkManager::MemoryChunk::Contains(const void* address) const
    {
        return mPoolAllocator.Contains(address);
    }

    // -------------------------------------------------------------------------
    // MemoryChunkManager
    // -------------------------------------------------------------------------

    MemoryChunkManager::MemoryChunkManager(const MemoryChunkManagerConfiguration& config)
        : mConfig(config)
        , mStackAllocator(config.StackAllocateSize)
    {
        CORE_ASSERT(config.BlockSize > INVALID_MEMORY_SIZE && "MemoryChunkManager block size must be greater than zero!");
        CORE_ASSERT(config.MaxBlockCountPerChunk > 0 && "MemoryChunk must contain at least one block!");
    }

    MemoryChunkManager::~MemoryChunkManager()
    {
        DestroyChunks();
    }

    void* MemoryChunkManager::Allocate()
    {
        // Newest chunks are the most likely to still have free blocks.
        for (auto chunk = mChunks.rbegin(); chunk != mChunks.rend(); ++chunk)
        {
            if (!(*chunk)->Empty())
            {
                return (*chunk)->Allocate();
            }
        }

        MemoryChunk* chunk = CreateChunk();
        if (!chunk)
        {
            IP_ENGINE_WARN("MemoryChunkManager can't allocate a block because the stack has no space for a new chunk!");
            return nullptr;
        }

        return chunk->Allocate();
    }

    void MemoryChunkManager::Free(void* memory)
    {
        if (!memory)
        {
            return;
        }

        for (MemoryChunk* chunk : mChunks)
        {
            if (chunk->Contains(memory))
            {
                chunk->Free(memory);
                return;
            }
        }

        IP_ENGINE_ERROR("MemoryChunkManager can't free address {} because it doesn't belong to any chunk!", memory);
    }

    void MemoryChunkManager::Clear()
    {
        DestroyChunks();
        mStackAllocator.Clear();
    }

    MemoryChunkManager::MemoryChunk* MemoryChunkManager::CreateChunk()
    {
        void* chunkMemory = mStackAllocator.Allocate(sizeof(MemoryChunk), alignof(MemoryChunk));
        if (!chunkMemory)
        {
            return nullptr;
        }

        MemoryChunk* chunk = new (chunkMemory) MemoryChunk(mConfig.BlockSize, mConfig.MaxBlockCountPerChunk, mConfig.BlockAlignment);
        mChunks.push_back(chunk);
        return chunk;
    }

    void MemoryChunkManager::DestroyChunks()
    {
        for (MemoryChunk* chunk : mChunks)
        {
            chunk->~MemoryChunk();
        }
        mChunks.clear();
    }
}
