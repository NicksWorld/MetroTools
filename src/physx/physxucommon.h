#pragma once
#include "mycommon.h"
#include "mymath.h"

// common interface to rule 'em all
struct IPhysXUtils {
    virtual bool CookedTriMeshToRawVertices(const void* cookedData, const size_t cookedDataSize, MyArray<vec3>& vertices, MyArray<uint16_t>& indices) = 0;
    virtual bool RawVerticesToCookedTriMesh(const vec3* vertices, const size_t numVertices, const uint16_t* indices, const size_t numIndices, BytesArray& cookedData) = 0;
};
