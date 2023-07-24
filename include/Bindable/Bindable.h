#pragma once
#include "../Renderer.h"

class Bindable
{
public:
	virtual void Bind(Renderer& rdr) noexcept = 0;
	virtual ~Bindable() = default;
protected:
	static ID3D12Device* GetDevice(Renderer& rdr) noexcept;
	static ID3D12GraphicsCommandList* GetCmdList(Renderer& rdr) noexcept;

};