#include "gui.hpp"
#include "../ui/ImGui/imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../ui/ImGui/imgui_internal.h"
#include "../ui/ImGui/imgui_impl_win32.h"
#include "../ui/ImGui/imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <codecvt>
#include <iomanip>
#include "../ui/ImGui/d3dx11tex_helper.h"
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#include "assets.hpp"
#include "../ui/ImGui/gui/gui.hpp"

#pragma comment(lib, "freetype64.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace ImGui;

// Global flags for Magic Bullet popup
static bool show_magic_bullet_popup = false;
static bool magic_bullet_pending_state = false;
static float popup_alpha = 0.0f;
static float yes_button_hover_alpha = 0.0f;
static float no_button_hover_alpha = 0.0f;

// Global flags for Miscellaneous popups
static bool show_no_recoil_popup = false;
static bool show_no_spread_popup = false;
static bool show_no_reload_popup = false;
static bool show_anti_headshot_popup = false;
static bool show_noclip_popup = false;
static float no_recoil_popup_alpha = 0.0f;
static float no_spread_popup_alpha = 0.0f;
static float no_reload_popup_alpha = 0.0f;
static float anti_headshot_popup_alpha = 0.0f;
static float noclip_popup_alpha = 0.0f;

// Hover animation states for Options buttons
static float teleport_hover_alpha = 0.0f;
static float heal_hover_alpha = 0.0f;
static float max_armor_hover_alpha = 0.0f;
static float fix_vehicle_hover_alpha = 0.0f;

// Hover animation states for Config buttons
static float save_config_hover_alpha = 0.0f;
static float load_config_hover_alpha = 0.0f;

// Hover animation states for Options buttons (Unload, Bypass)
static float unload_hover_alpha = 0.0f;
static float bypass_hover_alpha = 0.0f;

// Hover animation states for Noclip popup buttons
static float yes_button_hover_alpha_noclip = 0.0f;
static float no_button_hover_alpha_noclip = 0.0f;

void CustomStyleColor()
{
    ImGuiStyle& s = ImGui::GetStyle();
    ImGuiContext& g = *GImGui;
    
    s.Colors[ImGuiCol_PopupBg] = ImColor(11, 12, 13, 255);
    s.Colors[ImGuiCol_ChildBg] = ImColor(11,12,13,127);
    s.Colors[ImGuiCol_Border] = ImColor(52, 52, 52, 255);
    
    // Purple sliders - beautiful gradient
    s.Colors[ImGuiCol_SliderGrab] = ImColor(138, 43, 226, 200);
    s.Colors[ImGuiCol_SliderGrabActive] = ImColor(138, 43, 226, 255);
    
    // Purple checkboxes
    s.Colors[ImGuiCol_CheckMark] = ImColor(138, 43, 226, 255);
    s.Colors[ImGuiCol_FrameBg] = ImColor(18, 18, 20, 255);
    s.Colors[ImGuiCol_FrameBgHovered] = ImColor(30, 30, 35, 255);
    s.Colors[ImGuiCol_FrameBgActive] = ImColor(40, 40, 45, 255);
    
    // Purple buttons
    s.Colors[ImGuiCol_Button] = ImColor(138, 43, 226, 100);
    s.Colors[ImGuiCol_ButtonHovered] = ImColor(138, 43, 226, 150);
    s.Colors[ImGuiCol_ButtonActive] = ImColor(138, 43, 226, 200);
    
    // Purple header
    s.Colors[ImGuiCol_Header] = ImColor(138, 43, 226, 80);
    s.Colors[ImGuiCol_HeaderHovered] = ImColor(138, 43, 226, 120);
    s.Colors[ImGuiCol_HeaderActive] = ImColor(138, 43, 226, 170);
    
    s.ChildRounding = 10.f;
	s.WindowRounding = 0.f;
    s.WindowPadding = ImVec2(0, 0);
    s.FrameRounding = 3.f;
    s.GrabRounding = 3.f;
}

int sub_tabs[4] = {0, 0, 0, 0};

namespace FrameWork
{
	void Interface::Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device)
	{
		hWindow = Window;
		hTargetWindow = TargetWindow;
		IDevice = Device;

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGui_ImplWin32_Init(Window);
		ID3D11DeviceContext* pContext = nullptr;
		Device->GetImmediateContext(&pContext);
		if (pContext)
		{
			ImGui_ImplDX11_Init(Device, pContext);
			// Note: GetImmediateContext doesn't increment ref count, so don't Release
		}

		// Initialize new GUI system
		try {
			GUI::get().initialize(Device);
		} catch (...) {
			// Ignore if GUI initialization fails
		}

		if (bIsMenuOpen)
		{
			SetWindowLong(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
			SetForegroundWindow(hWindow);
		}
		else
		{
			SetWindowLong(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE);
		}
	}

	void Interface::UpdateStyle()
	{
	}

	void Interface::RenderGui()
	{
		if (!bIsMenuOpen)
			return;

		// Use new GUI system - draw() will render all tabs, checkboxes, etc.
		try {
			GUI::get().draw();
		} catch (...) {
			// Ignore if GUI draw fails
		}
	}

	LRESULT Interface::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (bIsMenuOpen && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;
		return false;
	}

	void Interface::HandleMenuKey()
	{
		static bool MenuKeyDown = false;
		if (GetAsyncKeyState(VK_INSERT) & 0x8000)
		{
			if (!MenuKeyDown)
			{
				MenuKeyDown = true;
				bIsMenuOpen = !bIsMenuOpen;
				
				if (bIsMenuOpen)
				{
					SetWindowLong(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
					SetForegroundWindow(hWindow);
				}
				else
				{
					SetWindowLong(hWindow, GWL_EXSTYLE, WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE);
				}
			}
		}
		else
		{
			MenuKeyDown = false;
		}
	}

	void Interface::ShutDown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
}
