#include <OBJ/ObjMeshConverter.h>
#include <stdexcept>

MeshRenderData ObjMeshConverter::ConvertToMeshRenderData(const ObjParser& objParser)
{
	MeshRenderData meshData;

	const auto& objVertices = objParser.GetVertices();
	
	meshData.vertices.reserve(objVertices.size() * 3); // Reserve space for x, y, z for each vertex

	for (const auto& objVertex : objVertices) {
		const auto& vertex = objVertex.GetVertex();
		meshData.vertices.push_back(vertex.GetX());
		meshData.vertices.push_back(vertex.GetY());
		meshData.vertices.push_back(vertex.GetZ());
	}

	const auto& objFaces = objParser.GetFaces();

	meshData.indices.reserve(objFaces.size() * 3); // Reserve space for 3 indices per face

	for (const auto& objFace : objFaces) {
		const auto& faceVertices = objFace.GetVertices();
		if (faceVertices.size() < 3) {
			throw std::runtime_error("Invalid face with less than 3 vertices.");
		}
		// Triangulate the face if it has more than 3 vertices
		for (size_t i = 1; i < faceVertices.size() - 1; ++i) {
			meshData.indices.push_back(faceVertices[0].vertexIndex - 1); // Convert to 0-based index
			meshData.indices.push_back(faceVertices[i].vertexIndex - 1);
			meshData.indices.push_back(faceVertices[i + 1].vertexIndex - 1);
		}
	}

	return meshData;
}

// Note: use a local include so IntelliSense resolves the header correctly.
