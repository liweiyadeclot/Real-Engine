#include "Model.h"

Model::Model(Renderer& rdr)
	: Drawable(rdr)
{
	Loader loader;
	loader.loadfromadd(".\\Models\\FlightHelmet\\FlightHelmet.gltf");

	//vertex
	for (size_t i = 0; i < loader.nodeInfos.size(); ++i) {
		auto pointerToNode = std::make_unique<Part>(loader.nodeInfos[i].name, 
			loader.nodeInfos[i].verticles, loader.nodeInfos[i].indices,
			rdr, loader.nodeInfos[i].tex,loader.nodeInfos[i].pbr,
			loader.nodeInfos[i].alphamode);
		parts.push_back(std::move(pointerToNode));
		
	}
}

void Model::Update(float dt, Renderer& rdr) noexcept
{
	for (auto& p : parts)
	{
		p->Update(dt, rdr);
	}
	return;
}

void Model::Draw(Renderer& rdr) const noexcept
{
	for (size_t i = 0; i < parts.size(); ++i) {
		parts[i]->Draw(rdr);
	}
}

void Model::RenderToShadowMap(Renderer& rdr) const noexcept
{
	for (size_t i = 0; i < parts.size(); ++i) {
		parts[i]->RenderToShadowMap(rdr);
	}
}

