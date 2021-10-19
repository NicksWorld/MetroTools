cmake_minimum_required(VERSION 3.16)
project(MetroTools LANGUAGES CXX)

add_library(metro STATIC)
add_library(MetroTools::Metro ALIAS metro)
set_target_properties(metro PROPERTIES CXX_STANDARD 17)
target_include_directories(metro PUBLIC ..)
target_sources(metro PRIVATE
    MetroBinArchive.cpp
    MetroBinArchive.h
    MetroBinArrayArchive.cpp
    MetroBinArrayArchive.h
    MetroCompression.cpp
    MetroCompression.h
    MetroConfigDatabase.cpp
    MetroConfigDatabase.h
    MetroConfigNames.h
    MetroContext.cpp
    MetroContext.h
    MetroFileSystem.cpp
    MetroFileSystem.h
    MetroFonts.cpp
    MetroFonts.h
    MetroLevel.cpp
    MetroLevel.h
    MetroLightProbe.cpp
    MetroLightProbe.h
    MetroLocalization.cpp
    MetroLocalization.h
    MetroMaterialsDatabase.cpp
    MetroMaterialsDatabase.h
    MetroModel.cpp
    MetroModel.h
    MetroMotion.cpp
    MetroMotion.h
    MetroSkeleton.cpp
    MetroSkeleton.h
    MetroSound.cpp
    MetroSound.h
    MetroTexture.cpp
    MetroTexture.h
    MetroTexturesDatabase.cpp
    MetroTexturesDatabase.h
    MetroTypedStrings.cpp
    MetroTypedStrings.h
    MetroTypes.h
    MetroWeaponry.cpp
    MetroWeaponry.h
    VFIReader.cpp
    VFIReader.h
    VFXReader.cpp
    VFXReader.h
    classes/MetroClasses.h
    entities/MetroEntity.cpp
    entities/MetroEntityFactory.cpp
    entities/MetroUObject.cpp
    entities/MetroEntity.h
    entities/MetroEntityFactory.h
    entities/MetroUObject.h
    physics/MetroPhysics.cpp
    physics/MetroPhysics.h
    reflection/MetroReflection.cpp
    reflection/MetroReflectionJson.cpp
    reflection/MetroReflection.h
    reflection/MetroReflectionBinary.inl
    reflection/MetroReflectionJson.inl
    scripts/MetroBlock.cpp
    scripts/MetroBlock.h
    scripts/MetroBlockFactory.cpp
    scripts/MetroBlockFactory.h
    scripts/MetroBlockMeta.h
    scripts/MetroScript.cpp
    scripts/MetroScript.h
    vfs/MetroVFS.cpp
    vfs/MetroVFS.h)
target_link_libraries(metro 
    PRIVATE
        MetroTools::Common

        pugixml
        Stb::Stb
        Lz4::Lz4
        QuickLz::QuickLz
        Jansson::Jansson
    PUBLIC
        Crunch::Crunch
        Physx284::Physx284)
