#pragma once
#include <pch.h>

namespace CoreEngine::Memory {
	constexpr uint8_t INVALID_MEMORY_SIZE = 0;
	constexpr uint8_t MAX_ALLOWED_ALIGNMENT = 128;

	class CORE_API MemoryAllocator {
	public:
		MemoryAllocator(size_t memorySize);
		virtual ~MemoryAllocator() { free(mStartAddress); mStartAddress = nullptr; }

		virtual void* Allocate(size_t memorySize, uint8_t alignment) = 0;
		virtual void Free(void* memory) = 0;
		virtual void Clear() = 0;

	protected:
		static bool CheckIsPowerOfTwo(uint8_t alignment);
		static uint8_t GetAddressAdjustment(const void* address, uint8_t alignment);

	protected:
		void* mStartAddress = nullptr;
		size_t mMemorySize = 0;
		size_t mUsedMemory = 0;
		size_t mAllocatedCount = 0;
	};
}