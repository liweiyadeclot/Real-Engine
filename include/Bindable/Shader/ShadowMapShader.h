#pragma once
#include "ShaderBase.h"

class ShadowMapShader : public ShaderBase
{
public:
	ShadowMapShader() = delete;
	ShadowMapShader(const ShadowMapShader&) = delete;
	ShadowMapShader(std::string Name, Renderer& rdr, const std::wstring& path,
		const std::vector<Renderer::RootParaType>& rootParas);
protected:
	virtual void CreatePipelineState(Renderer& rdr) override;
};