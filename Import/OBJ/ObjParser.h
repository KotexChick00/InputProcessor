#pragma once
#include <string>
#include <vector>
#include <OBJ/ObjVertex.h>
#include <OBJ/ObjFace.h>

class ObjParser
{
public:
	explicit ObjParser(const std::string& filename);

	void Parse();

	const std::vector<ObjVertex>& GetVertices() const { return m_vertices; }
	const std::vector<ObjFace>& GetFaces() const { return m_faces; }
	const ObjVertex& GetVertexAtIndex(unsigned int objIndex) const;
	const unsigned int RelativeToAbsoluteVertexIndex(int negativeIndex) const;

private:
	void ParseVertexLine(const std::string& line);
	void ParseFaceLine(const std::string& line);
	static std::vector<std::string> SplitString(const std::string& str, char delimiter);
	static ObjFaceVertex ParseFaceVertex(const std::string& token);


	std::string m_filename;
	std::vector<ObjVertex> m_vertices;
	std::vector<ObjFace> m_faces;
};

