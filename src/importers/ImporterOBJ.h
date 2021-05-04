#pragma once
#include "mycommon.h"
#include "metro/MetroTypes.h"

class MetroModelBase;
class MetroModelStd;

namespace tinyobj {
    struct attrib_t;
    struct shape_t;
}

class ImporterOBJ {
public:
    ImporterOBJ();
    ~ImporterOBJ();

    RefPtr<MetroModelBase>  ImportModel(const fs::path& path);

private:
    RefPtr<MetroModelStd>   CreateStdModel(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attrib);
};
