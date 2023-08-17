#include "Renderer.h"
#include "DX14Texture.h"
#include <xstring>
#include "imgui_impl_dx12.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

bool Renderer::InitRenderer(HWND hMainWnd)
{
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif
	m_hMainWnd = hMainWnd;

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_DxgiFactory)));

	// Try to create hardware device
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Device)
	);

	// Fallback to WARP device
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_Device)
		));
	}

	ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_Fence)));

	m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_BackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_Device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

	m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	m_SrvDescriptorHeaps.resize(MAX_DESCRIPTOR_SIZE);
	m_PBRDescriptorHeaps.resize(MAX_DESCRIPTOR_SIZE);

#ifdef _DEBUG
	LogAdapters();
#endif
	CreateCommandObjects();
	CreateSwapChain();
	CreateRtvAndDsvDescriptorHeaps();

	InitShadowResource();

	// Init ImGui
	InitImGuiResource();
	ImGui_ImplDX12_Init(m_Device.Get(), 3, DXGI_FORMAT_R8G8B8A8_UNORM,
		m_SrvHeapForImgui.Get(),
		m_SrvHeapForImgui->GetCPUDescriptorHandleForHeapStart(),
		m_SrvHeapForImgui->GetGPUDescriptorHandleForHeapStart());

	return true;
}

void Renderer::UpdateRenderer(const GameTimer& gt)
{
	UpdateFrameResource();
	
	if (m_CurrFrameResource->Fence != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	m_LightRotationAngle += 0.1f * gt.DeltaTime();
	XMMATRIX R = XMMatrixRotationY(m_LightRotationAngle);
	for (int i = 0; i < 3; ++i)
	{
		XMVECTOR lightDir = XMLoadFloat3(&m_BaseLightDirections[i]);
		lightDir = XMVector3TransformNormal(lightDir, R);
		XMStoreFloat3(&m_RotatedLightDirections[i], lightDir);
	}

	UpdateMainPassCB(gt);
	//UpdateShadowTransform(gt);
	//UpdateShadowPassCB(gt);
}

void Renderer::SetSceneCenter(float x, float y, float z)
{
	m_SceneBounds.Center = DirectX::XMFLOAT3(x, y, z);
}

void Renderer::SetSceneRadius(float r)
{
	m_SceneBounds.Radius = r;
}



void Renderer::PrepForDrawToScreen()
{
	// Specify the buffers we are going to render to
	m_CommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	auto passCB = m_CurrFrameResource->PassCB->Resource();
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCB->GetGPUVirtualAddress());

}

void Renderer::PrepForDrawToShadowMap()
{
	

	// TO-DO: Reset DSVforShadow before each frame draw
	m_CommandList->OMSetRenderTargets(0, nullptr, false, &m_NewShadowMap->Dsv());

	UINT passCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));
	auto passCB = m_CurrFrameResource->PassCB->Resource();
	// affine
	D3D12_GPU_VIRTUAL_ADDRESS passCBAddress = passCB->GetGPUVirtualAddress() + 1 * passCBByteSize;
	m_CommandList->SetGraphicsRootConstantBufferView(1, passCBAddress);
}

void Renderer::DrawIndexedToShadowMap(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex)
{
	DrawIndexed(count, StartIndexLocation, BaseVertexLocation, ObjIndex);
	
}

