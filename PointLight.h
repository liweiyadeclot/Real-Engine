#pragma once
#include "LightBase.h"
#include "Drawable.h"

class PointLight : public LightBase, public Drawable
{
public:
	PointLight() = delete;
	PointLight(Renderer& rdr);
	~PointLight() = default;

	virtual void Draw(Renderer& rdr) const noexcept override;
	virtual void UpdateDataToRenderer(Renderer& rdr, int i) override;
	virtual void UpdateShadowTransform(float x, float y, float z, float radius) override;
private:
	float fallOffStart;
	float fallOfEnd;
};