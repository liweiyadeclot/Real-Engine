#pragma once
#include "Bindable.h"

class Bindable;

class IndexBuffer : public Bindable
{
public:
	IndexBuffer(Renderer& rdr, const std::vector<unsigned short>& indices);
	void Bind(Renderer& gfx) noexcept override;
	UINT GetCount() const noexcept;

	D3D12_INDEX_BUFFER_VIEW IndexBufferView() const;
protected:
	UINT count;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBufferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBufferUploader = nullptr;

	// Data about the buffers.
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;
};