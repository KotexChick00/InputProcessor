#pragma once
#include <string>

enum class FileFormat
{
	Ascii_OBJ, // Default assumption if no other format is detected
	Ascii_STL,
	Ascii_FBX,
	Binary_STL,
	Binary_FBX,
	Unknown
	//for now
};
class FormatChecker
{
public:
	static FileFormat checkFileFormat(const std::string& filename);
	static const char* formatToString(FileFormat format) {
		switch (format) {
		case FileFormat::Ascii_OBJ: return "ASCII OBJ";
		case FileFormat::Ascii_STL: return "ASCII STL";
		case FileFormat::Ascii_FBX: return "ASCII FBX";
		case FileFormat::Binary_STL: return "Binary STL";
		case FileFormat::Binary_FBX: return "Binary FBX";
		default: return "Unknown Format";
		}
	}
};

