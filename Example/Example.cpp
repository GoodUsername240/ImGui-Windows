#include <Windows.h>
#include "ImGui Windows.hpp"
#pragma comment(lib, "ImGui Windows.lib")

WindowParams Params = WindowParams::Default;

bool bOpen = true;

ImGuiWindow* Draw() {
    // End if window was closed
    if (!bOpen) {
        ImGui_Window::End();
        return NULL;
    }

    // Without passing &bOpen to ImGui there will be no 'x' in the title bar, forcing the user to close it from the task bar
    ImGui::Begin("Example", &bOpen);
    ImGui::Text("Hello World!");
    ImGui::Button("Button");
    ImGui::End();
    return ImGui::FindWindowByName("Example");
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

#if 0 
    Params = (WindowParams)(Params | WindowParams::RandomClassName);
#endif

    // You are required to set up the ImGui context on your own, but do not initialize any of the backends
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = NULL;

    // ImGui_Window::Begin is blocking, and will return once the window is closed
    if (!ImGui_Window::Begin(const_cast<char*>("Example"), (DRAWCALLBACK)Draw, CW_USEDEFAULT, CW_USEDEFAULT, 550, 680, Params)) {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}