void Renderer::DrawIndexed(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex)
{
	// TO-DO: Topology needs to be binded
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto objectCB = m_CurrFrameResource->ObjectCB->Resource();
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjIndex * objCBByteSize;


	auto matCB = m_CurrFrameResource->MaterialCB->Resource();
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ObjIndex * matCBByteSize;

	
	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle;
	if (m_SrvDescriptorHeaps[ObjIndex] != nullptr)
	{
		texHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_SrvDescriptorHeaps[ObjIndex]->GetGPUDescriptorHandleForHeapStart());
	}
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeaps[ObjIndex].Get() };

	CD3DX12_GPU_DESCRIPTOR_HANDLE pbrHandle;
	if (m_PBRDescriptorHeaps[ObjIndex] != nullptr)
	{
		// model without PBR
		pbrHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_PBRDescriptorHeaps[ObjIndex]->GetGPUDescriptorHandleForHeapStart());
	}
	ID3D12DescriptorHeap* descriptorHeaps_pbr[] = { m_PBRDescriptorHeaps[ObjIndex].Get() };

	CD3DX12_GPU_DESCRIPTOR_HANDLE shadowMapHandle = m_NewShadowMap->Srv();
	ID3D12DescriptorHeap* descriptorHeaps_shadow[] = { m_SrvHeapForShadow.Get() };
	

	

	for (size_t i = 0; i < m_RootParaType.size(); ++i)
	{
		switch (m_RootParaType[i])
		{
		case RootParaType::MaterialConstBuffer:
			m_CommandList->SetGraphicsRootConstantBufferView((UINT)i, matCBAddress);
			break;
		case RootParaType::MaterialTable:
			m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps_pbr), descriptorHeaps_pbr);
			m_CommandList->SetGraphicsRootDescriptorTable((UINT)i, pbrHandle);
			break;
		case RootParaType::ObjectConstBuffer:
			m_CommandList->SetGraphicsRootConstantBufferView((UINT)i, objCBAddress);
			break;
		case RootParaType::TextureTable:
			m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			m_CommandList->SetGraphicsRootDescriptorTable((UINT)i, texHandle);
			break;
		case RootParaType::ShadowTable:
			m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps_shadow), descriptorHeaps_shadow);
			m_CommandList->SetGraphicsRootDescriptorTable((UINT)i, shadowMapHandle);
			break;
		}
	}
	
	

	m_CommandList->DrawIndexedInstanced(count, 1, StartIndexLocation, BaseVertexLocation, 0);

	//// Renderer Imgui
	//ID3D12DescriptorHeap* descriptorHeaps_Imgui[] = { m_SrvHeapForImgui.Get() };
	//m_CommandList->SetDescriptorHeaps(1, descriptorHeaps_Imgui);
	//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList.Get());
}

void Renderer::DrawSkyBox(UINT count, UINT StartIndexLocation, UINT BaseVertexLocation, UINT ObjIndex)
{
	m_CommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto objectCB = m_CurrFrameResource->ObjectCB->Resource();
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ObjIndex * objCBByteSize;

	auto passCB = m_CurrFrameResource->PassCB->Resource();

	auto matCB = m_CurrFrameResource->MaterialCB->Resource();
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
	D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ObjIndex * matCBByteSize;


	CD3DX12_GPU_DESCRIPTOR_HANDLE texHandle(m_SrvDescriptorHeaps[ObjIndex]->GetGPUDescriptorHandleForHeapStart());
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_SrvDescriptorHeaps[ObjIndex].Get() };

	for (size_t i = 0; i < m_RootParaType.size(); ++i)
	{
		switch (m_RootParaType[i])
		{
		case RootParaType::MainPassConstBuffer:
			m_CommandList->SetGraphicsRootConstantBufferView(i, passCB->GetGPUVirtualAddress());
			break;
		case RootParaType::MaterialConstBuffer:
			m_CommandList->SetGraphicsRootConstantBufferView(i, matCBAddress);
			break;
		case RootParaType::ObjectConstBuffer:
			m_CommandList->SetGraphicsRootConstantBufferView(i, objCBAddress);
			break;
		case RootParaType::TextureTable:
			m_CommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
			m_CommandList->SetGraphicsRootDescriptorTable(i, texHandle);
			break;
		default:
			break;
		}
	}


	m_CommandList->DrawIndexedInstanced(count, 1, StartIndexLocation, BaseVertexLocation, 0);
}

void Renderer::OnResize()
{
	mCamera.SetLens(0.25f * MathHelper::Pi,(float)m_ClientWidth / (float)m_ClientHeight, 1.0f, 1000.0f);
}

void Renderer::PresentOneFrame()
{
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	ThrowIfFailed(m_CommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(m_SwapChain->Present(0, 0));
	m_CurrBackBuffer = (m_CurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	m_CurrFrameResource->Fence = ++m_CurrentFence;

	// Notify the fence when the GPU completes commands up to this fence point.
	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));
}

void Renderer::ResetRenderer()
{
	// ThrowIfFailed(m_CommandAllocator->Reset());
	auto cmdListAlloc = m_CurrFrameResource->CmdListAlloc;

	ThrowIfFailed(cmdListAlloc->Reset());
	ThrowIfFailed(m_CommandList->Reset(cmdListAlloc.Get(), nullptr));

}

void Renderer::ExecuteCommands()
{
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	FlushCommandQueue();
}

