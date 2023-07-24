#pragma once
#include "MathHelper.h"
#include "FrameResource.h"
#include "GLTFloader.h"

class RenderItem
{
public:
    RenderItem() = default;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc;

    // Name
    std::string RIName;

    // Material
    Material* Mat;

    // World Matrix
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    // when modify object data, we should set NumFrameDirty = gNumeFrameResources so that we could update each frame resource.
    int NumFramesDirty = gNumFrameResources;

    // Index into GPU constant buffer(ObjectCB for each RI).
    UINT ObjCBIndex = -1;

    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;

    void BuildRIGeometry(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, 
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList, 
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries,  
        const nodeInfo& info);

    void BuildMaterial(UINT ObjCBIndex, std::unordered_map<std::string, std::unique_ptr<Material>>& mMaterials);
    
    void BuildRenderItem(UINT ObjCBIndex, std::vector<RenderItem*> * mRitemLayer, 
        std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries,
        std::vector<std::unique_ptr<RenderItem>>& mAllRitems,
        bool alphamode);

};