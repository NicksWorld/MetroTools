#include "mycommon.h"
#include <fstream>

MemStream OSReadFile(const fs::path& filePath) {
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

MemStream OSReadFileEX(const fs::path& filePath, const size_t subOffset, const size_t subLength) {
    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        file.seekg(0, std::ios::end);
        const size_t fileSize = file.tellg();

        if (subOffset < fileSize) {
            const size_t chunkSize = ((subOffset + subLength) <= fileSize) ? subLength : (fileSize - subOffset);

            void* fileData = malloc(chunkSize);
            file.seekg(subOffset, std::ios::beg);
            file.read(rcast<char*>(fileData), chunkSize);
            file.close();

            return std::move(MemStream(fileData, fileSize, true));
        } else {
            return MemStream{};
        }
    } else {
        return MemStream{};
    }
}

size_t OSWriteFile(const fs::path& filePath, const void* data, const size_t dataLength) {
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

size_t OSGetFileSize(const fs::path& filePath) {
    size_t result = 0;

    std::ifstream file(filePath, std::ifstream::binary);
    if (file.good()) {
        file.seekg(0, std::ios::end);
        result = file.tellg();
        file.close();
    }

    return result;
}

bool OSPathExists(const fs::path& pathToCheck) {
    std::error_code ec;
    return fs::exists(pathToCheck, ec);
}

bool OSPathIsFile(const fs::path& pathToCheck) {
    std::error_code ec;
    return fs::is_regular_file(pathToCheck, ec);
}

bool OSPathIsFolder(const fs::path& pathToCheck) {
    std::error_code ec;
    return fs::is_directory(pathToCheck, ec);
}

MyArray<fs::path> OSPathGetEntriesList(const fs::path& pathToCheck, const bool recursive, const bool onlyFiles, const fs::path& ext) {
    MyArray<fs::path> result;

    if (OSPathIsFolder(pathToCheck)) {
        std::error_code ec;

        for (auto& entry : fs::directory_iterator(pathToCheck)) {
            if (entry.is_regular_file(ec)) {
                if (ext.empty() || ext == entry.path().extension()) {
                    result.emplace_back(entry.path());
                }
            }
            if (entry.is_directory(ec)) {
                if (!onlyFiles) {
                    result.emplace_back(entry.path());
                }

                if (recursive) {
                    const MyArray<fs::path>& innerList = OSPathGetEntriesList(entry.path(), recursive, onlyFiles);
                    result.insert(result.end(), innerList.begin(), innerList.end());
                }
            }
        }
    }

    return result;
}
