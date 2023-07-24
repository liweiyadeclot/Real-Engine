#pragma once
#include "Drawable.h"
#include "BindableBase.h"

class SingleTriangle : public Drawable
{
public:
	SingleTriangle(Renderer& rdr);
	void Update(float dt, Renderer& rdr) noexcept override;
};