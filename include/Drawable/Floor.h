#pragma once
#include "Drawable.h"
#include "BindableBase.h"

class Floor : public Drawable
{
public:
	Floor(Renderer& rdr);
	void Update(float dt, Renderer& rdr) noexcept override;
};