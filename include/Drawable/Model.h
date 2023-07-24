#pragma once
#include "Drawable.h"
#include "BindableBase.h"
#include "GLTFloader.h"
#include "Part.h"

class Model : public Drawable
{
public:
	Model(Renderer& rdr);
	void Update(float dt, Renderer& rdr) noexcept override;
	void Draw(Renderer& rdr) const noexcept override;

	virtual void RenderToShadowMap(Renderer& rdr) const noexcept override;
private:
	std::vector<std::unique_ptr<Part>> parts;
};

