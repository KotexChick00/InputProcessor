#pragma once
#include <pch.h>
#include <Memory/PoolAllocator.hpp>
#include <Memory/StackAllocator.hpp>

namespace CoreEngine
{
    struct CORE_API MemoryChunkManagerConfiguration
    {
        size_t StackAllocateSize = 0;
        size_t BlockSize = 0;
        size_t MaxBlockCountPerChunk = 0;
        uint8_t BlockAlignment = alignof(std::max_align_t);
    };

    // Hands out fixed-size blocks from one or more MemoryChunks.
    // A new MemoryChunk is created on the StackAllocator when every chunk is full.
    class CORE_API MemoryChunkManager
    {
    private:
        class MemoryChunk
        {
        public:
            MemoryChunk(size_t blockSize, size_t blockCount, uint8_t alignment);
            ~MemoryChunk() = default;

            void* Allocate();
            void Free(void* memory);
            void Clear();

            // True when the chunk has no free block left.
            bool Empty() const;
            bool Contains(const void* address) const;

        private:
            PoolAllocator mPoolAllocator;
            uint8_t mAlignment = 0;
        };

    public:
        MemoryChunkManager(const MemoryChunkManagerConfiguration& config);
        ~MemoryChunkManager();

        MemoryChunkManager(const MemoryChunkManager&) = delete;
        MemoryChunkManager& operator=(const MemoryChunkManager&) = delete;

        void* Allocate();
        void Free(void* memory);
        void Clear();

        CORE_FORCE_INLINE size_t GetChunkCount() const { return mChunks.size(); }

    private:
        MemoryChunk* CreateChunk();
        void DestroyChunks();

    private:
        MemoryChunkManagerConfiguration mConfig;
        StackAllocator mStackAllocator;
        std::vector<MemoryChunk*> mChunks; // Borrowed, the objects live in mStackAllocator
    };
}
