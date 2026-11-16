#pragma once
#include <pch.h>
#include <Memory/MemoryAllocator.hpp>

namespace CoreEngine
{
    class CORE_API LinearAllocator : public MemoryAllocator
    {
    public:
        LinearAllocator(size_t memorySize)
            : MemoryAllocator(memorySize)
        {
        }
        ~LinearAllocator() = default;

        void* Allocate(size_t memorySize, uint8_t alignment) override;
        void Free(void* memory) override;
        void Clear() override;
    };
}