#pragma once
#include <vector>
#include "d4dApp.h"
#include "FrameResource.h"
#include "GLTFloader.h"
#include "Model.h"
#include "Renderer.h"
#include "Skybox.h"
#include "Floor.h"
#include "Lighting/LightBase.h"
#include "Lighting/DirectionalLight.h"
#include "Lighting/PointLight.h"

using Microsoft::WRL::ComPtr;

class DX14App : public D4DApp
{
public:
	DX14App(HINSTANCE hInstance);
	DX14App(const DX14App& rhs) = delete;
	DX14App& operator=(const DX14App& rhs) = delete;
	~DX14App();

	virtual bool Initialize() override;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	void OnKeyboardInput(const GameTimer& gt);
	//mouse msg
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

private:
	std::vector<std::shared_ptr<class Drawable>> drawables;
	POINT mLastMousePos;

	std::vector<std::shared_ptr<LightBase>> Lights;
};