#include <Memory/LinearMemoryAllocator.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine::Memory {
    void* LinearMemoryAllocator::Allocate(size_t memorySize, uint8_t alignment) {
        CORE_ASSERT(memorySize > INVALID_MEMORY_SIZE && "Allocated invalid memory size!");
        CORE_ASSERT(alignment < MAX_ALLOWED_ALIGNMENT && "Allcated invalid alignment");

        union {
            void* pVoidAddress;
            uintptr_t pUintAddress;
        };

        pVoidAddress = mStartAddress;
        pUintAddress += mUsedMemory;

        uint8_t adjustment = GetAddressAdjustment(pVoidAddress, alignment);
        if (mUsedMemory + adjustment + memorySize > mMemorySize) {
            IP_ENGINE_WARN("Linear allocator can't allocate with size {} due to not enough memory size!", memorySize);
            return nullptr;
        }

        pUintAddress += adjustment;
        mUsedMemory += adjustment + memorySize;
        mAllocatedCount += 1;

        return pVoidAddress;
    }

    void LinearMemoryAllocator::Free(void* memory) {
        CORE_ASSERT(false && "LinearMemoryAllocator doesn't support method free!");
    }

    void LinearMemoryAllocator::Clear() {
        mUsedMemory = 0;
        mAllocatedCount = 0;
    }
}