#pragma once
#include "../physxucommon.h"

#ifndef PHYSX2UTILS_IMPORT
#define MY_DLL_EXPORT __declspec(dllexport)
#else
#define MY_DLL_EXPORT __declspec(dllimport)
#endif

struct IPhysX2Utils : public IPhysXUtils {
};

namespace nux2 {
    constexpr size_t    kInitCooker = 1;

    MY_DLL_EXPORT bool  CreatePhysX2Utils(const size_t initFlags, IPhysX2Utils** utils);
    MY_DLL_EXPORT void  DestroyPhysX2Utils(IPhysX2Utils* utils);
} // namespace nux2
