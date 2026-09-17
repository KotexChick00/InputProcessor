#pragma once
#include <vector>
#include <optional>

struct ObjFaceVertex
{
	unsigned int vertexIndex; // Index of the vertex in the OBJ file (1-based)
	std::optional<unsigned int> textureIndex; // Index of the texture coordinate in the OBJ file (1-based)
	std::optional<unsigned int> normalIndex; // Index of the normal in the OBJ file (1-based)

	explicit ObjFaceVertex(  unsigned int vIndex, 
                    std::optional<unsigned int> vtIndex = std::nullopt, 
                    std::optional<unsigned int> vnIndex = std::nullopt)
		: vertexIndex(vIndex), textureIndex(vtIndex), normalIndex(vnIndex) {}

	static ObjFaceVertex OnlyVertex(unsigned int vIndex) {
		return ObjFaceVertex(vIndex);
	}
	static ObjFaceVertex VertexAndTexture(unsigned int vIndex, unsigned int vtIndex) {
		return ObjFaceVertex(vIndex, vtIndex);
	}

	static ObjFaceVertex VertexAndNormal(unsigned int vIndex, unsigned int vnIndex) {
		return ObjFaceVertex(vIndex, std::nullopt, vnIndex);
	}

	static ObjFaceVertex VertexTextureNormal(unsigned int vIndex, unsigned int vtIndex, unsigned int vnIndex) {
		return ObjFaceVertex(vIndex, vtIndex, vnIndex);
	}
};

class ObjFace {
public:
	ObjFace(const std::vector<ObjFaceVertex>& vertices) : m_vertices(vertices) {}
	const std::vector<ObjFaceVertex>& GetVertices() const { return m_vertices; }
	void SetVertices(const std::vector<ObjFaceVertex>& vertices) { m_vertices = vertices; }
	void PrintFaceIndices(unsigned int faceIndex) const;

private:
    std::vector<ObjFaceVertex> m_vertices;
};


/*
 * Represents a face in an OBJ file.
 * f  v1/vt1/vn1   v2/vt2/vn2   v3/vt3/vn3 . . .

    Polygonal geometry statement.

    Specifies a face element and its vertex reference number. You can
    optionally include the texture vertex and vertex normal reference
    numbers.

    The reference numbers for the vertices, texture vertices, and
    vertex normals must be separated by slashes (/). There is no space
    between the number and the slash.

    v is the reference number for a vertex in the face element. A
    minimum of three vertices are required.

    vt is an optional argument.

    vt is the reference number for a texture vertex in the face
    element. It always follows the first slash.

    vn is an optional argument.

    vn is the reference number for a vertex normal in the face element.
    It must always follow the second slash.
 */
