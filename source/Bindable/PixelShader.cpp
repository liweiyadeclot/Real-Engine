#include "PixelShader.h"

PixelShader::PixelShader(const std::wstring& path)
{
	pBytecodeBlob = d3dUtil::CompileShader(path, nullptr, "PS", "ps_5_0");
}

ID3DBlob* PixelShader::GetByteCode() const noexcept
{
	return pBytecodeBlob.Get();
}
