#include "Drawable.h"

void Drawable::Draw(Renderer& rdr) const noexcept
{

	for (auto& b : binds)
	{
		b->Bind(rdr);
	}

	rdr.ExecuteCommands();

	rdr.PrepForDrawToScreen();
	rdr.DrawIndexed(pIndexBuffer->GetCount(), StartIndexLocation, BaseVertexLocation, ObjIndex);
}

void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept
{
	binds.push_back(std::move(bind));
}

void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept
{
	pIndexBuffer = ibuf.get();

	binds.push_back(std::move(ibuf));
}
