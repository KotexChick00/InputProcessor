#pragma once
#include <vector>
#include <OBJ/ObjParser.h>

struct MeshRenderData{
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};

class ObjMeshConverter
{
public:
	static MeshRenderData ConvertToMeshRenderData(const ObjParser& objParser);
};

