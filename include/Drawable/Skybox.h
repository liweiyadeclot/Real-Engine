#pragma once
#include "Drawable.h"
#include "BindableBase.h"
#include "GeometryGenerator.h"

class Skybox : public Drawable
{
public:
	Skybox() = default;
	Skybox(Renderer& rdr);
	void Draw(Renderer& rdr) const noexcept override;

	virtual void RenderToShadowMap(Renderer& rdr) const noexcept override;
private:
	void AddTexture(Renderer& rdr);
	std::unique_ptr<class DX14Texture> m_Tex;
	void BuildMaterial();
	UINT m_indexcount;

};