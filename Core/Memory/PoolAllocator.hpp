#pragma once
#include <pch.h>
#include <Memory/MemoryAllocator.hpp>
#include <cstddef>

namespace CoreEngine
{
    class CORE_API PoolAllocator : public MemoryAllocator
    {
    public:
        PoolAllocator(size_t blockSize, size_t blockCount, uint8_t alignment = alignof(std::max_align_t));
        ~PoolAllocator() = default;

        void* Allocate(size_t memorySize, uint8_t alignment) override;
        void Free(void* memory) override;
        void Clear() override;

        // Only the block area counts, the alignment padding before the first block does not.
        bool Contains(const void* address) const override;

        CORE_FORCE_INLINE size_t GetBlockSize() const { return mBlockSize; }
        CORE_FORCE_INLINE size_t GetBlockCount() const { return mBlockCount; }
        CORE_FORCE_INLINE size_t GetFreeBlockCount() const { return mBlockCount - mAllocatedCount; }

    private:
        struct FreeBlock
        {
            FreeBlock* Next;
        };

        static uint8_t CalculatePoolAlignment(uint8_t alignment);
        static size_t CalculateBlockStride(size_t blockSize, uint8_t alignment);
        bool CheckIsBlockAddress(const void* address) const;

    private:
        size_t mBlockSize = 0;
        size_t mBlockStride = 0;
        size_t mBlockCount = 0;
        uint8_t mAlignment = 0;
        void* mFirstBlock = nullptr;
        FreeBlock* mFreeList = nullptr;
    };
}
