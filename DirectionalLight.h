#pragma once
#include "LightBase.h"

class DirectionalLight : public LightBase
{
public:
	DirectionalLight() = default;
	DirectionalLight(DirectX::XMFLOAT3 color, float intensity, DirectX::XMFLOAT3 dir)
		: LightBase(color, intensity), m_LightDir(dir){}

	virtual void SpawnImGuiControlPanel();
	virtual void UpdateShadowTransform(float x, float y, float z, float radius);
	virtual void UpdateDataToRenderer(Renderer& rdr, int i);
private:
	// m_LightDir is normalized
	DirectX::XMFLOAT3 m_LightDir;

};