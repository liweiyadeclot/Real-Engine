#include "VertexShader.h"

VertexShader::VertexShader(const std::wstring& path)
{
	pBytecodeBlob = d3dUtil::CompileShader(path, nullptr, "VS", "vs_5_0");
}

ID3DBlob* VertexShader::GetByteCode() const noexcept
{
	return pBytecodeBlob.Get();
}

