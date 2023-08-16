#pragma once
#include "d3dUtil.h"
#include "Renderer.h"

class LightBase
{
public:
	LightBase() = default;
	LightBase(DirectX::XMFLOAT3 color, float intensity)
		: m_LightColor(color), m_Intensity(intensity){}
	~LightBase() = default;

	virtual void SpawnImGuiControlPanel() = 0;
	virtual void UpdateShadowTransform(float x, float y, float z, float radius) = 0;
	virtual void UpdateDataToRenderer(Renderer& rdr, int i) = 0;
	void UpdateShadowPassCB(Renderer& rdr);

protected:
	DirectX::XMFLOAT3 m_LightPosW = { 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT3 m_LightColor = { 1.0f, 1.0f, 1.0f };
	float m_Intensity = 1.0f;
	DirectX::XMFLOAT4X4 m_LightView = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_LightProj = MathHelper::Identity4x4();
	DirectX::XMFLOAT4X4 m_ShadowTransform = MathHelper::Identity4x4();

	float m_LightNearZ;
	float m_LightFarZ;
};	