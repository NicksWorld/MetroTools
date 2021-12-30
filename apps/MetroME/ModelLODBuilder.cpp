#include "metro/MetroModel.h"
#include "meshoptimizer.h"

RefPtr<MetroModelBase> BuildModelLOD(RefPtr<MetroModelBase>& model, const float ratio) {
    assert(model->GetModelType() == MetroModelType::Std);

    MyArray<MetroModelGeomData> gds;
    model->CollectGeomData(gds);
    assert(gds.size() == 1);

    const MetroModelGeomData& gd = gds[0];

    const size_t indicesCount = scast<size_t>(gd.mesh->facesCount) * 3;
    const size_t targetIndecesCount = scast<size_t>(indicesCount * ratio);
    constexpr float targetError = 0.01f;
    float lodError = 0.0f;

    MyArray<uint16_t> lodIndices(indicesCount);
    const uint16_t* indices = rcast<const uint16_t*>(gd.faces);
    const float* vertices = rcast<const float*>(gd.vertices);
    const size_t lodIndicesCount = meshopt_simplify<uint16_t>(lodIndices.data(),
                                                              indices,
                                                              indicesCount,
                                                              vertices,
                                                              gd.mesh->verticesCount,
                                                              sizeof(VertexStatic),
                                                              targetIndecesCount,
                                                              targetError,
                                                              &lodError);
    lodIndices.resize(lodIndicesCount);

    MyArray<VertexStatic> lodVertices(lodIndicesCount < gd.mesh->verticesCount ? lodIndicesCount : gd.mesh->verticesCount);
    const size_t lodVerticesCount = meshopt_optimizeVertexFetch<uint16_t>(lodVertices.data(),
                                                                          lodIndices.data(),
                                                                          lodIndicesCount,
                                                                          gd.vertices,
                                                                          gd.mesh->verticesCount,
                                                                          sizeof(VertexStatic));
    lodVertices.resize(lodVerticesCount);

    RefPtr<MetroModelStd> result = MakeRefPtr<MetroModelStd>();
    result->CreateMesh(lodVerticesCount, lodIndicesCount / 3, 0, 0);
    result->CopyVerticesData(lodVertices.data());
    result->CopyFacesData(lodIndices.data());
    result->SetMaterialString(kEmptyString, 0); // force-create material strings array

    AABBox bbox;
    bbox.Reset();
    for (const VertexStatic& v : lodVertices) {
        bbox.Absorb(v.pos);
    }

    BSphere bsphere;
    bsphere.center = bbox.Center();
    bsphere.radius = Length(bbox.Extent());

    result->SetBBox(bbox);
    result->SetBSphere(bsphere);

    return result;
}
