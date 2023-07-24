#pragma once
#include "GLTFloader.h"
#include "Renderer.h"
#include "stb_image.h"
#include <string>

class DX14Texture
{
public:
	DX14Texture() = default;
	DX14Texture(const std::string& Name, const textureInfo& texInfo, Renderer& rdr);
	DX14Texture(const std::string& Name, const char* path, int ObjIndex, Renderer& rdr);
	~DX14Texture()
	{
		if (image_data_from_png != nullptr)
		{
			delete image_data_from_png;
			image_data_from_png = nullptr;
		}
	}
	CD3DX12_RESOURCE_DESC GetTexDesc()
	{
		return textureDesc;
	}

	unsigned char* GetDataPointer()
	{
		return image_data;
	}

	ID3D12Resource* GetResource()
	{
		return Resource.Get();
	}

	std::string GetName()
	{
		return Name;
	}

	ID3D12Resource* GetUploadHeap()
	{
		return UploadHeap.Get();
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

private:
	std::string Name;
	std::wstring Filename;

	CD3DX12_RESOURCE_DESC textureDesc{};
	unsigned char* image_data;
	unsigned char* image_data_from_png;


	int width;
	int height;
	int channels;
};