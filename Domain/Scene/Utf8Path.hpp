#pragma once
#include <filesystem>
#include <string>

// Đổi qua lại giữa std::string UTF-8 và std::filesystem::path.
// Trên Windows, path(std::string) và path::string() dùng bảng mã ANSI của hệ thống: tên có tiếng Việt
// có thể bị sai hoặc ném exception. Hai hàm này luôn dùng UTF-8 (trên Linux không đổi gì).
namespace Domain::Scene {
	inline std::filesystem::path PathFromUtf8(const std::string& text) {
		return std::filesystem::path(std::u8string(text.begin(), text.end()));
	}

	inline std::string PathToUtf8(const std::filesystem::path& path) {
		const std::u8string text = path.generic_u8string();
		return std::string(text.begin(), text.end());
	}
}
