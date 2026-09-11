#include <OBJ/ObjParser.h>
#include <AsciiReader.h>
#include <StringUtils.h>
#include <sstream>
#include <stdexcept>

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
		if (line[0] == 'v' && (line[1] == ' ' || line[1] == '\t')) {
			ParseVertexLine(line);
		}
		else if (line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
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
	if (tokens.size() < 4) {
		throw std::runtime_error("Invalid vertex line: " + line);
	}
	float x = StringToFloat(tokens[1]);
	float y = StringToFloat(tokens[2]);
	float z = StringToFloat(tokens[3]);

	unsigned int vertexIndex = static_cast<unsigned int>(m_vertices.size()) + 1;

	m_vertices.emplace_back(x, y, z, vertexIndex);
}

const ObjVertex& ObjParser::GetVertexAtIndex(unsigned int objIndex) const
{
	if (objIndex == 0 || objIndex > m_vertices.size()) {
		throw std::out_of_range("Vertex index out of range: " + std::to_string(objIndex));
	}
	return m_vertices[objIndex - 1]; // OBJ indices are 1-based
}

unsigned int ObjParser::ParseFaceIndex(const std::string& token)
{
	// token co the la "v", "v/vt", "v/vt/vn", hoac "v//vn"
// chi can lay phan v (vi tri vertex)
	size_t slashPos = token.find('/');
	std::string vertPart = (slashPos == std::string::npos)
		? token
		: token.substr(0, slashPos);

	// OBJ index von da bat dau tu 1, giu nguyen khong chuyen doi
	return StringToUnsignedInt(vertPart);
}

void ObjParser::ParseFaceLine(const std::string& line)
{
	std::vector<std::string> tokens = SplitString(line, ' ');
	if (tokens.size() < 4) {
		throw std::runtime_error("Dong f khong hop le: " + line);
	}

	unsigned int i0 = ParseFaceIndex(tokens[1]);
	unsigned int iPrev = ParseFaceIndex(tokens[2]);

	// Tam giac hoa kieu "fan": (i0, i1, i2), (i0, i2, i3), (i0, i3, i4)...
	// Voi face tam giac binh thuong (4 token), vong lap chi chay 1 lan.
	for (size_t k = 3; k < tokens.size(); ++k) {
		unsigned int iCur = ParseFaceIndex(tokens[k]);
		m_faces.emplace_back(i0, iPrev, iCur);
		iPrev = iCur;
	}
}