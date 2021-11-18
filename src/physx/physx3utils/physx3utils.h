#pragma once
#include "../physxucommon.h"

#ifndef PHYSX3UTILS_IMPORT
#define MY_DLL_EXPORT __declspec(dllexport)
#else
#define MY_DLL_EXPORT __declspec(dllimport)
#endif

struct IPhysX3Utils : public IPhysXUtils {
};

namespace nux3 {
    constexpr size_t    kInitCooker = 1;

    MY_DLL_EXPORT bool  CreatePhysX3Utils(const size_t initFlags, IPhysX3Utils** utils);
    MY_DLL_EXPORT void  DestroyPhysX3Utils(IPhysX3Utils* utils);
} // namespace nux2
