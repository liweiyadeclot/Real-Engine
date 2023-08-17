#pragma once
#include "ShaderBase.h"

class MinimalShader : public ShaderBase
{
public:
	MinimalShader() = delete;
	MinimalShader& operator=(const MinimalShader&) = delete;
	MinimalShader(std::string name, Renderer& rdr, const std::wstring& path,
		const std::vector<Renderer::RootParaType>& rootParas);
	~MinimalShader() = default;
	
	std::vector<Renderer::RootParaType> GetRootParaTypes()
	{
		return m_RootParas;
	}
protected:
	virtual void CreatePipelineState(Renderer& rdr) override;
};