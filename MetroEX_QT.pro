QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += libs/lz4/lz4hc.c \
    libs/lz4/lz4qt.c \
    libs/bc7enc16/bc7decomp.c \
    libs/bc7enc16/bc7enc16.c \
    libs/jansson/src/dump.c \
    libs/jansson/src/error.c \
    libs/jansson/src/hashtable.c \
    libs/jansson/src/hashtable_seed.c \
    libs/jansson/src/load.c \
    libs/jansson/src/memory.c \
    libs/jansson/src/strbuffer.c \
    libs/jansson/src/strconv.c \
    libs/jansson/src/utf.c \
    libs/jansson/src/value.c \
    libs/quicklz/quicklz.c \
    libs/xxhash/xxhashqt.c \
    libs/pugixml/src/pugixml.cpp \
    src/engine/Animator.cpp \
    src/engine/Camera.cpp \
    src/engine/LevelGeo.cpp \
    src/engine/Model.cpp \
    src/engine/Renderer.cpp \
    src/engine/ResourcesManager.cpp \
    src/engine/Scene.cpp \
    src/engine/Spawner.cpp \
    src/engine/Swapchain.cpp \
    src/engine/Texture.cpp \
    src/engine/scenenodes/LevelGeoNode.cpp \
    src/engine/scenenodes/ModelNode.cpp \
    src/engine/scenenodes/SceneNode.cpp \
    src/exporters/ExporterFBX.cpp \
    src/exporters/ExporterOBJ.cpp \
    src/main.cpp \
    src/ui/imagepanel.cpp \
    src/ui/modelinfopanel.cpp \
    src/ui/renderpanel.cpp \
    src/ui/imageinfopanel.cpp \
    src/ui/mainwindow.cpp \
    src/dds_utils.cpp \
    src/encoding.cpp \
    src/fileio.cpp \
    src/hashing.cpp \
    src/log.cpp \
    src/metro/MetroBinArchive.cpp \
    src/metro/MetroBinArrayArchive.cpp \
    src/metro/MetroCompression.cpp \
    src/metro/MetroConfigDatabase.cpp \
    src/metro/MetroContext.cpp \
    src/metro/MetroFileSystem.cpp \
    src/metro/MetroFonts.cpp \
    src/metro/MetroLevel.cpp \
    src/metro/MetroLightProbe.cpp \
    src/metro/MetroLocalization.cpp \
    src/metro/MetroMaterialsDatabase.cpp \
    src/metro/MetroModel.cpp \
    src/metro/MetroMotion.cpp \
    src/metro/MetroSkeleton.cpp \
    src/metro/MetroSound.cpp \
    src/metro/MetroTexture.cpp \
    src/metro/MetroTexturesDatabase.cpp \
    src/metro/MetroTypedStrings.cpp \
    src/metro/MetroWeaponry.cpp \
    src/metro/VFIReader.cpp \
    src/metro/VFXReader.cpp \
    src/metro/entities/MetroEntity.cpp \
    src/metro/entities/MetroEntityFactory.cpp \
    src/metro/entities/MetroUObject.cpp \
    src/metro/reflection/MetroReflection.cpp \
    src/metro/reflection/MetroReflectionJson.cpp \
    src/metro/vfs/MetroVFS.cpp \
    src/mex_settings.cpp \
    src/ui/settingsdlg.cpp

