#pragma once

#include "../d3dUtil.h"

class PixelShader
{
public:
	PixelShader() = default;
	PixelShader(const std::wstring& path);

	ID3DBlob* GetByteCode() const noexcept;
private:
	Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
};