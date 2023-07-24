#include "Drawable.h"

int Drawable::ObjectsCount = 0;

Drawable::Drawable(Renderer& rdr)
{
	ObjIndex = ObjectsCount; 
	ObjectsCount++;
	rdr.ObjectNum = ObjectsCount;

	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);

	m_ShadowShader = std::move(std::make_unique<ShadowMapShader>("PointLightShadow", rdr,
		L"./Shadow.hlsl", rootTypes));
}

void Drawable::RenderToShadowMap(Renderer& rdr) const noexcept
{
	for (auto& b : binds)
	{
		b->Bind(rdr);
	}
	m_ShadowShader->Bind(rdr);

	rdr.PrepForDrawToShadowMap();
	rdr.DrawIndexedToShadowMap(pIndexBuffer->GetCount(), StartIndexLocation, BaseVertexLocation, ObjIndex);
}

void Drawable::Draw(Renderer& rdr) const noexcept
{

	for (auto& b : binds)
	{
		b->Bind(rdr);

	}

	rdr.PrepForDrawToScreen();
	rdr.DrawIndexed(pIndexBuffer->GetCount(), StartIndexLocation, BaseVertexLocation, ObjIndex);
}

void Drawable::Update(float dt, Renderer& rdr) noexcept
{
	auto currObjectCB = rdr.GetCurrFrameResource()->ObjectCB.get();
	if (NumFramesDirty > 0)
	{
		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&m_World);
		DirectX::XMMATRIX texTransform = DirectX::XMLoadFloat4x4(&m_TexTransform);

		ObjectConstants objConstants;
		DirectX::XMStoreFloat4x4(&objConstants.World, DirectX::XMMatrixTranspose(texTransform));

		currObjectCB->CopyData(ObjIndex, objConstants);

		NumFramesDirty--;
	}

	// TO-DO: UpdateMaterial()
	auto currMaterialCB = rdr.GetCurrFrameResource()->MaterialCB.get();
	if (m_Mat == nullptr)
		return;
	if (m_Mat->NumFramesDirty > 0)
	{
		DirectX::XMMATRIX matTransform = DirectX::XMLoadFloat4x4(&m_Mat->MatTransform);

		MaterialConstants matConstants;
		matConstants.DiffuseAlbedo = m_Mat->DiffuseAlbedo;
		matConstants.FresnelR0 = m_Mat->FresnelR0;
		matConstants.Roughness = m_Mat->Roughness;
		DirectX::XMStoreFloat4x4(&matConstants.MatTransform, DirectX::XMMatrixTranspose(matTransform));

		currMaterialCB->CopyData(ObjIndex, matConstants);
		m_Mat->NumFramesDirty--;

	}
	return;
}

void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept
{
	binds.push_back(std::move(bind));
}


void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept
{
	pIndexBuffer = ibuf.get();

	binds.push_back(std::move(ibuf));
}

void Drawable::AddTexture(const std::string& Name, const textureInfo& texInfo, Renderer& rdr)
{
	m_Tex = std::make_unique<DX14Texture>(Name, texInfo, rdr);

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
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Format = m_Tex->GetTexDesc().Format;
	srvDesc.Texture2D.MipLevels = m_Tex->GetTexDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	rdr.GetDevice()->CreateShaderResourceView(m_Tex->GetResource(), &srvDesc, srvHandle);

	BuildMaterial();
}

void Drawable::AddTexture(const std::string& Name, const char* path, Renderer& rdr)
{
	m_Tex = std::make_unique<DX14Texture>(Name, path, ObjIndex, rdr);

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
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Format = m_Tex->GetTexDesc().Format;
	srvDesc.Texture2D.MipLevels = m_Tex->GetTexDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	rdr.GetDevice()->CreateShaderResourceView(m_Tex->GetResource(), &srvDesc, srvHandle);

	BuildMaterial();
}

void Drawable::AddPbr(const std::string& Name, const textureInfo& pbrInfo, Renderer& rdr)
{
	m_Pbr = std::make_unique<DX14Texture>(Name, pbrInfo, rdr);
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(rdr.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&(rdr.GetIndexpbrDescriptorHeap(ObjIndex)))));

	ID3D12DescriptorHeap* descriptorHeap[] = { rdr.GetIndexpbrDescriptorHeap(ObjIndex).Get() };

	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(rdr.GetIndexpbrDescriptorHeap(ObjIndex)->GetCPUDescriptorHandleForHeapStart());
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

	srvDesc.Format = m_Pbr->GetTexDesc().Format;
	srvDesc.Texture2D.MipLevels = m_Pbr->GetTexDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	rdr.GetDevice()->CreateShaderResourceView(m_Pbr->GetResource(), &srvDesc, srvHandle);

}

void Drawable::BuildMaterial()
{
	
	auto tempMat = std::make_unique<Material>();
	tempMat->Name = m_Tex->GetName();
	tempMat->MatCBIndex = ObjIndex;
	tempMat->DiffuseSrvHeapIndex = ObjIndex;
	tempMat->DiffuseAlbedo = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	tempMat->FresnelR0 = DirectX::XMFLOAT3(0.07f, 0.07f, 0.07f);
	tempMat->Roughness = 0.3f;
	m_Mat = std::move(tempMat);
}
