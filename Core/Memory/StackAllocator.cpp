#include <Memory/StackAllocator.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine
{
    void* StackAllocator::Allocate(size_t memorySize, uint8_t alignment)
    {
        CORE_ASSERT(memorySize > INVALID_MEMORY_SIZE && "Allocated invalid memory size!");
        CORE_ASSERT(alignment < MAX_ALLOWED_ALIGNMENT && "Allcated invalid alignment");

        union
        {
            void* asVoidAddress;
            uintptr_t asUintAddress;
            Header* asHeader;
        };

        asVoidAddress = mStartAddress;
        asUintAddress += mUsedMemory;

        uint8_t padding = CalculatePadding(asVoidAddress, alignment, sizeof(Header));

        if (mUsedMemory + padding + memorySize > mMemorySize)
        {
            IP_ENGINE_WARN("StackAllocator can't allocate with memory size {} due to not enough space!", mMemorySize);
            return nullptr;
        }

        asUintAddress += padding;
        asUintAddress -= sizeof(Header);
        asHeader->Padding = padding;
        asUintAddress += sizeof(Header);

        mUsedMemory += padding + memorySize;
        ++mAllocatedCount;

        return asVoidAddress;
    }

    void StackAllocator::Free(void* memory)
    {
        CORE_ASSERT(CheckAddressInBound(memory) && "Memory is not in bound");

        union
        {
            void* asVoidAddress;
            uintptr_t asUintAddress;
            Header* asHeader;
        };

        asVoidAddress = memory;
        asUintAddress -= sizeof(Header);
        mUsedMemory = reinterpret_cast<uintptr_t>(memory) - asHeader->Padding - reinterpret_cast<uintptr_t>(mStartAddress);
        --mAllocatedCount;
    }

    void StackAllocator::Clear()
    {
        mUsedMemory = 0;
        mAllocatedCount = 0;
    }

    uint8_t StackAllocator::CalculatePadding(void* address, uint8_t alignment, uint8_t extraSize)
    {
        CORE_ASSERT(CheckIsPowerOfTwo(alignment) && "Alignment is not power of 2");

        uint8_t adjustment = GetAddressAdjustment(address, alignment);
        uint8_t remainSize = adjustment < extraSize ? extraSize - adjustment : 0;

        return adjustment + alignment * ((remainSize + alignment - 1) / alignment);
    }

    bool StackAllocator::CheckAddressInBound(void* address)
    {
        uintptr_t ipAddress = reinterpret_cast<uintptr_t>(address);
        uintptr_t ipStartAddress = reinterpret_cast<uintptr_t>(mStartAddress);

        return ipAddress >= ipStartAddress && ipAddress < ipStartAddress + mUsedMemory;
    }
}