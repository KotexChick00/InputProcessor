#pragma once
#include <fstream>
#include <string>
#include <functional>

class AsciiReader {
public:
    explicit AsciiReader(const std::string& filename);

    // Đọc từng dòng, gọi callback ngay khi đọc — không giữ dòng nào trong bộ nhớ
    void forEachLine(const std::function<void(const std::string&)>& callback);

private:
    std::ifstream file_;
};