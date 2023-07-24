#pragma once
#include "d3dApp.h"
#include "FrameResource.h"
#include "RenderItem.h"
#include "GLTFloader.h"
#include "Texture2D.h"
#include "GeometryGenerator.h"
#include "Camera.h"

using Microsoft::WRL::ComPtr;

// classify RI property
enum class RenderLayer : int
{
    Opaque = 0,
    Transparent,
    Count
};


class DX13App :
    public D3DApp
{
public:
    DX13App(HINSTANCE hInstance);
    DX13App(const DX13App& rhs) = delete;
    DX13App& operator=(const DX13App& rhs) = delete;
    ~DX13App();

    virtual bool Initialize()override;
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)override;

private:
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;
    //mouse msg
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);

    void BuildDescriptorHeaps();
    // void BuildConstantBufferViews();
    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    // void BuildBoxGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    // void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);

    // Get Sampler
    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
    // about Frame Resource
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    // Hash
    // Restore RI submesh
    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    // List of all the render items.
    std::vector<std::unique_ptr<RenderItem>> mAllRitems;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    // Render items divided by PSO.
    std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

    // friend class RenderItem;

    // offset to the start of the pass CBVs
    UINT mPassCbvOffset = 0;

    // Root Signature
    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;


    // MainPass
    PassConstants mMainPassCB;

    // MainPass Data
    Camera mCamera;

    // Manipulate the Direct Light Pos
    float mSunTheta = 1.25f * DirectX::XM_PI;
    float mSunPhi = DirectX::XM_PIDIV4;

    // Point Light
    float x = 0.0f;
    float y = 0.0f;

    POINT mLastMousePos;
//************* Texture affair ******************
private:
    void DeliverDataFromTextureToHeap(class Texture2D& tex);
    void CreateSRVforTexture(std::vector<class Texture2D>& tex);
    void BuildMaterials();
private:
    UINT mCbvSrvDescriptorSize = 0;
    std::vector<Texture2D> mTexture2Ds;
};

