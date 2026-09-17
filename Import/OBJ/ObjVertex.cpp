#include <OBJ/ObjVertex.h>
#include <iostream>

void ObjVertex::PrintVertexCoordinates(unsigned int vertexIndex) const {
	std::cout << "Vertex Index: " << vertexIndex << ", Coordinates: ("
		<< m_vertex.GetX() << ", " << m_vertex.GetY() << ", " << m_vertex.GetZ() << ")" << std::endl;
}