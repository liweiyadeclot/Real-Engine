#pragma once
#include "d3dUtil.h"
#include "GameTimer.h" 
#include "FrameResource.h"
#include "Camera.h"
#include "ShadowMap.h"
#include <vector>


class Renderer
{
	friend class Bindable;
	friend class Shader;
	friend class D4DApp;
	friend class ShaderBase;
	friend class ModelShader;
	friend class SkyShader;
	friend class ShadowMapShader;
	friend class SkyBoxShadowMapShader;
	friend class LightBase;
	friend class MinimalShader;
public:
	enum class RootParaType
	{
		ObjectConstBuffer,
		MainPassConstBuffer,
		MaterialConstBuffer,
		TextureTable,
		MaterialTable,
		ShaderResource,	// for sky box only
		ShadowTable
	};


	Renderer() = default;
	// Renderer(HWND hWnd);
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	~Renderer() = default;

	bool InitRenderer(HWND hMainWnd);
	void UpdateRenderer(const GameTimer& gt);
	void SetSceneCenter(float x, float y, float z);
	void SetSceneRadius(float r);
	void PrepForDrawToScreen();
	void PrepForDrawToShadowMap();
	void DrawIndexedToShadowMap(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex);
	void DrawIndexed(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex);
	void DrawSkyBox(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex);
	void OnResize();
	void PresentOneFrame();
	void ResetRenderer();
	void ExecuteCommands();
	void DestructorInApp();
	void SetRenderToDrawToScreen();
	void SetRenderToDrawToShadowMap();

	// ******************* Test for ImGui ******************
	void DrawImGui();
	void SpawnLightControlWindow();
	//  ******************* ******************* *******************
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers();
	ID3D12Device* GetDevice()
	{
		return m_Device.Get();
	}

	ID3D12GraphicsCommandList* GetCommandList()
	{
		return m_CommandList.Get();
	}

	FrameResource* GetCurrFrameResource()
	{
		return m_CurrFrameResource;
	}


	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetIndexDescriptorHeap(int i)
	{
		return m_SrvDescriptorHeaps[i];
	}

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& GetIndexpbrDescriptorHeap(int i)
	{
		return m_PBRDescriptorHeaps[i];
	}

	void SetRendererState()
	{
		BuildFrameResource(ObjectNum);
	}

	void SetRootSigTypes(const std::vector<RootParaType>& rst)
	{
		m_RootParaType = rst;
	}

	void SetShadowTransform(const DirectX::XMFLOAT4X4& tsf)
	{
		m_ShadowTransform = tsf;
	}

	void SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW idbv);
	void SetVertexBuffer(D3D12_VERTEX_BUFFER_VIEW vtbv);
	FrameResource* GetpFrameResource();
	Camera mCamera;

	int ObjectNum = 0;
	int TexNum = 0;

	// TO-DO: Build Light PassConstantBuffer
	Light Lights[16];
private:
	HWND      m_hMainWnd = nullptr; // main window handle

	// std::vector<class Drawable> m_drawables;

	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RenderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTarget;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DepthStencilViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature = nullptr;

	GameTimer m_Timer;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_PSOs;

	Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
	UINT64 m_CurrentFence = 0;

	std::string m_CurrShaderAndPSO;

	std::wstring m_MainWndCaption = L"Real Engine";

	UINT64 m_FenceValue;
	HANDLE m_FenceEvent;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvDescriptorHeap = nullptr;
	const int MAX_DESCRIPTOR_SIZE = 32;
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_SrvDescriptorHeaps;
	std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> m_PBRDescriptorHeaps;
	UINT m_RtvDescriptorSize = 0;
	UINT m_DsvDescriptorSize = 0;
	UINT m_CbvSrvUavDescriptorSize = 0;

	// about Frame Resource
	std::vector<std::unique_ptr<FrameResource>> m_FrameResources;
	FrameResource* m_CurrFrameResource = nullptr;
	int m_CurrFrameResourceIndex = 0;

	static const int SwapChainBufferCount = 2;
	int m_CurrBackBuffer = 0;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_SwapChainBuffer[SwapChainBufferCount];

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;

	D3D12_VIEWPORT m_ScreenViewport;
	D3D12_RECT m_ScissorRect;

	bool m_4xMsaaState = false;
	UINT m_4xMsaaQuality = 0;

	D3D_DRIVER_TYPE m_d3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	DXGI_FORMAT m_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_DepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	int m_ClientWidth = 800;
	int m_ClientHeight = 600;

	// IndexBuffer
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	// VertexBuffer
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	// MainPass
	PassConstants m_MainPassCB;
	PassConstants m_ShadowPassCB;// index 1 of pass cbuffer.

	// RootSignatureTypes
	std::vector<RootParaType> m_RootParaType;

	float m_Theta = 1.5f * DirectX::XM_PI;
	float m_Phi = 0.2f * DirectX::XM_PI;
	float m_Radius = 15.0f;

	// *************** shadow affair **************
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ShadowMap;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeapForShadow;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeapForShadow;

	Camera m_CameraForShadow;
	std::unique_ptr<ShadowMap> m_NewShadowMap;
	// *************** End of Shadow affairs ***************


	// *************** Imgui ***************
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SrvHeapForImgui;
	// *************** End of Imgui ***************

	// TO-DO: Light System need to be completed, these rotated light is just for testing shadow.
	// TO-DO: Light System should be removed from class Renderer
	DirectX::BoundingSphere m_SceneBounds;
	float m_LightNearZ = 0.0f;
	float m_LightFarZ = 0.0f;

	DirectX::XMFLOAT3 m_LightPosW;
	DirectX::XMFLOAT4X4 m_LightView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_LightProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_ShadowTransform = MathHelper::Identity4x4();

	float m_LightRotationAngle = 0.0f;
	DirectX::XMFLOAT3 m_BaseLightDirections[3] = {
		DirectX::XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		DirectX::XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		DirectX::XMFLOAT3(0.0f, -0.707f, -0.707f)
	};
	DirectX::XMFLOAT3 m_RotatedLightDirections[3];	
	DirectX::XMFLOAT3 m_TestImGuiLightColor = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_TestImGuiLightPos = { 1.0f, 1.0f, 1.0f };



private:
	void CreateCommandObjects();
	void CreateSwapChain();
	void FlushCommandQueue();
	ID3D12Resource* CurrentBackBuffer() const;
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	void CalculateFrameStats();

	void CreateRtvAndDsvDescriptorHeaps();

	void LogAdapters();
	void LogAdapterOutputs(IDXGIAdapter* adapter);
	void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	
	void UpdateFrameResource();
	void UpdateMainPassCB(const GameTimer& gt);
	void BuildFrameResource(UINT ObjCount);

	// Shadow affairs
	void RenderToShadowMap();

	void UpdateShadowTransform(const GameTimer& gt);
	void UpdateShadowPassCB(const GameTimer& gt);

	void InitShadowResource();

	// ImGui affairs
	void InitImGuiResource();
	// TO-DO: Remove this function to LightSubSystem


};