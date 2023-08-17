#include "Lighting/LightBase.h"

using namespace DirectX;

void LightBase::UpdateShadowPassCB(Renderer& rdr)
{
	XMMATRIX view = XMLoadFloat4x4(&m_LightView);
	XMMATRIX proj = XMLoadFloat4x4(&m_LightProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	UINT w = rdr.m_NewShadowMap->Width();
	UINT h = rdr.m_NewShadowMap->Height();

	XMStoreFloat4x4(&rdr.m_ShadowPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&rdr.m_ShadowPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&rdr.m_ShadowPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&rdr.m_ShadowPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&rdr.m_ShadowPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&rdr.m_ShadowPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	rdr.m_ShadowPassCB.EyePosW = m_LightPosW;
	rdr.m_ShadowPassCB.RenderTargetSize = XMFLOAT2((float)w, (float)h);
	rdr.m_ShadowPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / w, 1.0f / h);
	rdr.m_ShadowPassCB.NearZ = m_LightNearZ;
	rdr.m_ShadowPassCB.FarZ = m_LightFarZ;

	auto currPassCB = rdr.m_CurrFrameResource->PassCB.get();

	// Now only one light have shadow and ShadowPassCB
	currPassCB->CopyData(1, rdr.m_ShadowPassCB);
}