void Renderer::DestructorInApp()
{
	if (m_Device != nullptr)
	{
		FlushCommandQueue();
	}
}


std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> Renderer::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow
	};
}

void Renderer::SetIndexBuffer(D3D12_INDEX_BUFFER_VIEW idbv)
{
	m_IndexBufferView = idbv;
}

void Renderer::SetVertexBuffer(D3D12_VERTEX_BUFFER_VIEW vtbv)
{
	m_VertexBufferView = vtbv;
}

FrameResource* Renderer::GetpFrameResource()
{
	return m_CurrFrameResource;
}

void Renderer::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));


	ThrowIfFailed(m_Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_CommandAllocator.GetAddressOf())
	));

	ThrowIfFailed(m_Device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_CommandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(m_CommandList.GetAddressOf())
	));

	m_CommandList->Close();
}



void Renderer::RenderToShadowMap()
{
	// render to shadowmap is the job of Drawable::Draw()
}

void Renderer::UpdateShadowTransform(const GameTimer& gt)
{
	using namespace DirectX;
	// Only the first "main" light casts a shadow.
	// XMVECTOR lightDir = XMLoadFloat3(&m_RotatedLightDirections[0]);
	// XMVECTOR lightPos = -0.1f * m_SceneBounds.Radius * lightDir;
	XMVECTOR lightPos = XMLoadFloat3(&m_TestImGuiLightPos);
	XMVECTOR targetPos = XMLoadFloat3(&m_SceneBounds.Center);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&m_LightPosW, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - m_SceneBounds.Radius;
	float b = sphereCenterLS.y - m_SceneBounds.Radius;
	float n = sphereCenterLS.z - m_SceneBounds.Radius;
	float r = sphereCenterLS.x + m_SceneBounds.Radius;
	float t = sphereCenterLS.y + m_SceneBounds.Radius;
	float f = sphereCenterLS.z + m_SceneBounds.Radius;

	m_LightNearZ = n;
	m_LightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;
	XMStoreFloat4x4(&m_LightView, lightView);
	XMStoreFloat4x4(&m_LightProj, lightProj);
	XMStoreFloat4x4(&m_ShadowTransform, S);
}

void Renderer::UpdateShadowPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&m_LightView);
	XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = m_NewShadowMap->Width();
	UINT h = m_NewShadowMap->Height();

	XMStoreFloat4x4(&m_ShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	m_ShadowPassCB.EyePosW = m_LightPosW;
	m_ShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	m_ShadowPassCB.NearZ = m_LightNearZ;
	m_ShadowPassCB.FarZ = m_LightFarZ;

	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(1, m_ShadowPassCB);
}

void Renderer::InitShadowResource()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc,
		IID_PPV_ARGS(m_SrvHeapForShadow.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeapForShadow.GetAddressOf())));


	m_NewShadowMap = std::make_unique<ShadowMap>(m_Device.Get(), 2048, 2048);
	auto srvForShadowCPUStart = m_SrvHeapForShadow->GetCPUDescriptorHandleForHeapStart();
	auto srvForShadowGPUStart = m_SrvHeapForShadow->GetGPUDescriptorHandleForHeapStart();
	auto dsvForShadowCPUStart = m_DsvHeapForShadow->GetCPUDescriptorHandleForHeapStart();
	m_NewShadowMap->BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE(srvForShadowCPUStart),
		CD3DX12_GPU_DESCRIPTOR_HANDLE(srvForShadowGPUStart),
		CD3DX12_CPU_DESCRIPTOR_HANDLE(dsvForShadowCPUStart)
	);
}

void Renderer::InitImGuiResource()
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDesc,
		IID_PPV_ARGS(m_SrvHeapForImgui.GetAddressOf())));
}

void Renderer::SpawnLightControlWindow()
{
	if (ImGui::Begin("Light"))
	{
		ImGui::Text("Color");
		ImGui::ColorEdit3("Diffuse Color", &m_TestImGuiLightColor.x);

		ImGui::Text("Position");
		ImGui::SliderFloat("X", &m_TestImGuiLightPos.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &m_TestImGuiLightPos.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &m_TestImGuiLightPos.z, -60.0f, 60.0f, "%.1f");
	}
	ImGui::End();
}

void Renderer::DrawImGui()
{
	// Renderer Imgui
	ID3D12DescriptorHeap* descriptorHeaps_Imgui[] = { m_SrvHeapForImgui.Get() };
	m_CommandList->SetDescriptorHeaps(1, descriptorHeaps_Imgui);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_CommandList.Get());
}

