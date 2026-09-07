#include <AsciiReader.h>
#include <stdexcept>

AsciiReader::AsciiReader(const std::string& filename)
    : file_(filename) {
    if (!file_.is_open()) {
        throw std::runtime_error("Không mở được file: " + filename);
    }
}

void AsciiReader::forEachLine(const std::function<void(const std::string&)>& callback) {
    std::string line;
    line.reserve(256); // tránh cấp phát lại bộ nhớ nhiều lần cho mỗi dòng

    while (std::getline(file_, line)) {
        callback(line);
    }
}