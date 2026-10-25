#include <Memory/MemoryAllocator.hpp>

namespace CoreEngine::Memory {
	MemoryAllocator::MemoryAllocator(size_t memorySize) : 
		mMemorySize(memorySize), mStartAddress(malloc(memorySize)), mAllocatedCount(0), mUsedMemory(0) { }

	bool MemoryAllocator::CheckIsPowerOfTwo(uint8_t alignment) {
		return (alignment & (alignment - 1)) == 0;
	}
	
	uint8_t MemoryAllocator::GetAddressAdjustment(const void* address, uint8_t alignment) {
		CORE_ASSERT(CheckIsPowerOfTwo(alignment) && "Alignment must be power of two");

		uintptr_t remain = reinterpret_cast<uintptr_t>(address) & (alignment - 1); // Mod operation
		return remain == 0 ? 0 : alignment - remain;
	}
}