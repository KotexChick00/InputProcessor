#pragma once
#include <vector>

template<class char_t>

inline bool isEndOfBuffer(char_t iterator, char_t bufferEnd) {
	if (iterator == bufferEnd) {
		return true;
	}

	--bufferEnd;

	return (iterator == bufferEnd);
	
}
