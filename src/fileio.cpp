#include "mycommon.h"
#include <fstream>

MemStream ReadOSFile(const fs::path& filePath) {
    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        void* fileData = nullptr;

        file.seekg(0, std::ios::end);
        const size_t fileSize = file.tellg();
        fileData = malloc(fileSize);
        file.seekg(0, std::ios::beg);
        file.read(rcast<char*>(fileData), fileSize);
        file.close();

        return std::move(MemStream(fileData, fileSize, true));
    } else {
        return MemStream{};
    }
}

size_t WriteOSFile(const fs::path& filePath, const void* data, const size_t dataLength) {
    size_t result = 0;

    std::ofstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        file.write(rcast<const char*>(data), dataLength);
        result = file.tellp();
        file.flush();
        file.close();
    }

    return result;
}

bool OSPathExists(const fs::path& pathToCheck) {
    std::error_code ec;
    return fs::exists(pathToCheck, ec);
}
