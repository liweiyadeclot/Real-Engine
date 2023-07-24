#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(Renderer& rdr, const std::vector<unsigned short>& indices)
	:
	count((UINT)indices.size())
{
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &IndexBufferCPU));
	CopyMemory(IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	IndexBufferGPU = d3dUtil::CreateDefaultBuffer(GetDevice(rdr),
		GetCmdList(rdr), indices.data(), ibByteSize, IndexBufferUploader);

	IndexFormat = DXGI_FORMAT_R16_UINT;
	IndexBufferByteSize = ibByteSize;

}

void IndexBuffer::Bind(Renderer& rdr) noexcept
{
	GetCmdList(rdr)->IASetIndexBuffer(&IndexBufferView());
}

UINT IndexBuffer::GetCount() const noexcept
{
	return count;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::IndexBufferView() const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = IndexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = IndexFormat;
	ibv.SizeInBytes = IndexBufferByteSize;

	return ibv;
}