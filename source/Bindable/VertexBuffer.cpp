#include "VertexBuffer.h"

void VertexBuffer::Bind(Renderer& rdr) noexcept
{
	GetCmdList(rdr)->IASetVertexBuffers(0, 1, &VertexBufferView());
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::VertexBufferView() const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = VertexBufferGPU->GetGPUVirtualAddress();
	vbv.StrideInBytes = VertexByteStride;
	vbv.SizeInBytes = VertexBufferByteSize;

	return vbv;
}
