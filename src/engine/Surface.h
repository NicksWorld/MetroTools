#pragma once
#include "Texture.h"

namespace u4a {

struct Surface {
    Texture*    base;
    Texture*    normal;
    Texture*    bump;
};

} // namespace u4a
