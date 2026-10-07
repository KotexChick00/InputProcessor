#pragma once
#include <pch.h>
#include <Memory/MemoryAllocator.hpp>

namespace CoreEngine::Memory {
	class CORE_API LinearMemoryAllocator : public MemoryAllocator {
	public:
		LinearMemoryAllocator(size_t memorySize): MemoryAllocator(memorySize) { }
		~LinearMemoryAllocator() = default;

		void* Allocate(size_t memorySize, uint8_t alignment) override;
		void Free(void* memory) override;
		void Clear() override;
	};
}