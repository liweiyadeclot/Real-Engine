#pragma once

#include "d3dUtil.h"
#include "d3dx12.h"
#include "DX13App.h"
#include "GLTFloader.h"
#include <string>

class Texture2D
{
public:
	Texture2D() = default;
	void LoadFromFile(const std::string& Name, const char* FileName);
	void LoadFromData(const nodeInfo& info);
	void LoadFromData(const textureInfo& texInfo, const std::string& Name);
	CD3DX12_RESOURCE_DESC GetTexDesc()
	{
		return textureDesc;
	}

	Texture* GetTex()
	{
		return &mTex;
	}

	unsigned char* GetDataPointer()
	{
		return image_data;
	}
public:
	int width;
	int height;
	int channels;
private:
	Texture mTex;

	CD3DX12_RESOURCE_DESC textureDesc = {};

	unsigned char* image_data;
};