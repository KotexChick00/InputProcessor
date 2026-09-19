#include <FormatChecker.h>
#include <fstream>
#include <array>
#include <string_view>

FileFormat FormatChecker::checkFileFormat(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	std::array<char, 84> header{};
	file.read(header.data(), header.size());
	size_t bytesRead = file.gcount();

	// Magic bytes check for Binary FBX
	if (bytesRead >= 20
		&& std::string_view(header.data(), 20) == "Kaydara FBX Binary  ") {
		return FileFormat::Binary_FBX;
	}

	// Coincidence
	if (bytesRead >= 5){
		std::string_view magic(header.data(), 5);

		// Magic bytes check for ASCII STL
		if (magic == "solid") return FileFormat::Ascii_STL;

		// Magic bytes check for ASCII FBX
		if (magic == "; FBX") return FileFormat::Ascii_FBX;
	}

	// File size check for Binary STL
	// Binary STL files have a 80-byte header followed by a 4-byte unsigned int for the number of triangles. Then each triangle is 50 bytes. So the total size should be 84 + 50 * numTriangles.
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	if (fileSize >= 84) {
		file.seekg(80, std::ios::beg);
		uint32_t numTriangles;
		file.read(reinterpret_cast<char*>(&numTriangles), sizeof(numTriangles));
		size_t expectedSize = 84 + static_cast<size_t>(numTriangles) * 50;
		if (fileSize == expectedSize) {
			return FileFormat::Binary_STL;
		}
	}

	// If none of the above checks passed, we may assume it's a Text OBJ file. For now.
	return FileFormat::Ascii_OBJ;

}