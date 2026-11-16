#pragma once
#include <pch.h>
#include <Memory/LinearAllocator.hpp>
#include <Memory/StackAllocator.hpp>

namespace CoreEngine
{
    struct CORE_API MemoryConfiguration
    {
        size_t LinearAllocateSize = 0;
        size_t StackAllocateSize = 0;
    };

    // Owns a per-frame LinearAllocator (reset by Update every frame) and a StackAllocator.
    // Stack frees that break LIFO order are deferred until every allocation above them is freed.
    class CORE_API MemoryManager
    {
    private:
        struct StackRecord
        {
            void* Address;
            size_t Size;
            bool PendingFree;
        };

    public:
        MemoryManager(const MemoryConfiguration& config);
        ~MemoryManager();

        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;

        void Update();
        void Clear();

        void* AllocateOnPerframe(size_t memorySize, uint8_t alignment);

        void* AllocateOnStack(size_t memorySize, uint8_t alignment);
        void FreeOnStack(void* memory);
        void ClearOnStack();

        // Logs a warning for every stack allocation that was never freed. Also called by the destructor.
        void DetectMemoryLeak();

    private:
        void ReleasePendingOnTop();

    private:
        LinearAllocator mPerframeAllocator;
        StackAllocator mStackAllocator;
        std::vector<StackRecord> mStackRecords; // Bottom -> top, mirrors the stack
    };
}
