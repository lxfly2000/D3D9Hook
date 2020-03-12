#pragma once
#include<Windows.h>
#include<d3d9.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

LRESULT CALLBACK KeyOverlayExtraProcess(WNDPROC oldProc, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL KeyOverlayInit(HWND hwnd, LPDIRECT3DDEVICE9 pDevice);
void KeyOverlayUninit();
void KeyOverlayDraw();
