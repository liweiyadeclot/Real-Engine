#include "ShadowMapShader.h"

ShadowMapShader::ShadowMapShader(std::string Name, Renderer& rdr, const std::wstring& path, 
	const std::vector<Renderer::RootParaType>& rootParas)
	: ShaderBase(Name, rdr, path, rootParas)
{

}

void ShadowMapShader::CreatePipelineState(Renderer& rdr)
{
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs , _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_RootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(m_vs.GetByteCode()->GetBufferPointer()), m_vs.GetByteCode()->GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(m_ps.GetByteCode()->GetBufferPointer()), m_ps.GetByteCode()->GetBufferSize() };

	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.DepthBias = 100000;
	psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
	psoDesc.RasterizerState.SlopeScaledDepthBias = 1.0f;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	psoDesc.NumRenderTargets = 0;		// no render targets
	psoDesc.SampleDesc.Count = rdr.m_4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = rdr.m_4xMsaaState ? (rdr.m_4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = rdr.m_DepthStencilFormat;

	ThrowIfFailed(rdr.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
}
