#pragma once
#include "Part.h"

Part::Part(std::string& name, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices,
	Renderer& rdr, const textureInfo& texinfo, const textureInfo& pbrinfo, bool alphamode)
	: Drawable(rdr)
{
	//vertex
	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MaterialConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::TextureTable);
	rootTypes.push_back(Renderer::RootParaType::MaterialTable);
	rootTypes.push_back(Renderer::RootParaType::ShadowTable);

	auto pointerToShader = std::make_unique<ModelShader>("test", rdr, L"./demo.hlsl", rootTypes, alphamode, false);
	AddBind(std::move(pointerToShader));

	AddBind(std::make_unique<VertexBuffer>(rdr, vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(rdr, indices));
	AddTexture(name, texinfo, rdr);
	AddPbr(name, pbrinfo, rdr);
	//pbr
	//tex
}

void Part::Update(float dt, Renderer& rdr) noexcept
{
	Drawable::Update(dt, rdr);
	return;
}