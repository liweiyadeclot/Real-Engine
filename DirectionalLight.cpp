#include "DirectionalLight.h"
#include "imgui.h"

void DirectionalLight::SpawnImGuiControlPanel()
{
	if (ImGui::Begin("DirectionalLight"))
	{
		ImGui::Text("Color");
		ImGui::ColorEdit3("Diffuse Color", &m_LightColor.x);


		ImGui::Text("Position");
		ImGui::SliderFloat("X", &m_LightPosW.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &m_LightPosW.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &m_LightPosW.z, -60.0f, 60.0f, "%.1f");
	}
	ImGui::End();
}

void DirectionalLight::UpdateShadowTransform(float x, float y, float z, float radius)
{
	using namespace DirectX;
	XMVECTOR lightPos = XMLoadFloat3(&m_LightPosW);
	XMFLOAT3 sceneCenter(x, y, z);
	XMVECTOR targetPos = XMLoadFloat3(&sceneCenter);
	XMVECTOR lightUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, lightUp);

	XMStoreFloat3(&m_LightPosW, lightPos);

	// Transform bounding sphere to light space.
	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, lightView));

	// Ortho frustum in light space encloses scene.
	float l = sphereCenterLS.x - radius;
	float b = sphereCenterLS.y - radius;
	float n = sphereCenterLS.z - radius;
	float r = sphereCenterLS.x + radius;
	float t = sphereCenterLS.y + radius;
	float f = sphereCenterLS.z + radius;

	m_LightNearZ = n;
	m_LightFarZ = f;
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	// Transform NDC space [-1,+1]^2 to texture space [0,1]^2
	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMMATRIX S = lightView * lightProj * T;
	XMStoreFloat4x4(&m_LightView, lightView);
	XMStoreFloat4x4(&m_LightProj, lightProj);
	XMStoreFloat4x4(&m_ShadowTransform, S);
}

void DirectionalLight::UpdateDataToRenderer(Renderer& rdr, int i)
{
	rdr.Lights[i].Direction = m_LightDir;
	rdr.Lights[i].Strength = DirectX::XMFLOAT3(m_Intensity * m_LightColor.x, m_Intensity * 
		m_LightColor.y, m_Intensity * m_LightColor.z);
}
