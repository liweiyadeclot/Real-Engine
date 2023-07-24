#pragma once
#include "ShaderBase.h"

class ModelShader : public ShaderBase
{
public:
	ModelShader() = delete;
	ModelShader& operator=(const ShaderBase&) = delete;
	ModelShader(std::string name, Renderer& rdr, const std::wstring& path,
		const std::vector<Renderer::RootParaType>& rootParas, bool alphamode, bool skymode);
	~ModelShader() = default;

	std::vector<Renderer::RootParaType> GetRootParaTypes()
	{
		return m_RootParas;
	}
protected:
	virtual void CreatePipelineState(Renderer& rdr) override;
private:
	bool m_Alphamode;
	bool m_Skymode;
};