#pragma once
#include "Bindable.h"

class VertexBuffer : public Bindable
{
public:
	template<typename V>
	VertexBuffer(Renderer& rdr, const std::vector<V>& vertices)
	{
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(V);

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &VertexBufferCPU));
		CopyMemory(VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		VertexBufferGPU = d3dUtil::CreateDefaultBuffer(GetDevice(rdr),
			GetCmdList(rdr), vertices.data(), vbByteSize, VertexBufferUploader);

		VertexByteStride = sizeof(V);
		VertexBufferByteSize = vbByteSize;
	}


	void Bind(Renderer& rdr) noexcept override;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const;
protected:
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBufferUploader = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
};

