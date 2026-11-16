#include <Memory/MemoryManager.hpp>
#include <Logger/Logger.hpp>

namespace CoreEngine
{
    MemoryManager::MemoryManager(const MemoryConfiguration& config)
        : mPerframeAllocator(config.LinearAllocateSize)
        , mStackAllocator(config.StackAllocateSize)
    {
    }

    MemoryManager::~MemoryManager()
    {
        DetectMemoryLeak();
    }

    void MemoryManager::Update()
    {
        mPerframeAllocator.Clear();
    }

    void MemoryManager::Clear()
    {
        Update();
        ClearOnStack();
    }

    void* MemoryManager::AllocateOnPerframe(size_t memorySize, uint8_t alignment)
    {
        return mPerframeAllocator.Allocate(memorySize, alignment);
    }

    void* MemoryManager::AllocateOnStack(size_t memorySize, uint8_t alignment)
    {
        void* memory = mStackAllocator.Allocate(memorySize, alignment);
        if (memory)
        {
            mStackRecords.push_back({memory, memorySize, false});
        }

        return memory;
    }

    void MemoryManager::FreeOnStack(void* memory)
    {
        if (!memory)
        {
            return;
        }

        auto record = std::find_if(
            mStackRecords.rbegin(),
            mStackRecords.rend(),
            [memory](const StackRecord& stackRecord) { return stackRecord.Address == memory; });

        if (record == mStackRecords.rend())
        {
            IP_ENGINE_ERROR("MemoryManager can't free address {} because it is not allocated on stack!", memory);
            return;
        }

        if (record->PendingFree)
        {
            IP_ENGINE_WARN("MemoryManager: address {} is already freed on stack!", memory);
            return;
        }

        if (record == mStackRecords.rbegin())
        {
            mStackAllocator.Free(memory);
            mStackRecords.pop_back();
            ReleasePendingOnTop();
        }
        else
        {
            IP_ENGINE_WARN(
                "MemoryManager: address {} is freed out of stack order, it will be released after the allocations above it are freed!",
                memory);
            record->PendingFree = true;
        }
    }

    void MemoryManager::ClearOnStack()
    {
        mStackAllocator.Clear();
        mStackRecords.clear();
    }

    void MemoryManager::DetectMemoryLeak()
    {
        size_t leakCount = 0;
        size_t leakSize = 0;

        for (const StackRecord& record : mStackRecords)
        {
            if (record.PendingFree)
            {
                continue;
            }

            IP_ENGINE_WARN("MemoryManager: memory leak at address {} with size {}!", record.Address, record.Size);
            ++leakCount;
            leakSize += record.Size;
        }

        if (leakCount > 0)
        {
            IP_ENGINE_WARN("MemoryManager detected {} memory leak(s) with total size {} on stack!", leakCount, leakSize);
        }
    }

    void MemoryManager::ReleasePendingOnTop()
    {
        while (!mStackRecords.empty() && mStackRecords.back().PendingFree)
        {
            mStackAllocator.Free(mStackRecords.back().Address);
            mStackRecords.pop_back();
        }
    }
}
