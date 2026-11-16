#pragma once
#include <pch.h>
#include <Memory/MemoryAllocator.hpp>

namespace CoreEngine
{
    class CORE_API StackAllocator : public MemoryAllocator
    {
    private:
        struct Header
        {
            uint8_t Padding;
        };

    public:
        StackAllocator(size_t memorySize)
            : MemoryAllocator(memorySize)
        {
        }
        ~StackAllocator() = default;

        void* Allocate(size_t memorySize, uint8_t alignment) override;
        void Free(void* memory) override;
        void Clear() override;

    private:
        bool CheckAddressInBound(void* address);
        static uint8_t CalculatePadding(void* address, uint8_t alignment, uint8_t extraSize);
    };
}