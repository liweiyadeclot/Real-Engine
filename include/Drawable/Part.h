#pragma once
#include "Drawable.h"
#include "BindableBase.h"

class Part : public Drawable
{
public:
	Part() = default;
	Part(const Part& rhs) = delete;
	Part(std::string& name, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, 
		Renderer& rdr, const textureInfo& texinfo, const textureInfo& pbrinfo,bool alphamode);

	void Update(float dt, Renderer& rdr) noexcept override;
};