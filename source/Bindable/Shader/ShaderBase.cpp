#include "ShaderBase.h"

ShaderBase::ShaderBase(std::string name, Renderer& rdr, const std::wstring& path, const std::vector<Renderer::RootParaType>& rootParas)
	: m_Name(name), m_RootParas(rootParas), m_vs(path), m_ps(path)
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	CD3DX12_DESCRIPTOR_RANGE matTable;
	CD3DX12_DESCRIPTOR_RANGE shadowTable;
	int tableNum = 0;
	std::vector<CD3DX12_ROOT_PARAMETER> slotRootParameter(m_RootParas.size());
	for (size_t i = 0; i < m_RootParas.size(); ++i)
	{
		switch (m_RootParas[i])
		{
		case Renderer::RootParaType::MainPassConstBuffer:
		case Renderer::RootParaType::ObjectConstBuffer:
		case Renderer::RootParaType::MaterialConstBuffer:
			slotRootParameter[i].InitAsConstantBufferView(int(i));
			break;
		case Renderer::RootParaType::ShaderResource:
			slotRootParameter[i].InitAsShaderResourceView(0, 1);
			break;
		case Renderer::RootParaType::TextureTable:
			// Texture affair
			texTable.Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				1,
				tableNum
			);
			tableNum++;
			slotRootParameter[i].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
			break;
		case Renderer::RootParaType::MaterialTable:
			matTable.Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				1,
				tableNum);
			tableNum++;
			slotRootParameter[i].InitAsDescriptorTable(1, &matTable, D3D12_SHADER_VISIBILITY_PIXEL);
			break;
		case Renderer::RootParaType::ShadowTable:
			shadowTable.Init(
				D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				1,
				tableNum);
			tableNum++;
			slotRootParameter[i].InitAsDescriptorTable(1, &shadowTable, D3D12_SHADER_VISIBILITY_PIXEL);
		default:
			break;
		}
	}

	auto staticSamplers = rdr.GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(slotRootParameter.size(), &(slotRootParameter[0]),
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		&serializedRootSignature, &errorBlob));

	ThrowIfFailed(rdr.GetDevice()->CreateRootSignature(0, serializedRootSignature->GetBufferPointer(),
		serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
}


void ShaderBase::Bind(Renderer& rdr) noexcept
{
	rdr.m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	rdr.m_RootSignature = m_RootSignature;
	// Pipeline State should be set only once for a shader
	if (m_PipelineState == nullptr)
	{
		CreatePipelineState(rdr);
	}
	rdr.m_CommandList->SetPipelineState(m_PipelineState.Get());
	rdr.SetRootSigTypes(m_RootParas);
}

void ShaderBase::CreatePipelineState(Renderer& rdr)
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

	ThrowIfFailed(rdr.GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_PipelineState)));
}


