#include "src/OBJ/Vertex.h"
#include <cstdio>

int main() {
	Vect4 v1(1.0f, 2.0f, 3.0f); // A regular vertex
	Vect4 v2(1.0f, 2.0f, 3.0f, 0.0f); // A vertex at infinity
	if (v1.IsInfinityFar()) {
		// This should not be printed
		printf("v1 is at infinity.\n");
	}
	else {
		printf("v1 is not at infinity.\n");
	}
	if (v2.IsInfinityFar()) {
		printf("v2 is at infinity.\n");
	}
	else {
		// This should not be printed
		printf("v2 is not at infinity.\n");
	}
	return 0;
}