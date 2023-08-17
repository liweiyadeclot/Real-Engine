#pragma once
#include "imgui.h"
#include "Drawable.h"
#include "LightBase.h"


class PointLight : public LightBase, public Drawable
{
public:
	PointLight() = delete;
	PointLight(Renderer& rdr);
	~PointLight() = default;

	virtual void Update(float dt, Renderer& rdr) noexcept override;
	virtual void RenderToShadowMap(Renderer& rdr) const noexcept;
	virtual void UpdateDataToRenderer(Renderer& rdr, int i) override;
	virtual void UpdateShadowTransform(float x, float y, float z, float radius) override;
	virtual void SpawnImGuiControlPanel() override;

private:
	Renderer rdr;

	float fallOffStart = 1.0f;
	float fallOffEnd = 100.0f;

	UINT m_IndexCount;
};