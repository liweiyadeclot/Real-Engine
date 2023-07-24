#pragma once
#include "ShadowMapShader.h"

class SkyBoxShadowMapShader : public ShadowMapShader
{
public:
	SkyBoxShadowMapShader() = delete;
	SkyBoxShadowMapShader(const SkyBoxShadowMapShader&) = delete;

	SkyBoxShadowMapShader(std::string Name, Renderer& rdr, const std::wstring& path,
		const std::vector<Renderer::RootParaType>& rootParas);

	virtual void CreatePipelineState(Renderer& rdr) override;
};