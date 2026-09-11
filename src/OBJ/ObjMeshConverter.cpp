#include <OBJ/ObjMeshConverter.h>

MeshRenderData ObjMeshConverter::ConvertToMeshRenderData(const ObjParser& objParser)
{
	MeshRenderData meshData;

	const auto& objVertices = objParser.GetVertices();
	
	meshData.vertices.reserve(objVertices.size() * 3); // Reserve space for x, y, z for each vertex

	for (const auto& objVertex : objVertices)
	{
		const Vector3& vertex = objVertex.GetVertex();
		meshData.vertices.push_back(vertex.GetX());
		meshData.vertices.push_back(vertex.GetY());
		meshData.vertices.push_back(vertex.GetZ());
	}

	const auto& objFaces = objParser.GetFaces();
	meshData.indices.reserve(objFaces.size() * 3); // Reserve space for 3 indices per face

	for (const auto& objFace : objFaces)
	{
		meshData.indices.push_back(objFace.GetIndexOfVertex(0) - 1 );
		meshData.indices.push_back(objFace.GetIndexOfVertex(1) - 1 );
		meshData.indices.push_back(objFace.GetIndexOfVertex(2) - 1 );
		// Note: OBJ indices are 1-based, so we subtract 1 to convert to 0-based indices
	}

	return meshData;
}

// Note: use a local include so IntelliSense resolves the header correctly.
