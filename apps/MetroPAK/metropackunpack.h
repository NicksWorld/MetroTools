#pragma once

#include "mycommon.h"

struct MetroPackUnpack {
    static void UnpackArchive(const fs::path& archivePath, const fs::path& outputFolderPath, std::function<bool(float)> progress);
    static void PackArchive2033(const fs::path& contentFolderPath, const fs::path& archivePath, std::function<bool(float)> progress);
};
