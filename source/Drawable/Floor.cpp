#include "Floor.h"

Floor::Floor(Renderer& rdr)
	: Drawable(rdr)
{
	namespace dx = DirectX;

	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MaterialConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::TextureTable);
	rootTypes.push_back(Renderer::RootParaType::ShadowTable);


	auto pointerToShader = std::make_unique<ShaderBase>("test", rdr, L"./floor.hlsl", rootTypes);
	AddBind(std::move(pointerToShader));
	AddTexture(std::string("bricks"), "./Models/Cube/Cube_BaseColor.png", rdr);
	std::vector<Vertex> floorV =
	{
		Vertex(-2.0f, -0.001f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f), // 0 
		Vertex(-2.0f, -0.001f,  2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f),
		Vertex(2.0f, -0.001f,  2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f),
		Vertex(2.0f, -0.001f, -2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 4.0f),
	};

	AddBind(std::make_unique<VertexBuffer>(rdr, floorV));

	std::vector<std::uint16_t> floorI =
	{
		0, 1, 2,
		0, 2, 3,
	};
	AddIndexBuffer(std::make_unique<IndexBuffer>(rdr, floorI));
}

void Floor::Update(float dt, Renderer& rdr) noexcept
{
	// do not update
	Drawable::Update(dt, rdr);
	return;
}
