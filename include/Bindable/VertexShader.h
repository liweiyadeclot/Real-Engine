#pragma once
#include "Bindable.h"

class VertexShader
{
public:
	VertexShader() = default;
	VertexShader(const std::wstring& path);

	ID3DBlob* GetByteCode() const noexcept;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob = nullptr;
};