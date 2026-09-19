#pragma once
#include <DataTypes.h>

class ObjVertex {
public:
	ObjVertex(float x, float y, float z, unsigned int vertexIndex)
		: m_vertex(x, y, z), m_vertexIndex(vertexIndex) {}
	ObjVertex(const Vector3& vertex, unsigned int vertexIndex)
		: m_vertex(vertex), m_vertexIndex(vertexIndex) {}
	unsigned int GetVertexIndex() const {
		return m_vertexIndex;
	}
	const Vector3& GetVertex() const {
		return m_vertex;
	}
	void SetVertex(const Vector3& vertex) { m_vertex = vertex; }
	void SetVertexIndex(unsigned int vertexIndex) { m_vertexIndex = vertexIndex; }
	void SetVertexAtIndex(unsigned int vertexIndex, const Vector3& vertex) {
		m_vertexIndex = vertexIndex;
		m_vertex = vertex;
	}
	void PrintVertexCoordinates(unsigned int vertexIndex) const;
private: 
	Vector3 m_vertex;
	unsigned int m_vertexIndex;
};