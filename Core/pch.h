#pragma once

// Standard Library
#include <iostream>
#include <memory>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <stdint.h>
#include <limits>
#include <bitset>
#include <cassert>
#include <sstream>
#include <istream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <stack>
#include <queue>

template<typename T> using Shared = std::shared_ptr<T>;
template<typename T> using Unique = std::shared_ptr<T>;

#if ON_CORE
	#ifdef DYNAMIC_BUILD
		#ifdef _MSC_VER
			#define CORE_API __declspec(dllexport)
		#else
			#define CORE_API __attribute__((visibility("default")))
		#endif
	#else
		#define CORE_API
	#endif
#else
	#ifdef DYNAMIC_IMPORT
		#ifdef _MSC_VER
			#define CORE_API __declspec(dllimport)
		#else
			#define CORE_API
		#endif
	#else
		#define CORE_API
	#endif
#endif

// Runtime assert
#define CORE_ASSERT assert

// Static assert
#if defined(__clang__) || defined(__gcc__)
	#define CORE_STATIC_ASSERT _Static_assert
#else
	#define CORE_STATIC_ASSERT static_assert
#endif

// Inline
#if defined(__clang__) || defined(__gcc__)
	#define CORE_FORCE_INLINE __attribute__((always_inline)) inline
	#define CORE_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
	#define CORE_FORCE_INLINE __forceinline
	#define CORE_NOINLINE __declspec(noinline)
#else
	#define CORE_FORCE_INLINE inline
	#define CORE_NOINLINE
#endif

// Free memory
#define CORE_FREE(memory) if (memory != nullptr) { delete memory; memory = nullptr; }