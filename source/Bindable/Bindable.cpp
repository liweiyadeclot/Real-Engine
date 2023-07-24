#include "Bindable.h"

ID3D12Device* Bindable::GetDevice(Renderer& rdr) noexcept
{
	return rdr.m_Device.Get();
}

ID3D12GraphicsCommandList* Bindable::GetCmdList(Renderer& rdr) noexcept
{
	return rdr.m_CommandList.Get();
}