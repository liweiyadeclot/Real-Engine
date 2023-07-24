#include "../include/RenderItem.h"

void RenderItem::BuildRIGeometry(Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice, 
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCommandList,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries, 
	const nodeInfo& info)
{
	// Specify the Name.
	this->RIName = info.name;

	// Calculate the size of vb & ib.
	const UINT vbByteSize = (UINT)info.verticles.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)info.indices.size() * sizeof(std::uint16_t);

	// Create MeshGeometry and VertexBuffer & IndexBuffer
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = info.name;
	
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), info.verticles.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), info.indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), info.verticles.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), info.indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;


	// Create Submesh for MeshGeometry
	// Store the Offset for Each Submesh
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)info.indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	// Collect Submesh to MeshGeometry
	geo->DrawArgs[info.name] = submesh;

	mGeometries[geo->Name] = std::move(geo);
}

void RenderItem::BuildMaterial(UINT ObjCBIndex, std::unordered_map<std::string, std::unique_ptr<Material>>& mMaterials) 
{
	this->Mat->Name = this->RIName;
	this->Mat->MatCBIndex = ObjCBIndex;
	this->Mat->DiffuseSrvHeapIndex = ObjCBIndex;
	this->Mat->NormalSrvHeapIndex = ObjCBIndex;

	auto abc = std::make_unique<Material>(*(this->Mat));
	mMaterials[this->RIName] = std::move(abc);
}

void RenderItem::BuildRenderItem(UINT ObjCBIndex, std::vector<RenderItem*>* mRitemLayer,
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>>& mGeometries,
	std::vector<std::unique_ptr<RenderItem>>& mAllRitems,
	bool alphamode)
{
	this->World = MathHelper::Identity4x4();
	this->TexTransform = MathHelper::Identity4x4();
	this->ObjCBIndex = ObjCBIndex;
	this->Geo = mGeometries[this->RIName].get();
	this->IndexCount = this->Geo->DrawArgs[this->RIName].IndexCount;
	this->StartIndexLocation = this->Geo->DrawArgs[this->RIName].StartIndexLocation;
	this->BaseVertexLocation = this->Geo->DrawArgs[this->RIName].BaseVertexLocation;
	// auto abc = std::make_unique<RenderItem>();
	auto abc = std::make_unique<RenderItem>(*this);
	if(alphamode)
		mRitemLayer[1].push_back(abc.get());
	else
		mRitemLayer[0].push_back(abc.get());

	// auto temp = std::make_unique<RenderItem>(this);
	mAllRitems.push_back(std::move(abc));
}
