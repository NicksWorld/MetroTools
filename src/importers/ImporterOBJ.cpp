#include "ImporterOBJ.h"

#include "metro/MetroModel.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


struct VerticesCollector {
    MyArray<VertexStatic>   vertices;
    MyArray<uint16_t>       indices;

    void AddVertex(const VertexStatic& v) {
        const auto begin = this->vertices.begin();
        const auto end = this->vertices.end();
        const auto it = std::find_if(begin, end, [&v](const VertexStatic& vertex)->bool {
            return (v.pos == vertex.pos && v.normal == vertex.normal && v.aux0 == vertex.aux0 && v.aux1 == vertex.aux1 && v.uv == vertex.uv);
        });

        if (it == end) {
            indices.push_back(scast<uint16_t>(this->vertices.size()));
            this->vertices.push_back(v);
        } else {
            const size_t idx = std::distance(begin, it);
            this->indices.push_back(scast<uint16_t>(idx));
        }
    }
};



ImporterOBJ::ImporterOBJ() {
}
ImporterOBJ::~ImporterOBJ() {
}

RefPtr<MetroModelBase> ImporterOBJ::ImportModel(const fs::path& path) {
    RefPtr<MetroModelBase> result;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, error;

    const bool loaded = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, path.u8string().data());
    if (loaded) {
        RefPtr<MetroModelHierarchy> newModel = MakeRefPtr<MetroModelHierarchy>();
        newModel->SetModelVersion(22); //! Redux

        for (const tinyobj::shape_t& shape : shapes) {
            RefPtr<MetroModelBase> child = this->CreateStdModel(shape, attrib);
            if (child) {
                newModel->AddChild(child);
            }
        }

        result = newModel;
    }

    return result;
}

RefPtr<MetroModelStd> ImporterOBJ::CreateStdModel(const tinyobj::shape_t& shape, const tinyobj::attrib_t& attrib) {
    VerticesCollector collector;

    const size_t numObjFaces = shape.mesh.num_face_vertices.size();

    AABBox bbox;
    bbox.Reset();

    size_t vIdx = 0;
    for (size_t f = 0; f < numObjFaces; ++f) {
        assert(shape.mesh.num_face_vertices[f] == 3);
        for (size_t j = 0; j < 3; ++j) {
            //#NOTE_SK: invert vertices order
            const size_t invJ = 2 - j;
            const tinyobj::index_t& i = shape.mesh.indices[vIdx + invJ];

            vec3 pos;
            vec3 normal;
            vec2 uv;

            pos.x = attrib.vertices[3 * i.vertex_index + 0];
            pos.y = attrib.vertices[3 * i.vertex_index + 1];
            pos.z = attrib.vertices[3 * i.vertex_index + 2];
            normal.x = attrib.normals[3 * i.normal_index + 0];
            normal.y = attrib.normals[3 * i.normal_index + 1];
            normal.z = attrib.normals[3 * i.normal_index + 2];
            uv.x = attrib.texcoords[2 * i.texcoord_index + 0];
            uv.y = attrib.texcoords[2 * i.texcoord_index + 1];

            VertexStatic vertex = {};
            vertex.pos = MetroSwizzle(pos);
            vertex.normal = EncodeNormal(MetroSwizzle(normal), 1.0f);
            vertex.uv = uv;

            collector.AddVertex(vertex);

            bbox.Absorb(pos);
        }

        vIdx += 3;
    }

    const size_t numVertices = collector.vertices.size();
    const size_t numFaces = collector.indices.size() / 3;

    RefPtr<MetroModelStd> result = MakeRefPtr<MetroModelStd>();
    result->CreateMesh(numVertices, numFaces);
    result->CopyVerticesData(collector.vertices.data());
    result->CopyFacesData(collector.indices.data());
    result->SetMaterialString(kEmptyString, 0); // force-create material strings array

    BSphere bsphere;
    bsphere.center = bbox.Center();
    bsphere.radius = Length(bbox.Extent());

    result->SetBBox(bbox);
    result->SetBSphere(bsphere);

    result->SetModelVersion(22); //! Redux

    return result;
}
