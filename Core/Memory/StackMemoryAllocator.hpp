#pragma once
#include <pch.h>
#include <Memory/MemoryAllocator.hpp>

namespace CoreEngine::Memory {
	class CORE_API StackMemoryAllocator : public MemoryAllocator {
	private:
		struct Header {
			uint8_t Padding;
		};
	public:
		StackMemoryAllocator(size_t memorySize) : MemoryAllocator(memorySize) { }
		~StackMemoryAllocator() = default;

		void* Allocate(size_t memorySize, uint8_t alignment) override;
		void Free(void* memory) override;
		void Clear() override;

	private:
		bool CheckAddressInBound(void* address);
		static uint8_t CalculatePadding(void* address, uint8_t alignment, uint8_t extraSize);
	};
}