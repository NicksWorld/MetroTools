#include "ExporterOBJ.h"
#include "metro/MetroModel.h"
#include "metro/MetroContext.h"

#include <fstream>
#include <sstream>

ExporterOBJ::ExporterOBJ()
    : mExcludeCollision(false)
{
}
ExporterOBJ::~ExporterOBJ() {
}


void ExporterOBJ::SetExcludeCollision(const bool b) {
    mExcludeCollision = b;
}

void ExporterOBJ::SetExporterName(const CharString& name) {
    mExporterName = name;
}

void ExporterOBJ::SetTexturesExtension(const CharString& ext) {
    mTexturesExtension = ext;
}


static MyArray<MetroVertex> MakeCommonVertices(const MetroModelGeomData& gd) {
    MyArray<MetroVertex> result;

    if (gd.mesh->vertexType == MetroVertexType::Skin) {
        const VertexSkinned* srcVerts = rcast<const VertexSkinned*>(gd.vertices);

        result.resize(gd.mesh->verticesCount);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            dstVerts->pos *= gd.mesh->verticesScale;
            ++srcVerts;
            ++dstVerts;
        }
    } else {
        const VertexStatic* srcVerts = rcast<const VertexStatic*>(gd.vertices);

        result.resize(gd.mesh->verticesCount);
        MetroVertex* dstVerts = result.data();

        for (size_t i = 0; i < gd.mesh->verticesCount; ++i) {
            *dstVerts = ConvertVertex(*srcVerts);
            dstVerts->pos *= gd.mesh->verticesScale;
            ++srcVerts;
            ++dstVerts;
        }
    }

    return std::move(result);
}

bool ExporterOBJ::ExportModel(const MetroModelBase& model, const fs::path& filePath) const {
    bool result = false;

    std::ofstream file(filePath, std::ofstream::binary);
    if (file.good()) {
        CharString matName = filePath.filename().string();
        matName[matName.size() - 3] = 'm';
        matName[matName.size() - 2] = 't';
        matName[matName.size() - 1] = 'l';

        std::ostringstream stringBuilder;
        stringBuilder << "# Generated from Metro model file" << std::endl;
        stringBuilder << CharString("# using ").append(mExporterName).append(" tool made by iOrange") << std::endl << std::endl;
        stringBuilder << "mtllib " << matName << std::endl << std::endl;

        MyArray<MetroModelGeomData> gds;
        model.CollectGeomData(gds);
        size_t lastIdx = 0, meshIdx = 0;
        for (const auto& gd : gds) {
            MyArray<MetroVertex> vertices = MakeCommonVertices(gd);

            for (const MetroVertex& v : vertices) {
                stringBuilder << "v " << v.pos.x << ' ' << v.pos.y << ' ' << v.pos.z << std::endl;
            }
            stringBuilder << "# " << vertices.size() << " vertices" << std::endl << std::endl;

            for (const MetroVertex& v : vertices) {
                stringBuilder << "vt " << v.uv0.x << ' ' << (1.0f - v.uv0.y) << std::endl;
            }
            stringBuilder << "# " << vertices.size() << " texcoords" << std::endl << std::endl;

            for (const MetroVertex& v : vertices) {
                stringBuilder << "vn " << v.normal.x << ' ' << v.normal.y << ' ' << v.normal.z << std::endl;
            }
            stringBuilder << "# " << vertices.size() << " normals" << std::endl << std::endl;

            stringBuilder << "g Mesh_" << meshIdx << std::endl;
            stringBuilder << "usemtl " << "Material_" << meshIdx << std::endl;

            const MetroFace* faces = scast<const MetroFace*>(gd.faces);
            for (size_t i = 0; i < gd.mesh->facesCount; ++i) {
                const size_t a = faces[i].a + lastIdx + 1;
                const size_t b = faces[i].b + lastIdx + 1;
                const size_t c = faces[i].c + lastIdx + 1;

                stringBuilder << "f " << a << '/' << a << '/' << a <<
                                  ' ' << b << '/' << b << '/' << b <<
                                  ' ' << c << '/' << c << '/' << c << std::endl;
            }
            stringBuilder << "# " << gd.mesh->facesCount << " faces" << std::endl << std::endl;

            ++meshIdx;
            lastIdx += vertices.size();
        }

        const CharString& str = stringBuilder.str();
        file.write(str.c_str(), str.length());
        file.flush();

        CharString matPath = filePath.string();
        matPath[matPath.size() - 3] = 'm';
        matPath[matPath.size() - 2] = 't';
        matPath[matPath.size() - 1] = 'l';

        std::ofstream mtlFile(matPath, std::ofstream::binary);
        if (mtlFile.good()) {
            std::ostringstream mtlBuilder;
            mtlBuilder << "# Generated from Metro model file" << std::endl;
            mtlBuilder << CharString("# using ").append(mExporterName).append(" tool made by iOrange") << std::endl << std::endl;

            meshIdx = 0;
            for (const auto& gd : gds) {
                mtlBuilder << "newmtl " << "Material_" << meshIdx << std::endl;

                const StringView& textureName = gd.model->GetMaterialString(MetroModelBase::kMaterialStringTexture);

                MetroSurfaceDescription surfaceSet = MetroContext::Get().GetTexturesDB().GetSurfaceSetFromName(textureName, false);
                const CharString& albedoName = surfaceSet.albedo;
                const CharString& normalmapName = surfaceSet.normalmap;

                CharString textureFileName = fs::path(albedoName).filename().string() + mTexturesExtension;

                mtlBuilder << "Kd 1 1 1" << std::endl;
                mtlBuilder << "Ke 0 0 0" << std::endl;
                mtlBuilder << "Ns 1000" << std::endl;
                mtlBuilder << "illum 2" << std::endl;
                mtlBuilder << "map_Ka " << textureFileName << std::endl;
                mtlBuilder << "map_Kd " << textureFileName << std::endl;

                if (!normalmapName.empty()) {
                    CharString normalmapFileName = fs::path(normalmapName).filename().string() + mTexturesExtension;
                    mtlBuilder << "bump " << normalmapFileName << std::endl;
                    mtlBuilder << "map_bump " << normalmapFileName << std::endl;
                }

                mtlBuilder << std::endl;
                ++meshIdx;
            }

            const CharString& mtlStr = mtlBuilder.str();
            mtlFile.write(mtlStr.c_str(), mtlStr.length());
            mtlFile.flush();
        }

        result = true;
    }

    return result;
}

