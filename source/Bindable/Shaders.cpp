#include "Shader/Shaders.h"


Shader::Shader(std::string name, Renderer& rdr, const CD3DX12_ROOT_PARAMETER* rootParameters,
	UINT numRootParameters, const std::wstring& path)
	: Name(name), vs(VertexShader(path)), ps(PixelShader(path)), rtSig(nullptr)
{

	// root signature affair
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(numRootParameters, rootParameters, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSignature, &errorBlob));
	ThrowIfFailed(rdr.m_Device->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&rtSig)));
}

// TO-DO: class shader is the base class of our shader
// we can create different shader for shadow, sky box with 
// different ways to bind rootSignature to pipeline
void Shader::Bind(Renderer& rdr) noexcept
{
	// set the root signature
	rdr.m_CommandList->SetGraphicsRootSignature(rtSig.Get());
	rdr.m_RootSignature = rtSig;
	// Pipeline State shoule be set only once for a shader
	if(ppState == nullptr)
		CreatePipelineState(rdr);
	rdr.m_CommandList->SetPipelineState(ppState.Get());
	rdr.m_PSOs[Name] = ppState;
	rdr.m_CurrShaderAndPSO = Name;
}

ID3D12RootSignature* Shader::GetRootSignature()
{
	return rtSig.Get();
}

void Shader::CreatePipelineState(Renderer& rdr)
{
	// Define the vertex input layout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs , _countof(inputElementDescs) };
	psoDesc.pRootSignature = rtSig.Get();

	psoDesc.VS = { reinterpret_cast<UINT8*>(vs.GetByteCode()->GetBufferPointer()), vs.GetByteCode()->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(ps.GetByteCode()->GetBufferPointer()), ps.GetByteCode()->GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = rdr.m_BackBufferFormat;
	psoDesc.SampleDesc.Count = rdr.m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = rdr.m_4xMsaaState ? (rdr.m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = rdr.m_DepthStencilFormat;
	ThrowIfFailed(rdr.m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&ppState)));
}
