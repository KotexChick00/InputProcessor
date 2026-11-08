#include <FormatChecker.h>
#include <fstream>
#include <array>
#include <string_view>
#include <cstdint>

FileFormat FormatChecker::checkFileFormat(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        // If the file does not exist or cannot be opened, return Unknown so the
        // caller can distinguish "missing file" from "OBJ by default". The
        // previous implementation fell through to `return FileFormat::Ascii_OBJ`
        // for any unreadable file, which silently mis-routed missing files into
        // the OBJ parser.
        if (!file.is_open()) {
                return FileFormat::Unknown;
        }

        std::array<char, 84> header{};
        file.read(header.data(), header.size());
        size_t bytesRead = static_cast<size_t>(file.gcount());

        // Magic bytes check for Binary FBX. The official magic is
        // "Kaydara FBX Binary  \x00\x1A\x00" (23 bytes); checking the first
        // 20 bytes is sufficient in practice and is the historical behavior.
        if (bytesRead >= 20
                && std::string_view(header.data(), 20) == "Kaydara FBX Binary  ") {
                return FileFormat::Binary_FBX;
        }

        bool startsWithSolid = false;
        if (bytesRead >= 5) {
                std::string_view magic(header.data(), 5);

                // Many binary STL writers (Fusion360, SolidWorks, ...) put the
                // word "solid" inside the 80-byte binary header. We therefore
                // cannot conclude Ascii_STL from the magic alone; we set a flag
                // and let the binary-STL size check below make the final call.
                if (magic == "solid") startsWithSolid = true;

                // Magic bytes check for ASCII FBX
                if (magic == "; FBX") return FileFormat::Ascii_FBX;
        }

        // File size check for Binary STL.
        // Binary STL files have an 80-byte header followed by a 4-byte unsigned
        // int (number of triangles), then 50 bytes per triangle. Total size =
        // 84 + 50 * numTriangles.
        file.clear(); // clear EOF flag from the earlier read so seekg/tellg work
        file.seekg(0, std::ios::end);
        std::streampos endPos = file.tellg();
        // tellg() returns pos_type(-1) on failure; cast to size_t would yield
        // ~0ULL and produce nonsense comparisons below.
        if (endPos == std::streampos(-1)) {
                return FileFormat::Unknown;
        }
        size_t fileSize = static_cast<size_t>(endPos);
        if (fileSize >= 84) {
                file.seekg(80, std::ios::beg);
                // Read 4 bytes and assemble little-endian explicitly. The
                // previous `reinterpret_cast<uint32_t*>(&numTriangles)` assumed
                // the host is little-endian, which is wrong on big-endian
                // targets and is technically UB via strict aliasing.
                unsigned char bytes[4] = {0, 0, 0, 0};
                file.read(reinterpret_cast<char*>(bytes), 4);
                if (file.gcount() == 4) {
                        uint32_t numTriangles =
                            static_cast<uint32_t>(bytes[0])
                          | (static_cast<uint32_t>(bytes[1]) << 8)
                          | (static_cast<uint32_t>(bytes[2]) << 16)
                          | (static_cast<uint32_t>(bytes[3]) << 24);
                        // Use >= instead of == to accept STL files with
                        // trailing metadata/padding/comments. (Exact == is
                        // the historical behavior but produces false negatives
                        // for many real-world binary STL exports.)
                        size_t expectedSize = 84 + static_cast<size_t>(numTriangles) * 50;
                        if (fileSize >= expectedSize) {
                                return FileFormat::Binary_STL;
                        }
                }
        }

        // If we got here and the file started with "solid", it is plausibly
        // an ASCII STL (we only reach this branch if the binary size check
        // failed).
        if (startsWithSolid) {
                return FileFormat::Ascii_STL;
        }

        // If none of the above checks passed, we may assume it's a Text OBJ
        // file. For now.
        return FileFormat::Ascii_OBJ;
}
