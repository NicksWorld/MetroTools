set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

# PhysX 2 Utils
add_library(physx2utils SHARED)
add_library(Physx2utils::Physx2utils ALIAS physx2utils)

set_target_properties(physx2utils PROPERTIES CXX_STANDARD 17)
target_sources(physx2utils PRIVATE
    physx2utils/physx2utils.cpp
)

target_link_libraries(physx2utils
    PRIVATE
        MetroTools::Common

    PUBLIC
        Physx284::Physx284
        PhysXLoader64.lib
        PhysXCore64.lib
        PhysXCooking64.lib
        NxCharacter64.lib
        Delayimp.lib
)
target_link_options(physx2utils PRIVATE
    "/DELAYLOAD:cudart64_30_9.dll"
    "/DELAYLOAD:NxCharacter64.dll"
    "/DELAYLOAD:PhysXCooking64.dll"
    "/DELAYLOAD:PhysXCore64.dll"
    "/DELAYLOAD:PhysXDevice64.dll"
    "/DELAYLOAD:PhysXLoader64.dll"
)
target_include_directories(physx2utils INTERFACE physx2utils)


# PhysX 3 Utils
add_library(physx3utils SHARED)
add_library(Physx3utils::Physx3utils ALIAS physx3utils)

set_target_properties(physx3utils PROPERTIES CXX_STANDARD 17)
target_sources(physx3utils PRIVATE
    physx3utils/physx3utils.cpp
)

target_link_libraries(physx3utils
    PRIVATE
        MetroTools::Common

    PUBLIC
        Physx34::Physx34
        PhysX3_x64.lib
        PhysX3Common_x64.lib
        PhysX3Cooking_x64.lib
        PxFoundation_x64.lib
        Delayimp.lib
)
target_link_options(physx3utils PRIVATE
    "/DELAYLOAD:PhysX3_x64.dll"
    "/DELAYLOAD:PhysX3CharacterKinematic_x64.dll"
    "/DELAYLOAD:PhysX3Common_x64.dll"
    "/DELAYLOAD:PhysX3Cooking_x64.dll"
    "/DELAYLOAD:PhysX3Gpu_x64.dll"
    "/DELAYLOAD:PhysXDevice64.dll"
    "/DELAYLOAD:PxFoundation_x64.dll"
)
target_include_directories(physx3utils INTERFACE physx3utils)

set_target_properties(physx2utils physx3utils uninstall PROPERTIES FOLDER "external")
