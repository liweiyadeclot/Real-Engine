#pragma once
#include "Bindable.h"
#include "VertexShader.h"
#include "PixelShader.h"


class Shader : public Bindable
{
public:
	Shader() = delete;
	Shader& operator=(const Shader&) = delete;
	// Shader is the base class of all shaders
	// other derived shaders should has its own
	// rootParameters
	Shader(std::string name, Renderer& rdr, const CD3DX12_ROOT_PARAMETER* rootParameters, UINT numRootParameters, const std::wstring& path);
	~Shader() = default;

	void Bind(Renderer& rdr) noexcept override;

	ID3D12RootSignature* GetRootSignature();
public:
	VertexShader vs;
	PixelShader ps;
private:
	void CreatePipelineState(Renderer& rdr);
private:
	std::string Name;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rtSig;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> ppState = nullptr;
	// TO-DO: sampler for texture & material
};