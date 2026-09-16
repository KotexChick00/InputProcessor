#include <OBJ/ObjFace.h>
#include <iostream>

void ObjFace::PrintFaceIndices(unsigned int faceIndex) const
{
    std::cout << "Face " << faceIndex << ": ";
    for (const auto& vertex : m_vertices)
    {
        std::cout << vertex.vertexIndex;

		if (!vertex.textureIndex.has_value() && !vertex.normalIndex.has_value())
		{
			// Only vertex index
		}
		else if (vertex.textureIndex.has_value() && !vertex.normalIndex.has_value())
		{
			std::cout << "/" << vertex.textureIndex.value();
		}
		else if (!vertex.textureIndex.has_value() && vertex.normalIndex.has_value())
		{
			std::cout << "//" << vertex.normalIndex.value();
		}
		else if (vertex.textureIndex.has_value() && vertex.normalIndex.has_value())
		{
			std::cout << "/" << vertex.textureIndex.value() << "/" << vertex.normalIndex.value();
		}

        std::cout << " ";
    }
    std::cout << std::endl;
}