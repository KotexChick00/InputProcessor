#include <OBJ/ObjParser.h>
#include <AsciiReader.h>
#include <StringUtils.h>
#include <sstream>
#include <stdexcept>
#include <vector>

ObjParser::ObjParser(const std::string& filename)
	: m_filename(filename)
{}

void ObjParser::Parse()
{
	AsciiReader reader(m_filename);
	reader.forEachLine([this](const std::string& line) {
		if (line.empty() || line[0] == '#') {
			return; // Bỏ qua dòng trống và comment
		}
		if (line[0] == 'v' && (line.size() > 1 && (line[1] == ' ' || line[1] == '\t'))) {
			ParseVertexLine(line);
		}
		else if (line[0] == 'f' && (line.size() > 1 && (line[1] == ' ' || line[1] == '\t'))) {
			ParseFaceLine(line);
		}
		// Tạm thời bỏ qua các loại dòng khác (vn, vt, etc.)
		});
}

std::vector<std::string> ObjParser::SplitString(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

// Chưa hỗ trợ x y z w, chỉ hỗ trợ x y z
void ObjParser::ParseVertexLine(const std::string& line)
{
	auto tokens = SplitString(line, ' ');

	// Filter out empty tokens produced by consecutive spaces so that
	// "v  1.0  2.0 3.0" becomes ["v","1.0","2.0","3.0"]
	std::vector<std::string> parts;
	parts.reserve(tokens.size());
	for (const auto& t : tokens) {
		if (!t.empty()) parts.push_back(t);
	}

	if (parts.size() < 4) {
		throw std::runtime_error("Invalid vertex line (not enough coordinates): " + line);
	}

	try {
		float x = StringToFloat(parts[1]);
		float y = StringToFloat(parts[2]);
		float z = StringToFloat(parts[3]);

		unsigned int vertexIndex = static_cast<unsigned int>(m_vertices.size()) + 1;
		m_vertices.emplace_back(x, y, z, vertexIndex);
	}
	catch (const std::runtime_error& e) {
		// Re-throw with line context for easier debugging
		throw std::runtime_error(std::string("Failed to parse vertex coordinates: ") + line + " (" + e.what() + ")");
	}
}

const ObjVertex& ObjParser::GetVertexAtIndex(unsigned int objIndex) const
{
	if (objIndex == 0 || objIndex > m_vertices.size()) {
		throw std::out_of_range("Vertex index out of range: " + std::to_string(objIndex));
	}
	return m_vertices[objIndex - 1]; // OBJ indices are 1-based
}

ObjFaceVertex ObjParser::ParseFaceVertex(const std::string& token)
{
	// token co the la "v", "v/vt", "v/vt/vn", hoac "v//vn"
// chi can lay phan v (vi tri vertex)
	size_t slashPos = token.find('/');
	std::string vertPart = (slashPos == std::string::npos)
		? token
		: token.substr(0, slashPos);

	// OBJ index von da bat dau tu 1, giu nguyen khong chuyen doi
	unsigned int vertexIndex = std::stoul(vertPart);
	return ObjFaceVertex(vertexIndex);
}

void ObjParser::ParseFaceLine(const std::string& line)
{
	std::vector<std::string> tokens = SplitString(line, ' ');
	if (tokens.size() < 4) {
		throw std::runtime_error("Dong f khong hop le: " + line);
	}

	std::vector<ObjFaceVertex> faceVertices;
	faceVertices.reserve(tokens.size() - 1); // tokens[0] la "f", nen chi can reserve tokens.size() - 1
	for (size_t i = 1; i < tokens.size(); ++i) {
		faceVertices.push_back(ParseFaceVertex(tokens[i]));
	}
	m_faces.push_back(ObjFace(faceVertices));
}