void Renderer::SetRenderToDrawToScreen()
{
	m_CommandList->RSSetViewports(1, &m_ScreenViewport);
	m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

	// Indicate a state transition on the resource usage
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_NewShadowMap->Resource(),
		D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ));
	// Clear the back buffer and depth buffer
	m_CommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::White, 0, nullptr);
	m_CommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr);
}

void Renderer::SetRenderToDrawToShadowMap()
{
	m_CommandList->RSSetViewports(1, &m_NewShadowMap->Viewport());
	m_CommandList->RSSetScissorRects(1, &m_NewShadowMap->ScissorRect());

	m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		m_NewShadowMap->Resource(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_DEPTH_WRITE));

	m_CommandList->ClearDepthStencilView(m_NewShadowMap->Dsv(),
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
}

void Renderer::CreateSwapChain()
{
	// Release the previous swapchain we will be recreateing.
	m_SwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_ClientWidth;
	sd.BufferDesc.Height = m_ClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = m_BackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
	sd.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = m_hMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(m_DxgiFactory->CreateSwapChain(
		m_CommandQueue.Get(),
		&sd,
		m_SwapChain.GetAddressOf()));
}


void Renderer::FlushCommandQueue()
{
	m_CurrentFence++;

	ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), m_CurrentFence));

	if (m_Fence->GetCompletedValue() < m_CurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrentFence, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

ID3D12Resource* Renderer::CurrentBackBuffer() const
{
	return m_SwapChainBuffer[m_CurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_RtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_CurrBackBuffer,
		m_RtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
	return m_DsvHeap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wstring fpsStr = std::to_wstring(fps);
		std::wstring mspfStr = std::to_wstring(mspf);

		std::wstring windowText = m_MainWndCaption +
			L"    fps: " + fpsStr +
			L"   mspf: " + mspfStr;

		SetWindowText(m_hMainWnd, windowText.c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void Renderer::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(m_RtvHeap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_Device->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS(m_DsvHeap.GetAddressOf())));
}

void Renderer::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_DxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		LogAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void Renderer::LogAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_BackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void Renderer::LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

void Renderer::UpdateFrameResource()
{
	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % gNumFrameResources;
	m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

	
}

void Renderer::UpdateMainPassCB(const GameTimer& gt)
{
	using namespace DirectX;
	XMMATRIX view = mCamera.GetView();
	XMMATRIX proj = mCamera.GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);
	XMMATRIX shadowTransform = XMLoadFloat4x4(&m_ShadowTransform);

	XMStoreFloat4x4(&m_MainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&m_MainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&m_MainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&m_MainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&m_MainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_MainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	XMStoreFloat4x4(&m_MainPassCB.ShadowTransform, XMMatrixTranspose(shadowTransform));

	m_MainPassCB.EyePosW = mCamera.GetPosition3f();
	m_MainPassCB.RenderTargetSize = XMFLOAT2((float)m_ClientWidth, (float)m_ClientHeight);
	m_MainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / m_ClientWidth, 1.0f / m_ClientHeight);
	m_MainPassCB.NearZ = 1.0f;
	m_MainPassCB.FarZ = 1000.0f;
	m_MainPassCB.TotalTime = gt.TotalTime();
	m_MainPassCB.DeltaTime = gt.DeltaTime();

	m_MainPassCB.AmbientLight = { 0.2f, 0.2f, 0.2f, 1.0f };

	m_MainPassCB.Lights[0].Direction = Lights[0].Direction;
	m_MainPassCB.Lights[0].Strength = Lights[0].Strength;

	/*m_MainPassCB.Lights[1].Direction = m_RotatedLightDirections[1];
	m_MainPassCB.Lights[1].Strength = { 0.4f, 0.4f, 0.4f };
	m_MainPassCB.Lights[2].Direction = m_RotatedLightDirections[2];
	m_MainPassCB.Lights[2].Strength = { 0.2f, 0.2f, 0.2f };*/


	auto currPassCB = m_CurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, m_MainPassCB);
}


void Renderer::BuildFrameResource(UINT ObjCount)
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		m_FrameResources.push_back(std::make_unique<FrameResource>(m_Device.Get(),
			2, ObjCount, ObjCount));

	}
}





