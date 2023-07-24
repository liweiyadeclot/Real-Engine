#include "Skybox.h"
#include "SkyBoxShadowMapShader.h"

Skybox::Skybox(Renderer& rdr)
	: Drawable(rdr)
{
	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::ShaderResource);
	rootTypes.push_back(Renderer::RootParaType::TextureTable);
	auto pointerToShader = std::make_unique<ModelShader>("test", rdr, L"./sky.hlsl", rootTypes, false, true);
	AddBind(std::move(pointerToShader));

	GeometryGenerator geogen;
	GeometryGenerator::MeshData skySphere = geogen.CreateSphere(1.0, 20, 20);
	size_t verticescount = skySphere.Vertices.size();
	std::vector<Vertex> vertices(verticescount);
	for (size_t i = 0; i < verticescount; ++i) {
		vertices[i].Pos = skySphere.Vertices[i].Position;
		vertices[i].Normal = skySphere.Vertices[i].Normal;
		vertices[i].TexC = skySphere.Vertices[i].TexC;
	}
	std::vector<uint16_t> indices = skySphere.GetIndices16();
	m_indexcount = indices.size();

	AddBind(std::make_unique<VertexBuffer>(rdr, vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(rdr, indices));
	AddTexture(rdr);

	m_ShadowShader = std::move(std::make_unique<SkyBoxShadowMapShader>("SkyBoxShadow", rdr,
		L"./Shadow.hlsl", rootTypes));
}

void Skybox::Draw(Renderer& rdr) const noexcept
{
	for (auto& b : binds)
	{
		b->Bind(rdr);

	}

	rdr.PrepForDrawToScreen();
	rdr.DrawSkyBox(m_indexcount, StartIndexLocation, BaseVertexLocation, ObjIndex);

}

void Skybox::RenderToShadowMap(Renderer& rdr) const noexcept
{
	// do nothing
}

void Skybox::AddTexture(Renderer& rdr)
{
	m_Tex = std::make_unique<DX14Texture>();
	const std::wstring texpath = L"./Textures/grasscube1024.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(rdr.GetDevice(),rdr.GetCommandList(), texpath.c_str(),
		m_Tex->Resource, m_Tex->UploadHeap));

	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(rdr.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&(rdr.GetIndexDescriptorHeap(ObjIndex)))));

	ID3D12DescriptorHeap* descriptorHeap[] = { rdr.GetIndexDescriptorHeap(ObjIndex).Get() };
	// rdr.GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeap), descriptorHeap);

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(rdr.GetIndexDescriptorHeap(ObjIndex)->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Format = DXGI_FORMAT_BC1_UNORM;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	rdr.GetDevice()->CreateShaderResourceView(m_Tex->GetResource(), &srvDesc, srvHandle);

	BuildMaterial();
}

void Skybox::BuildMaterial()
{
	auto tempMat = std::make_unique<Material>();

	tempMat->MatCBIndex = 15;
	tempMat->DiffuseSrvHeapIndex = 15;
	tempMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tempMat->FresnelR0 = DirectX::XMFLOAT3(0.05f, 0.05f, 0.05f);
	tempMat->Roughness = 0.1f;

	m_Mat = std::move(tempMat);
}
