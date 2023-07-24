#include "DirectX14App.h"

const int gNumFrameResources = 3;

DX14App::DX14App(HINSTANCE hInstance)
	: D4DApp(hInstance)
{
	rdr.SetSceneCenter(0.0f, 0.0f, 0.0f);
	rdr.SetSceneRadius(sqrtf(10.0f * 10.0f + 15.0f * 15.0f));
}

DX14App::~DX14App()
{
	rdr.DestructorInApp();
}

bool DX14App::Initialize()
{
	if(!D4DApp::Initialize())
		return false;
	drawables.push_back(std::make_unique<Skybox>(rdr));
	drawables.push_back(std::make_unique<Floor>(rdr));
	drawables.push_back(std::make_unique<Model>(rdr));
	rdr.SetRendererState();
	rdr.ExecuteCommands();
	return true;
}

LRESULT DX14App::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return D4DApp::MsgProc(hwnd, msg, wParam, lParam);
}

void DX14App::OnResize()
{
	D4DApp::OnResize();
	rdr.OnResize();
}

void DX14App::Update(const GameTimer& gt)
{
	OnKeyboardInput(gt);
	
	rdr.UpdateRenderer(gt);

	for (auto& d : drawables)
	{
		d->Update(gt.DeltaTime(), rdr);
	}
}

void DX14App::Draw(const GameTimer& gt)
{
	rdr.ResetRenderer();
	rdr.SetRenderToDrawToShadowMap();
	for (auto& d : drawables)
	{
		d->RenderToShadowMap(rdr);
	}
	rdr.SetRenderToDrawToScreen();
	for (auto& d : drawables)
	{
		d->Draw(rdr);
	}
	rdr.PresentOneFrame();
}

void DX14App::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();
	if (GetAsyncKeyState('W') & 0x8000)
		rdr.mCamera.Walk(10.0f * dt);
	if (GetAsyncKeyState('A') & 0x8000)
		rdr.mCamera.Strafe(10.0f * dt);
	if (GetAsyncKeyState('S') & 0x8000)
		rdr.mCamera.Walk(-10.0f * dt);
	if (GetAsyncKeyState('D') & 0x8000)
		rdr.mCamera.Strafe(-10.0f * dt);

	rdr.mCamera.UpdateViewMatrix();
	return;
}

void DX14App::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mhMainWnd);
	return;
}

void DX14App::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void DX14App::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update camera
		rdr.mCamera.RotateY(-dx);
		rdr.mCamera.Pitch(dy);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	return;
}

