#include "../include/Texture2D.h"
#include "../include/stb_image.h"


void Texture2D::LoadFromFile(const std::string& Name, const char* FileName)
{
	mTex.Name = Name;
	mTex.Filename = *FileName;
	// use stbImage to load .png file
	image_data = stbi_load(FileName, &width, &height, &channels, STBI_rgb_alpha);

	// texture source desc
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


}

void Texture2D::LoadFromData(const nodeInfo& info)
{
	image_data = info.tex.data;
	width = info.tex.width;
	height = info.tex.height;
	channels = info.tex.channel;

	mTex.Name = info.name;


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
}

void Texture2D::LoadFromData(const textureInfo& texInfo, const std::string& Name)
{
	image_data = texInfo.data;
	width = texInfo.width;
	height = texInfo.height;
	channels = texInfo.channel;

	mTex.Name = Name;


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
}

