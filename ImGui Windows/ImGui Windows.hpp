#pragma once
#ifndef _IMGUI_WINDOWS
#define _IMGUI_WINDOWS

#include <stdint.h>
#include <Windows.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

enum WindowParams : uint8_t {
	Default = 0,
	RandomClassName = 1
};

//WindowParams operator|(WindowParams param1, WindowParams param2) {
//	return (WindowParams)((uint8_t)param1 | (uint8_t)param2);
//}
//
WindowParams operator|=(WindowParams param1, WindowParams param2);
//
//WindowParams operator&(WindowParams param1, WindowParams param2) {
//	return (WindowParams)((uint8_t)param1 & (uint8_t)param2);
//}


namespace ImGui_Window {

	/// <summary>
	/// Creates a new window.
	/// </summary>
	/// <param name="Name">Name of the ImGui window</param>
	/// <param name="x">Starting x position</param>
	/// <param name="y">Starting y position</param>
	/// <param name="width">Starting width</param>
	/// <param name="height">Starting height</param>
	/// <param name="Params">Window settings</param>
	/// <returns>True on new window created, if a window is already open it will return false</returns>
	bool Begin(char* Name, ImGuiWindow* (__stdcall* DrawCallback)(), int x, int y, int width, int height, WindowParams Params = WindowParams::Default);

	/// <summary>
	/// Changes the size of the window
	/// </summary>
	/// <param name="width">New width</param>
	/// <param name="height">New height</param>
	void Resize(int width, int height);

	/// <summary>
	/// Moves the window
	/// </summary>
	/// <param name="x">New x position</param>
	/// <param name="y">New y position</param>
	/// <param name="bRelative">Whether or not the new x & y positions are relative to the windows current position</param>
	void Move(int x, int y, bool bRelative = false);

	/// <summary>
	/// Checks whether or not the window is open
	/// </summary>
	/// <returns>True/false depending on window state</returns>
	bool IsOpen();

	/// <summary>
	/// Gets window handle
	/// </summary>
	/// <returns>NULL if no window exists, otherwise it returns the handle to the window</returns>
	HWND GetHandle();

	/// <summary>
	/// Closes window and cleans up ImGui
	/// </summary>
	void End();
}

#endif // _IMGUI_WINDOWS