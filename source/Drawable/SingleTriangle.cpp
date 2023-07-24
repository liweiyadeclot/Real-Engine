#include "SingleTriangle.h"

SingleTriangle::SingleTriangle(Renderer& rdr)
	: Drawable(rdr)
{
	namespace dx = DirectX;

	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);

	auto pointerToShader = std::make_unique<ShaderBase>("test", rdr, L"./testNewStruct.hlsl", rootTypes);
	AddBind(std::move(pointerToShader));
	AddTexture(std::string("bricks"), "./Models/Cube/Cube_BaseColor.png", rdr);
	struct Vertex
	{
		dx::XMFLOAT3 pos;
		dx::XMFLOAT4 color;
	};

	std::vector<Vertex> data(3);
	data[0].pos = { -1.0f, -1.0f, 1.0f };
	data[1].pos = {  1.0f, -1.0f, 1.0f };
	data[2].pos = {  0.0f,  0.5f, 1.0f };

	data[0].color = { 1.0f, 0.0f, 0.0f, 1.0f };
	data[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	data[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };

	AddBind(std::make_unique<VertexBuffer>(rdr, data));

	std::vector<unsigned short> indices = { 0, 1, 2 };
	AddIndexBuffer(std::make_unique<IndexBuffer>(rdr, indices));
}

void SingleTriangle::Update(float dt, Renderer& rdr) noexcept
{
	// do not update
	Drawable::Update(dt, rdr);
	return;
}
