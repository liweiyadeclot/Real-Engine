#include "DirectX14App.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

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

	auto ptrToPointLight = std::make_shared<PointLight>(rdr);
	Lights.push_back(std::make_unique<DirectionalLight>());
	//Lights.push_back(ptrToPointLight);
	//drawables.push_back(ptrToPointLight);

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

	for (size_t i = 0; i < Lights.size(); ++i)
	{
		Lights[i]->UpdateShadowTransform(0.0f, 0.0f, 0.0f, sqrtf(5.0f * 5.0f + 7.5f * 7.5f));
		Lights[i]->UpdateDataToRenderer(rdr, i);
	}
	
	// only first light can make shadow
	if(Lights.size() > 0)
		Lights[0]->UpdateShadowPassCB(rdr);

	
}

void DX14App::Draw(const GameTimer& gt)
{
	rdr.ResetRenderer();

	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	static bool show_demo_window = true;

	for (auto& light : Lights)
	{
		light->SpawnImGuiControlPanel();
	}

	SpawnMainControlPanel();

	ImGui::Render();

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
	rdr.DrawImGui();
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
	if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
		rdr.mCamera.IfControlinigCamera = true;
	else
		rdr.mCamera.IfControlinigCamera = false;
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
	if ((btnState & MK_LBUTTON) != 0 && rdr.mCamera.IfControlinigCamera)
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

void DX14App::SpawnMainControlPanel()
{
	if (ImGui::Begin("MainControlPanel"))
	{
		if (ImGui::Button("Add Point light"));
		{
			auto ptrToPointLight = std::make_shared<PointLight>(rdr);
			rdr.ClearRenderer();
			Lights.push_back(ptrToPointLight);
			drawables.push_back(ptrToPointLight);
			rdr.ResetRendererState();
			rdr.ResetRenderer();
			rdr.ExecuteCommands();
			rdr.ResetRenderer();
		}

		if (ImGui::Button("Add Model"))
		{
			showAddModelWindow = true;
		}
	}
	ImGui::End();
}


