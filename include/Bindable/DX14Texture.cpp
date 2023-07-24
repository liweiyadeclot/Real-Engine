#include "DX14Texture.h"

DX14Texture::DX14Texture(const std::string& Name, const textureInfo& texInfo, Renderer& rdr)
	: image_data(texInfo.data), width(texInfo.width), height(texInfo.height),
		channels(texInfo.channel), Name(Name)
{
	textureDesc = CD3DX12_RESOURCE_DESC(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		width,
		height,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_NONE
	);




	ThrowIfFailed(rdr.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&(textureDesc),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Resource)
	));

	// Create uploadBuffer
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Resource.Get(), 0, 1);


	ThrowIfFailed(rdr.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadHeap)
	));

	// upload picture data to heap resource
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = image_data;
	textureData.RowPitch = width * 4;
	textureData.SlicePitch = textureData.RowPitch * height;

	// deliver data
	UpdateSubresources(rdr.GetCommandList(), Resource.Get(), UploadHeap.Get(), 0u, 0u, 1u, &textureData);

	// convert state of texture resource to SRV
	rdr.GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

	// TO-DO: Create SRV for textures after all texture done

	
}

DX14Texture::DX14Texture(const std::string& Name, const char* path, int ObjIndex, Renderer& rdr)
	: Name(Name)
{
	textureInfo tex;
	tex.data = stbi_load(path, &tex.width, &tex.height, &tex.channel, 0);
	
	image_data_from_png = tex.data;
	width = tex.width;
	height = tex.height;
	channels = tex.channel;

	textureDesc = CD3DX12_RESOURCE_DESC(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		width,
		height,
		1,
		1,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_NONE
	);




	ThrowIfFailed(rdr.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&(textureDesc),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&Resource)
	));

	// Create uploadBuffer
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(Resource.Get(), 0, 1);


	ThrowIfFailed(rdr.GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&UploadHeap)
	));

	// upload picture data to heap resource
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = image_data_from_png;
	textureData.RowPitch = width * 4;
	textureData.SlicePitch = textureData.RowPitch * height;

	// deliver data
	UpdateSubresources(rdr.GetCommandList(), Resource.Get(), UploadHeap.Get(), 0u, 0u, 1u, &textureData);

	// convert state of texture resource to SRV
	rdr.GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(Resource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
}