HEADERS += \
    libs/bc7enc16/bc7decomp.h \
    libs/bc7enc16/bc7enc16.h \
    libs/bc7enc16/dds_defs.h \
    libs/jansson/src/jansson.h \
    libs/lz4/lz4.h \
    libs/lz4/lz4hc.h \
    libs/pugixml/src/pugixml.hpp \
    libs/quicklz/quicklz.h \
    libs/xxhash/xxhash.h \
    src/engine/Animator.h \
    src/engine/Camera.h \
    src/engine/LevelGeo.h \
    src/engine/Model.h \
    src/engine/Renderer.h \
    src/engine/RendererTypes.inl \
    src/engine/ResourcesManager.h \
    src/engine/Scene.h \
    src/engine/Spawner.h \
    src/engine/Surface.h \
    src/engine/Swapchain.h \
    src/engine/Texture.h \
    src/engine/scenenodes/LevelGeoNode.h \
    src/engine/scenenodes/ModelNode.h \
    src/engine/scenenodes/SceneNode.h \
    src/engine/shaders/all_shaders.h \
    src/exporters/ExporterFBX.h \
    src/exporters/ExporterOBJ.h \
    src/ui/imagepanel.h \
    src/ui/modelinfopanel.h \
    src/ui/renderpanel.h \
    src/ui/imageinfopanel.h \
    src/ui/mainwindow.h \
    src/dds_utils.h \
    src/log.h \
    src/metro/MetroBinArchive.h \
    src/metro/MetroBinArrayArchive.h \
    src/metro/MetroCompression.h \
    src/metro/MetroConfigDatabase.h \
    src/metro/MetroConfigNames.h \
    src/metro/MetroContext.h \
    src/metro/MetroFileSystem.h \
    src/metro/MetroFonts.h \
    src/metro/MetroLevel.h \
    src/metro/MetroLightProbe.h \
    src/metro/MetroLocalization.h \
    src/metro/MetroMaterialsDatabase.h \
    src/metro/MetroModel.h \
    src/metro/MetroMotion.h \
    src/metro/MetroSkeleton.h \
    src/metro/MetroSound.h \
    src/metro/MetroTexture.h \
    src/metro/MetroTexturesDatabase.h \
    src/metro/MetroTypedStrings.h \
    src/metro/MetroTypes.h \
    src/metro/MetroWeaponry.h \
    src/metro/VFIReader.h \
    src/metro/VFXReader.h \
    src/metro/classes/MetroClasses.h \
    src/metro/entities/MetroEntity.h \
    src/metro/entities/MetroEntityFactory.h \
    src/metro/entities/MetroUObject.h \
    src/metro/reflection/MetroReflection.h \
    src/metro/reflection/MetroReflectionBinary.inl \
    src/metro/reflection/MetroReflectionJson.inl \
    src/metro/textures/MetroTexturesDBImpl2033.inl \
    src/metro/textures/MetroTexturesDBImplArktika1.inl \
    src/metro/textures/MetroTexturesDBImplExodus.inl \
    src/metro/textures/MetroTexturesDBImplLastLight.inl \
    src/metro/textures/MetroTexturesDBImplRedux.inl \
    src/metro/vfs/MetroVFS.h \
    src/mex_settings.h \
    src/mycommon.h \
    src/mymath.h \
    src/ui/settingsdlg.h

FORMS += \
    src/ui/imageinfopanel.ui \
    src/ui/mainwindow.ui \
    src/ui/modelinfopanel.ui \
    src/ui/settingsdlg.ui

INCLUDEPATH += src \
    libs/glm \
    libs/stb \
    libs/lz4 \
    libs/quicklz \
    libs/bc7enc16 \
    libs/xxhash \
    libs/fbx/include \
    libs/pugixml/src \
    libs/jansson/src \
    libs/imgui \
    libs/imgui/examples \
    libs/tinyobjloader

DEFINES += _CRT_SECURE_NO_WARNINGS=1 \
    HAVE_CONFIG_H=1

LIBS += -L"libs/fbx/lib/x64"

CONFIG(debug, debug|release ) {
    #debug build
    LIBS += -l"$$PWD/libs/crunch/lib/VC9/Debug_DLL/x64/crnlibD_DLL_x64_VC9"
} else:CONFIG(release, debug|release) {
    #release build
    LIBS += -l"$$PWD/libs/crunch/lib/VC9/Release_DLL/x64/crnlib_DLL_x64_VC9"
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/resources.qrc
