#pragma once
#include "Bindable.h"
#include "VertexShader.h"
#include "PixelShader.h"

class ShaderBase : public Bindable
{
public:
	ShaderBase() = delete;
	ShaderBase& operator=(const ShaderBase&) = delete;
	ShaderBase(std::string name, Renderer& rdr, const std::wstring& path,
		const std::vector<Renderer::RootParaType>& rootParas);
	~ShaderBase() = default;
	
	void Bind(Renderer& rdr) noexcept;
	std::vector<Renderer::RootParaType> GetRootParaTypes()
	{
		return m_RootParas;
	}

protected:
	// TO-DO: InputLayout and RootSignature affairs
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PipelineState;
	std::vector<Renderer::RootParaType> m_RootParas;
	virtual void CreatePipelineState(Renderer& rdr);
	VertexShader m_vs;
	PixelShader m_ps;
private:
	std::string m_Name;
};