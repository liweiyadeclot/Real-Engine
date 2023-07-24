#include "ModelShader.h"

ModelShader::ModelShader(std::string name, Renderer& rdr, const std::wstring& path, 
	const std::vector<Renderer::RootParaType>& rootParas, bool alphamode, bool skymode)
	: ShaderBase(name, rdr, path, rootParas), m_Alphamode(alphamode), m_Skymode(skymode)
{

}

void ModelShader::CreatePipelineState(Renderer& rdr)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	if (m_Skymode) {
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs , _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(m_vs.GetByteCode()->GetBufferPointer()), m_vs.GetByteCode()->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(m_ps.GetByteCode()->GetBufferPointer()), m_ps.GetByteCode()->GetBufferSize() };
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

	if (m_Alphamode)
	{
		D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
		transparencyBlendDesc.BlendEnable = true;
		transparencyBlendDesc.LogicOpEnable = false;
		transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		psoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	}
	if (m_Skymode) {
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	}

	ThrowIfFailed(rdr.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
}
