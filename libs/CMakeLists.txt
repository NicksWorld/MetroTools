set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(GLM_TEST_ENABLE OFF)
add_subdirectory(glm)

add_subdirectory(pugixml)

# set(JANSSON_BUILD_DOCS OFF)
# add_subdirectory(jansson)
add_library(jansson STATIC)
add_library(Jansson::Jansson ALIAS jansson)
target_compile_definitions(jansson PUBLIC HAVE_CONFIG_H=1)
target_include_directories(jansson PUBLIC jansson/src)
target_sources(jansson PRIVATE
    jansson/src/dump.c
    jansson/src/error.c
    jansson/src/hashtable.c
    jansson/src/hashtable_seed.c
    jansson/src/load.c
    jansson/src/memory.c
    jansson/src/strbuffer.c
    jansson/src/strconv.c
    jansson/src/utf.c
    jansson/src/value.c
    jansson/src/jansson.h)

add_library(stb INTERFACE)
add_library(Stb::Stb ALIAS stb)
target_include_directories(stb INTERFACE stb)

add_library(tinyobjloader INTERFACE)
add_library(TinyObjLoader::TinyObjLoader ALIAS tinyobjloader)
target_include_directories(tinyobjloader INTERFACE tinyobjloader)

add_library(lz4 STATIC)
add_library(Lz4::Lz4 ALIAS lz4)
target_include_directories(lz4 PUBLIC lz4)
target_sources(lz4 PRIVATE
    lz4/lz4hc.c
    lz4/lz4.c
    lz4/lz4.h
    lz4/lz4hc.h)

add_library(quicklz STATIC)
add_library(QuickLz::QuickLz ALIAS quicklz)
target_include_directories(quicklz PUBLIC quicklz)
target_sources(quicklz PRIVATE
    quicklz/quicklz.c
    quicklz/quicklz.h)

add_library(bc7enc16 STATIC)
add_library(Bc7enc16::Bc7enc16 ALIAS bc7enc16)
target_include_directories(bc7enc16 PUBLIC bc7enc16)
target_sources(bc7enc16 PRIVATE
    bc7enc16/bc7decomp.c
    bc7enc16/bc7enc16.c
    bc7enc16/bc7decomp.h
    bc7enc16/bc7enc16.h
    bc7enc16/dds_defs.h)

add_library(xxhash STATIC)
add_library(Xxhash::Xxhash ALIAS xxhash)
target_include_directories(xxhash PUBLIC xxhash)
target_sources(xxhash PRIVATE
    xxhash/xxhash.c
    xxhash/xxhash.h)

add_library(fbx INTERFACE)
add_library(Fbx::Fbx ALIAS fbx)
target_include_directories(fbx INTERFACE fbx/include)
target_link_directories(fbx INTERFACE fbx/lib/x64)

add_library(physx284 INTERFACE)
add_library(Physx284::Physx284 ALIAS physx284)
target_include_directories(physx284 INTERFACE
    physx284/Foundation/include
    physx284/Physics/include
    physx284/Cooking/include
    physx284/PhysXLoader/include
)
target_link_directories(physx284 INTERFACE physx284/lib/win64)

add_library(physx34 INTERFACE)
add_library(Physx34::Physx34 ALIAS physx34)
target_include_directories(physx34 INTERFACE
    physx34/Include
)
target_link_directories(physx34 INTERFACE physx34/lib)

add_library(crunch INTERFACE)
add_library(Crunch::Crunch ALIAS crunch)
target_include_directories(crunch INTERFACE crunch/inc)
target_link_directories(crunch INTERFACE crunch/lib/VC9)
target_link_libraries(crunch INTERFACE $<IF:$<CONFIG:Debug>,Debug_DLL/x64/crnlibD_DLL_x64_VC9,Release_DLL/x64/crnlib_DLL_x64_VC9>)

set(BUILD_EXAMPLES ON)
add_subdirectory(nodeeditor)

add_subdirectory(QtPropertyBrowser)

add_subdirectory(multitoolbox)

add_library(tinygltf INTERFACE)
add_library(Tinygltf::Tinygltf ALIAS tinygltf)
target_include_directories(tinygltf INTERFACE tinygltf)

add_library(embree3 INTERFACE)
add_library(Embree3::Embree3 ALIAS embree3)
target_include_directories(embree3 INTERFACE embree3/include)
target_link_directories(embree3 INTERFACE embree3/lib)

add_library(tbb INTERFACE)
add_library(Tbb::Tbb ALIAS tbb)
target_include_directories(tbb INTERFACE tbb/include)
target_link_directories(tbb INTERFACE tbb/lib/intel64/vc14)

set_target_properties(bc7enc16 glm_shared glm_static jansson lz4 pugixml quicklz xxhash uninstall PROPERTIES FOLDER "external")
