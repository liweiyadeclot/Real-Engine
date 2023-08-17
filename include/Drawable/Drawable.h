#pragma once
#include "Bindable.h"
#include "IndexBuffer.h"
#include "Renderer.h"
#include "DX14Texture.h"
#include "ShadowMapShader.h"

class Drawable
{
public:
	Drawable() = delete;
	Drawable(Renderer& rdr);
	Drawable(const Drawable&) = delete;
	virtual void RenderToShadowMap(Renderer& rdr) const noexcept;
	virtual void Draw(Renderer& rdr) const noexcept;
	virtual void Update(float dt, Renderer& rdr) noexcept;
	virtual ~Drawable() = default;

public:
	static int ObjectsCount;
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	int ObjIndex = 0;
	int NumFramesDirty = gNumFrameResources;
	DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
protected:
	template<typename T>
	T* QueryBindable() noexcept
	{
		for (auto& pb : binds)
		{
			if (auto pt = tynamic_cast<T*>(pb.get()))
			{
				return pt;
			}
		}
		return nullptr;
	}
	void AddBind(std::unique_ptr<Bindable> bind) noexcept;
	void AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept;
	void AddTexture(const std::string& Name, const textureInfo& texInfo, Renderer& rdr);
	void AddTexture(const std::string& Name, const char* path, Renderer& rdr);
	void AddPbr(const std::string& Name, const textureInfo& pbrInfo, Renderer& rdr);
protected:
	std::vector<std::unique_ptr<Bindable>> binds;
	std::unique_ptr<Material> m_Mat;
	DirectX::XMFLOAT4X4 m_World = MathHelper::Identity4x4();

	DirectX::XMFLOAT4X4 m_TexTransform = MathHelper::Identity4x4();
	std::unique_ptr<ShadowMapShader> m_ShadowShader;

	DirectX::XMFLOAT3 m_PrePos = {0.0f, 0.0f, 0.0f};

private:
	const IndexBuffer* pIndexBuffer = nullptr;
	std::unique_ptr<class DX14Texture> m_Tex;
	std::unique_ptr<class DX14Texture> m_Pbr;
	
private:
	void BuildMaterial();
	
};

