/***** INCLUDES *****/
#include "ImGui Windows.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <stdlib.h>
#include <ctime>
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#pragma comment(lib, "d3d11.lib")


/***** GLOBALS ****/

namespace _ImGui_Windows_Reserved {
	static ID3D11Device* g_pd3dDevice = nullptr;
	static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
	static IDXGISwapChain* g_pSwapChain = nullptr;
	static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
	IDXGIOutput* g_pOutput = NULL;
	bool bMinimized = false, bOpen = false;
	int width = 0, height = 0;
	HWND hWnd = NULL;
	DRAWCALLBACK DrawCallback = NULL;
	ImGuiWindow* pWindow = (ImGuiWindow*)1;
}
using namespace _ImGui_Windows_Reserved;


/***** FORWARDS *****/

// Private forwards
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI WindowThread(void* args);

// ImGui Provided Functions
void CleanupRenderTarget();
void CreateRenderTarget();
void CleanupDeviceD3D();
bool CreateDeviceD3D(HWND hWnd);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


/***** FUNCTIONS *****/

bool ImGui_Window::Begin(char* Name, DRAWCALLBACK DrawCallback, int x, int y, int width, int height, WindowParams Params) {
	if (bOpen)
		return false;
	bOpen = true;
	_ImGui_Windows_Reserved::DrawCallback = DrawCallback;
	_ImGui_Windows_Reserved::width = width;
	_ImGui_Windows_Reserved::height = height;

	// Get class & window name
	char* WindowName = Name;
	if (Params & WindowParams::RandomClassName) {
		srand(time(NULL));
		size_t szWindowName = 128 + rand() % 128;
		WindowName = reinterpret_cast<char*>(malloc(szWindowName));
		if (!WindowName)
			return (bOpen = false);
		for (int i = 0; i < szWindowName - 1; i++) {
			WindowName[i] = 1 + rand() % 254;
		}
		WindowName[szWindowName - 1] = 0;
	}

	// Register class
	WNDCLASSEXA WindowClass = { 0 };
	WindowClass.cbSize = sizeof(WNDCLASSEXA);
	WindowClass.lpfnWndProc = (WNDPROC)WndProc;
	WindowClass.hInstance = GetModuleHandle(NULL);
	WindowClass.style = CS_CLASSDC;
	WindowClass.lpszClassName = WindowName;
	if (!RegisterClassExA(&WindowClass)) {
		if (Params & WindowParams::RandomClassName)
			free(WindowName);
		return (bOpen = false);
	}

	// Create window
	if (!(hWnd = CreateWindowExA(
		0,
		WindowName,
		WindowName,
		0,
		x,
		y,
		width,
		height,
		NULL,
		NULL,
		GetModuleHandleA(NULL),
		NULL
	))) {
		UnregisterClassA(WindowName, GetModuleHandleA(NULL));
		if (Params & WindowParams::RandomClassName)
			free(WindowName);
		return (bOpen = false);
	}
	SetWindowLongA(hWnd, GWL_STYLE, 0);
	ShowWindow(hWnd, SW_SHOW);

	// Setup ImGui
	CreateDeviceD3D(hWnd);
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);
	CleanupRenderTarget();
	g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	g_pSwapChain->GetContainingOutput(&g_pOutput);
	CreateRenderTarget();

	// Create rendering thread
	if (!CreateThread(0, 0, WindowThread, 0, 0, 0)) {
		ImGui_Window::End();
		UnregisterClassA(WindowName, GetModuleHandleA(NULL));
		if (Params & WindowParams::RandomClassName)
			free(WindowName);
		return false;
	}

	// Handle message queue
	MSG msg;
	while (bOpen) {
		while (bOpen && PeekMessage(&msg, hWnd, 0, 0, 1)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!ImGui::IsMouseDown(0)) {
			g_pOutput->WaitForVBlank();
		}
	}
	return true;
}

void ImGui_Window::Resize(int width, int height) {
	if (!bOpen)
		return;

	_ImGui_Windows_Reserved::width = width;
	_ImGui_Windows_Reserved::height = height;
	CleanupRenderTarget();
	g_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	CreateRenderTarget();
	SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
}

void ImGui_Window::Move(int x, int y, bool bRelative) {
	if (bRelative) {
		RECT WindowRect;
		GetWindowRect(hWnd, &WindowRect);
		x += WindowRect.left;
		y += WindowRect.top;
	}
	SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

bool ImGui_Window::IsOpen() {
	return bOpen;
}

HWND ImGui_Window::GetHandle() {
	return hWnd;
}

void ImGui_Window::End() {
	if (!bOpen)
		return;
	bOpen = false;
	pWindow = (ImGuiWindow*)1;
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	CleanupDeviceD3D();
}

WindowParams operator|=(WindowParams param1, WindowParams param2) {
	return (WindowParams)(param1 | param2);
}

// Window thread
DWORD WINAPI WindowThread(void* args) {
	while (bOpen) {
		// Setup frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (pWindow > (void*)1) {
			// Minimize window
			if (pWindow->Collapsed) {
				if (!bMinimized) {
					ShowWindow(hWnd, SW_MINIMIZE);
				}
				ImGui::SetWindowCollapsed(pWindow, false);
			}

			// Move/resize window
			else if (!bMinimized) {
				if (pWindow->Pos.x != 0 || pWindow->Pos.y != 0) {
					ImGui_Window::Move((int)pWindow->Pos.x, (int)pWindow->Pos.y, true);
				}
				if (pWindow->Size.x != width || pWindow->Size.y != height) {
					ImGui_Window::Resize((int)pWindow->Size.x, (int)pWindow->Size.y);
				}
			}
		} else if (pWindow == NULL) {
			bOpen = false;
			break;
		}

		// Render
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(width, height));
		pWindow = DrawCallback();

		// Finish frame
		ImGui::End();
		ImGui::Render();
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		g_pSwapChain->Present(1, 0);
	}
	ImGui_Window::End();
	return 0;
}

// Message handler
LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_QUIT:
	case WM_DESTROY:
	case WM_CLOSE:
		ImGui_Window::End();
		break;
	case WM_SHOWWINDOW:
		bMinimized = wParam == FALSE;
		break;
	}
	return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) ? TRUE : DefWindowProc(hWnd, uMsg, wParam, lParam);
}


/***** IMGUI PROVIDED DX11 FUNCTIONS *****/

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}