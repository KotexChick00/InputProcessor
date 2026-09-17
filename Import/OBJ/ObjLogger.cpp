#include <OBJ/ObjLogger.h>
#include <iostream>

void ObjLogger::LogParsedData(const ObjParser& objParser)
{
	const auto& vertices = objParser.GetVertices();
	const auto& faces = objParser.GetFaces();

	std::cout << "===== OBJ Parse Result =====" << std::endl;
	std::cout << "Total Vertices: " << vertices.size() << std::endl;
	std::cout << "Total Faces: " << faces.size() << std::endl;
	std::cout << "-----------------------------" << std::endl;

	unsigned int vertIdx = 1; // OBJ 1-based
	for (const auto& v : vertices)
	{
		v.PrintVertexCoordinates(vertIdx);
		++vertIdx;
	}

	std::cout << "-----------------------------" << std::endl;

	unsigned int faceIdx = 1;
	for (const auto& f : faces)
	{
		f.PrintFaceIndices(faceIdx);
		++faceIdx;
	}

	std::cout << "=============================" << std::endl;
}