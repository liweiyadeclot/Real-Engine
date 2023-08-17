#include "Lighting/PointLight.h"
#include "MinimalShader.h"
#include "GeometryGenerator.h"
#include "VertexBuffer.h"
#include "imgui.h"

PointLight::PointLight(Renderer& rdr)
	: Drawable(rdr), LightBase()
{
	std::vector<Renderer::RootParaType> rootTypes;
	rootTypes.push_back(Renderer::RootParaType::ObjectConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::MainPassConstBuffer);
	rootTypes.push_back(Renderer::RootParaType::ShaderResource);

	auto pointerToShader = std::make_unique<MinimalShader>("MinimalShader", rdr, L"./minimalShader.hlsl", rootTypes
	);
	AddBind(std::move(pointerToShader));

	GeometryGenerator geogen;
	GeometryGenerator::MeshData lightSphere = geogen.CreateSphere(1.0f, 10, 10);
	size_t verticesCount = lightSphere.Vertices.size();
	std::vector<Vertex> vertices(verticesCount);
	for (size_t i = 0; i < verticesCount; ++i)
	{
		vertices[i].Pos = lightSphere.Vertices[i].Position;
		vertices[i].Normal = lightSphere.Vertices[i].Normal;
		vertices[i].TexC = lightSphere.Vertices[i].TexC;
	}

	std::vector<uint16_t> indices = lightSphere.GetIndices16();
	m_IndexCount = indices.size();

	AddBind(std::make_unique<VertexBuffer>(rdr, vertices));
	AddIndexBuffer(std::make_unique<IndexBuffer>(rdr, indices));
}


void PointLight::Update(float dt, Renderer& rdr)  noexcept
{
	DirectX::XMStoreFloat4x4(&m_World, DirectX::XMMatrixTranslation(m_LightPosW.x, m_LightPosW.y, m_LightPosW.z) *
		DirectX::XMLoadFloat4x4(&m_World));
	Drawable::Update(dt, rdr);
	return;
}

void PointLight::DrawToShadowMap(Renderer& rdr) const noexcept
{
	// Do not draw shadow
	return;
}

void PointLight::UpdateDataToRenderer(Renderer& rdr, int i)
{
	rdr.Lights[i].Strength = DirectX::XMFLOAT3(m_Intensity * m_LightColor.x,
		m_Intensity * m_LightColor.y,
		m_Intensity * m_LightColor.z);

	rdr.Lights[i].Position = m_LightPosW;
	rdr.Lights[i].FalloffStart = fallOffStart;
	rdr.Lights[i].FalloffEnd = fallOffEnd;

	// rdr.SetShadowTransform(m_ShadowTransform);
}

void PointLight::UpdateShadowTransform(float x, float y, float z, float radius)
{
	// do not have shadow
	return;
}

void PointLight::SpawnImGuiControlPanel()
{
	if (ImGui::Begin("PointLight"))
	{
		ImGui::Text("Color");
		ImGui::ColorEdit3("Diffuse Color", &m_LightColor.x);


		ImGui::Text("Position");
		ImGui::SliderFloat("X", &m_LightPosW.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &m_LightPosW.y, -10.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &m_LightPosW.z, -60.0f, 60.0f, "%.1f");
	}
	ImGui::End();
}