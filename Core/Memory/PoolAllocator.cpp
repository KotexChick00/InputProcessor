#include <Memory/PoolAllocator.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine
{
    PoolAllocator::PoolAllocator(size_t blockSize, size_t blockCount, uint8_t alignment)
        // Reserve alignment - 1 extra bytes so the first block can be moved onto the alignment.
        : MemoryAllocator(CalculateBlockStride(blockSize, CalculatePoolAlignment(alignment)) * blockCount + CalculatePoolAlignment(alignment) - 1)
        , mBlockSize(blockSize)
        , mBlockStride(CalculateBlockStride(blockSize, CalculatePoolAlignment(alignment)))
        , mBlockCount(blockCount)
        , mAlignment(CalculatePoolAlignment(alignment))
    {
        CORE_ASSERT(blockSize > INVALID_MEMORY_SIZE && "Pool block size must be greater than zero!");
        CORE_ASSERT(blockCount > 0 && "Pool must contain at least one block!");
        CORE_ASSERT(CheckIsPowerOfTwo(mAlignment) && mAlignment <= MAX_ALLOWED_ALIGNMENT && "Invalid pool alignment!");
        CORE_ASSERT(mStartAddress && "Pool allocator failed to reserve memory!");

        uintptr_t firstBlock = reinterpret_cast<uintptr_t>(mStartAddress) + GetAddressAdjustment(mStartAddress, mAlignment);
        mFirstBlock = reinterpret_cast<void*>(firstBlock);
        Clear();
    }

    void* PoolAllocator::Allocate(size_t memorySize, uint8_t alignment)
    {
        CORE_ASSERT(memorySize > INVALID_MEMORY_SIZE && "Allocated invalid memory size!");
        CORE_ASSERT(memorySize <= mBlockSize && "Allocation is larger than the pool block size!");
        CORE_ASSERT(alignment > 0 && CheckIsPowerOfTwo(alignment) && "Alignment must be power of two");
        CORE_ASSERT(alignment <= mAlignment && "Allocation alignment is stricter than the pool alignment!");

        if (!mFreeList)
        {
            IP_ENGINE_WARN("PoolAllocator is out of blocks ({} blocks of {} bytes)!", mBlockCount, mBlockSize);
            return nullptr;
        }

        FreeBlock* block = mFreeList;
        mFreeList = block->Next;

        mUsedMemory += mBlockStride;
        ++mAllocatedCount;

        return block;
    }

    void PoolAllocator::Free(void* memory)
    {
        if (!memory) return;
        CORE_ASSERT(CheckIsBlockAddress(memory) && "Memory doesn't belong to this pool!");
        CORE_ASSERT(mAllocatedCount > 0 && "Freeing more blocks than were allocated!");

        FreeBlock* block = static_cast<FreeBlock*>(memory);
        block->Next = mFreeList;
        mFreeList = block;

        mUsedMemory -= mBlockStride;
        --mAllocatedCount;
    }

    void PoolAllocator::Clear()
    {
        // Link blocks back to front so that allocations hand out ascending addresses.
        mFreeList = nullptr;
        uintptr_t firstBlock = reinterpret_cast<uintptr_t>(mFirstBlock);
        for (size_t index = mBlockCount; index > 0; --index)
        {
            FreeBlock* block = reinterpret_cast<FreeBlock*>(firstBlock + (index - 1) * mBlockStride);
            block->Next = mFreeList;
            mFreeList = block;
        }

        mUsedMemory = 0;
        mAllocatedCount = 0;
    }

    uint8_t PoolAllocator::CalculatePoolAlignment(uint8_t alignment)
    {
        // Every block must be able to hold the free-list pointer.
        constexpr uint8_t MinAlignment = static_cast<uint8_t>(alignof(FreeBlock));
        return alignment < MinAlignment ? MinAlignment : alignment;
    }

    size_t PoolAllocator::CalculateBlockStride(size_t blockSize, uint8_t alignment)
    {
        size_t stride = blockSize < sizeof(FreeBlock) ? sizeof(FreeBlock) : blockSize;
        return (stride + alignment - 1) & ~static_cast<size_t>(alignment - 1);
    }

    bool PoolAllocator::CheckIsBlockAddress(const void* address) const
    {
        uintptr_t ipAddress = reinterpret_cast<uintptr_t>(address);
        uintptr_t ipFirstBlock = reinterpret_cast<uintptr_t>(mFirstBlock);

        if (ipAddress < ipFirstBlock || ipAddress >= ipFirstBlock + mBlockCount * mBlockStride) return false;
        return (ipAddress - ipFirstBlock) % mBlockStride == 0;
    }
}
