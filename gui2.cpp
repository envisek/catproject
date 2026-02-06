#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "../imgui.h"
#include "../imgui_internal.h"
#include "../imgui_impl_dx11.h"
#include "../imgui_impl_win32.h"
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <d3d11.h>
#include "../d3dx11tex_helper.h"
#include <functional>
#include "thirdparty/include/animations.hpp"
#include "managers/FontManager.hpp"
#include "managers/ImageManager.hpp"
#include "compbuilder/CompBuilder.hpp"
#include "managers/WidgetsManager.hpp"
#include "managers/StyleManager.hpp"
#include "fonts.h"
#include "unicodes.hpp"
#include "managers/TabsManager.hpp"
#include "managers/ChildManager.hpp"
#include "managers/PopupManager.hpp"
#include "managers/LangManager.hpp"
#include "managers/SearchManager.hpp"
#include "../../../../ishowspeed/options.hpp"
#include "../../../../ishowspeed/configs.hpp"
#include "../../../../ishowspeed/sdk/fivem.hpp"

#include "gui.hpp"

#pragma comment(lib, "freetype64.lib")

using namespace ImGui;

#include "managers/NotifyManager.hpp"

// Global configuration manager to provide defaults and persistence
static Cheat::ConfigManager g_ConfigManager;

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

void GUI::initialize( ID3D11Device* device ) {
	size = ImVec2{ 750, 500 };

	GImGui->IO.IniFilename = "";

	StyleManager::get( ).Styles( );
	StyleManager::get( ).Colors( );
	fonts.resize( fonts_size );

	fonts.at( font ).setup( b_font, sizeof( b_font ),
		{ 14, 12 },
		GetIO( ).Fonts->GetGlyphRangesCyrillic( ) );

	const static ImWchar icons_ranges[] = { 0x1 + 16000, 0x1 + 17170, 0 };
	fonts.at( icons ).setup( glyphter, sizeof( glyphter ),
		{ 14, 12 },
		icons_ranges );

	LangManager::get( ).initialize( );

	ChildManager& cm = ChildManager::get( );
	auto& widgets = WidgetsManager::get( );

	// Setup() is called in ConfigManager constructor, which sets all default values
	// Then try to load persisted configuration (which will overwrite defaults if file exists and is valid)
	bool configLoaded = g_ConfigManager.LoadConfig();
	
	// If config file doesn't exist or is invalid, ensure defaults are set
	// This is a safety measure in case Setup() wasn't called properly or values were reset
	if (!configLoaded) {
		// Config file doesn't exist or is invalid, so ensure defaults from configs.cpp are set
		// Setup() was already called in constructor, but call it again to be safe
		g_ConfigManager.Setup();
	}

	static bool bools[100];
	static int ints[100];
	static float floats[100];
	static bool multicombo_values[5];
	static float col[100][4];
	static char buf[16];

	// AimAssistance -> Silent subtab (index 0, subtab 0)
	TabsManager::get( ).add_page( 0, [&]( ) {
		BeginGroup( );
		{
			cm.beginchild( "Silent" );
			{
				widgets.Checkbox( "Enable", &g_Options.LegitBot.SilentAim.Enabled);
				SameLine(GetWindowWidth() - 70);
				SetCursorPosY(GetCursorPosY() - 2.0f);
				widgets.Binder("##SilentEnableKey", &g_Options.LegitBot.SilentAim.KeyBind);
				SetCursorPosY(GetCursorPosY() + 2.0f);
				widgets.SliderInt("Field of View", &g_Options.LegitBot.SilentAim.Fov, 0, 300, "%d");
				widgets.SliderInt("Miss Chance", &g_Options.LegitBot.SilentAim.MissChance, 0, 100, "%d%%");
				widgets.SliderInt("Max Distance", &g_Options.LegitBot.SilentAim.MaxDistance, 0, 500, "%dm");
				
				// Bones label and combobox
				float bones_text_y = GetCursorPosY();
				Text("Bones");
				float text_height = GetTextLineHeight();
				SetCursorPosY(bones_text_y + text_height - 10.0f);
				std::vector<const char*> bone_items = { "Head", "Neck", "Chest", "Stomach", "Pelvis" };
				widgets.Combo("##Bones", &g_Options.LegitBot.SilentAim.HitBox, bone_items);
			}
			cm.endchild( );
		}
		EndGroup( );

		SameLine( );
		
		BeginGroup( );
		{
			cm.beginchild("Miscellaneous");
			{
				widgets.Checkbox("Draw FOV", &g_Options.Misc.Screen.ShowSilentAimFov);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##FOVColor", g_Options.Misc.Screen.SilentFovColor);
				
				widgets.Checkbox("Dynamic Field of view", &g_Options.LegitBot.SilentAim.DynamicFOV);
				widgets.Checkbox("Visible only", &g_Options.LegitBot.SilentAim.VisibleCheck);
				widgets.Checkbox("Shot NPC", &g_Options.LegitBot.SilentAim.ShotNPC);
			}
			cm.endchild();
			
			cm.beginchild("Settings");
			{
				auto window_pos = GetWindowPos();
				auto cursor_y = GetCursorPosY();
				ImColor bright_red_color(255, 80, 80, 255);
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_mb = g_Options.LegitBot.SilentAim.MagicBullet;
				if (widgets.Checkbox("Magic Bullet", &current_mb)) {
					if (current_mb && !g_Options.LegitBot.SilentAim.MagicBullet) {
						// User wants to enable - show popup
						show_magic_bullet_popup = true;
						magic_bullet_pending_state = true;
						popup_alpha = 0.0f;
					} else if (!current_mb) {
						// User wants to disable - do it immediately
						g_Options.LegitBot.SilentAim.MagicBullet = false;
					}
				}
				widgets.Checkbox("Predict", &g_Options.LegitBot.SilentAim.Prediction);
				widgets.Checkbox("Force Driver", &g_Options.LegitBot.SilentAim.ForceDriver);
				widgets.Checkbox("AC Bypass", &g_Options.LegitBot.SilentAim.ACBypass);
				widgets.Checkbox("Exclude Dead", &g_Options.LegitBot.SilentAim.ExcludeDead);
				widgets.Checkbox("Exclude God", &g_Options.LegitBot.SilentAim.ExcludeGod);
			}
			cm.endchild();
		}
		EndGroup( );
	} );

	TabsManager::get( ).add_page( 0, [&]( ) {
		BeginGroup( );
		{
			// First child - TriggerBot
			cm.beginchild( "TriggerBot" );
			{
				widgets.Checkbox( "Enable", &g_Options.LegitBot.Trigger.Enabled);
				SameLine(GetWindowWidth() - 70);
				SetCursorPosY(GetCursorPosY() - 2.0f);
				widgets.Binder("##TriggerbotEnableKey", &g_Options.LegitBot.Trigger.KeyBind);
				SetCursorPosY(GetCursorPosY() + 2.0f);
				
				// Type label and combobox
				float type_text_y = GetCursorPosY();
				Text("Type");
				float text_height = GetTextLineHeight();
				SetCursorPosY(type_text_y + text_height - 10.0f);
				std::vector<const char*> type_items = { "First Person", "Third Person" };
				widgets.Combo("##Type", &g_Options.LegitBot.Trigger.Type, type_items);
				
				// Field of view slider
				widgets.SliderInt("Field of view", &g_Options.LegitBot.Trigger.FOV, 0, 300, "%d");
			}
			cm.endchild( );
		}
		EndGroup( );

		SameLine( );
		
		BeginGroup( );
		{
			// Second child - Options
			cm.beginchild("Options");
			{
				widgets.Checkbox("Draw FOV", &g_Options.Misc.Screen.ShowTriggerFov);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##TriggerFOVColor", g_Options.Misc.Screen.TriggerFovColor);
				
				widgets.Checkbox("Exclude Dead", &g_Options.LegitBot.Trigger.ExcludeDead);
				widgets.Checkbox("Shot NPC", &g_Options.LegitBot.Trigger.ShotNPC);
				widgets.Checkbox("Visible Only", &g_Options.LegitBot.Trigger.VisibleCheck);
				
				widgets.SliderInt("Max Distance", &g_Options.LegitBot.Trigger.MaxDistance, 0, 500, "%dm");
				widgets.SliderInt("Shot Delay", &g_Options.LegitBot.Trigger.ShotDelay, 0, 100, "%d ms");
				widgets.SliderInt("Reaction Time", &g_Options.LegitBot.Trigger.ReactionTime, 0, 100, "%d ms");
			}
			cm.endchild();
		}
		EndGroup( );
	} );

	// AimAssistance -> Aimbot subtab (index 0, subtab 2)
	TabsManager::get( ).add_page( 0, [&]( ) {
		BeginGroup( );
		{
			// First child - Aimbot
			cm.beginchild( "Aimbot" );
			{
				widgets.Checkbox( "Enable", &g_Options.LegitBot.AimBot.Enabled);
				SameLine(GetWindowWidth() - 70);
				SetCursorPosY(GetCursorPosY() - 2.0f);
				widgets.Binder("##AimbotKey", &g_Options.LegitBot.AimBot.KeyBind);
				SetCursorPosY(GetCursorPosY() + 2.0f);
				
				// Draw FOV with color picker
				widgets.Checkbox("Draw FOV", &g_Options.Misc.Screen.ShowAimbotFov);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##AimbotFOVColor", g_Options.Misc.Screen.AimbotFovColor);
				
				widgets.Checkbox("Target NPC", &g_Options.LegitBot.AimBot.TargetNPC);
				widgets.Checkbox("Visible Check", &g_Options.LegitBot.AimBot.VisibleCheck);
				widgets.Checkbox("Exclude Dead", &g_Options.LegitBot.AimBot.ExcludeDead);
			}
			cm.endchild( );
		}
		EndGroup( );

		SameLine( );
		
		BeginGroup( );
		{
			// Second child - Options
			cm.beginchild("Options");
			{
				// Bones combobox (same as Silent)
				float bones_text_y = GetCursorPosY();
				Text("Bones");
				float text_height = GetTextLineHeight();
				SetCursorPosY(bones_text_y + text_height - 10.0f);
				std::vector<const char*> bone_items = { "Head", "Neck", "Chest", "Stomach", "Pelvis" };
				widgets.Combo("##AimbotBones", &g_Options.LegitBot.AimBot.HitBox, bone_items);
				
				// Sliders
				widgets.SliderInt("Field of view", &g_Options.LegitBot.AimBot.FOV, 0, 300, "%d");
				widgets.SliderInt("Max Distance", &g_Options.LegitBot.AimBot.MaxDistance, 0, 500, "%dm");
				widgets.SliderInt("Smooth X", &g_Options.LegitBot.AimBot.SmoothHorizontal, 0, 100, "%d");
				widgets.SliderInt("Smooth Y", &g_Options.LegitBot.AimBot.SmoothVertical, 0, 100, "%d");
			}
			cm.endchild();
		}
		EndGroup( );
	} );

	TabsManager::get().add_page(1, [&]() {
		BeginGroup();
		{
			cm.beginchild("Visualization");
			{
				widgets.Checkbox("Enable", &g_Options.Visuals.ESP.Players.Enabled);
				widgets.SliderInt("Max Distance", &g_Options.Visuals.ESP.Players.RenderDistance, 0, 300, "%dm");
				widgets.Checkbox("Filter LocalPlayer", &g_Options.Visuals.ESP.Players.ShowLocalPlayer);
				widgets.Checkbox("Filter NPC", &g_Options.Visuals.ESP.Players.ShowNPCs);
				widgets.Checkbox("Visible Only", &g_Options.Visuals.ESP.Players.VisibleOnly);
				widgets.Checkbox("Exclude Dead", &g_Options.Visuals.ESP.Players.ExcludeDead);
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			cm.beginchild("Options");
			{
				widgets.Checkbox("Box", &g_Options.Visuals.ESP.Players.Box);
				
				widgets.Checkbox("Skeleton", &g_Options.Visuals.ESP.Players.Skeleton);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##SkeletonColor", g_Options.Visuals.ESP.Players.SkeletonColor);
				
				widgets.Checkbox("Name", &g_Options.Visuals.ESP.Players.Name);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##NameColor", g_Options.Visuals.ESP.Players.NameColor);
				
				widgets.Checkbox("ID", &g_Options.Visuals.ESP.Players.ID);
				widgets.Checkbox("Health Bar", &g_Options.Visuals.ESP.Players.HealthBar);
				widgets.Checkbox("Armor Bar", &g_Options.Visuals.ESP.Players.ArmorBar);
				
				widgets.Checkbox("Weapon Name", &g_Options.Visuals.ESP.Players.WeaponName);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##WeaponNameColor", g_Options.Visuals.ESP.Players.WeaponNameColor);
				
				widgets.Checkbox("Distance", &g_Options.Visuals.ESP.Players.Distance);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##DistanceColor", g_Options.Visuals.ESP.Players.DistanceColor);
				
				widgets.Checkbox("Invincible", &g_Options.Visuals.ESP.Players.Invincible);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##InvincibleColor", g_Options.Visuals.ESP.Players.InvincibleColor);
				
				widgets.Checkbox("Snaplines", &g_Options.Visuals.ESP.Players.SnapLines);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##SnaplineColor", g_Options.Visuals.ESP.Players.SnaplinesColor);
			}
			cm.endchild();
		}
		EndGroup();
		});

	// Miscellaneous tab (index 2)
	TabsManager::get().add_page(2, [&]() {
		BeginGroup();
		{
			// First child - Settings (left side)
			cm.beginchild("Settings");
			{
				// God Mode (without warning)
				widgets.Checkbox("God Mode", &g_Options.Misc.Exploits.LocalPlayer.God);
				
				// No Recoil (with warning)
				auto window_pos = GetWindowPos();
				auto cursor_y = GetCursorPosY();
				ImColor bright_red_color(255, 80, 80, 255);
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, 
					window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, 
					bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_no_recoil = g_Options.Misc.Exploits.Weapon.NoRecoil;
				if (widgets.Checkbox("No Recoil", &current_no_recoil)) {
					if (current_no_recoil && !g_Options.Misc.Exploits.Weapon.NoRecoil) {
						show_no_recoil_popup = true;
						no_recoil_popup_alpha = 0.0f;
					} else if (!current_no_recoil) {
						g_Options.Misc.Exploits.Weapon.NoRecoil = false;
					}
				}
				
				// No Spread (with warning)
				cursor_y = GetCursorPosY();
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, 
					window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, 
					bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_no_spread = g_Options.Misc.Exploits.Weapon.NoSpread;
				if (widgets.Checkbox("No Spread", &current_no_spread)) {
					if (current_no_spread && !g_Options.Misc.Exploits.Weapon.NoSpread) {
						show_no_spread_popup = true;
						no_spread_popup_alpha = 0.0f;
					} else if (!current_no_spread) {
						g_Options.Misc.Exploits.Weapon.NoSpread = false;
					}
				}
				
				// No Reload (with warning)
				cursor_y = GetCursorPosY();
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, 
					window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, 
					bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_no_reload = g_Options.Misc.Exploits.Weapon.NoReload;
				if (widgets.Checkbox("No Reload", &current_no_reload)) {
					if (current_no_reload && !g_Options.Misc.Exploits.Weapon.NoReload) {
						show_no_reload_popup = true;
						no_reload_popup_alpha = 0.0f;
					} else if (!current_no_reload) {
						g_Options.Misc.Exploits.Weapon.NoReload = false;
					}
				}
				
				// Anti Headshot (with warning)
				cursor_y = GetCursorPosY();
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, 
					window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, 
					bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_anti_headshot = g_Options.Misc.Exploits.LocalPlayer.AntiHeadshot;
				if (widgets.Checkbox("Anti Headshot", &current_anti_headshot)) {
					if (current_anti_headshot && !g_Options.Misc.Exploits.LocalPlayer.AntiHeadshot) {
						show_anti_headshot_popup = true;
						anti_headshot_popup_alpha = 0.0f;
					} else if (!current_anti_headshot) {
						g_Options.Misc.Exploits.LocalPlayer.AntiHeadshot = false;
					}
				}
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			// Second child - Noclip (right side)
			cm.beginchild("Noclip");
			{
				// Noclip checkbox (with warning)
				auto window_pos = GetWindowPos();
				auto cursor_y = GetCursorPosY();
				ImColor bright_red_color(255, 80, 80, 255);
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, 
					window_pos + ImVec2{GImGui->Style.WindowPadding.x + 1, cursor_y + 2}, 
					bright_red_color, i_alert_triangle);
				
				SetCursorPosX(GImGui->Style.WindowPadding.x + 26);
				bool current_noclip = g_Options.Misc.Exploits.LocalPlayer.Noclip;
				if (widgets.Checkbox("Noclip", &current_noclip)) {
					if (current_noclip && !g_Options.Misc.Exploits.LocalPlayer.Noclip) {
						show_noclip_popup = true;
						noclip_popup_alpha = 0.0f;
					} else if (!current_noclip) {
						g_Options.Misc.Exploits.LocalPlayer.Noclip = false;
					}
				}
				SameLine(GetWindowWidth() - 70);
				SetCursorPosY(GetCursorPosY() - 2.0f);
				widgets.Binder("##NoclipKey", &g_Options.Misc.Exploits.LocalPlayer.NoclipBind);
				SetCursorPosY(GetCursorPosY() + 2.0f);
				
				// Speed slider
				widgets.SliderInt("Speed", &g_Options.Misc.Exploits.LocalPlayer.NoclipSpeed, 0, 100, "%d m/s");
			}
			cm.endchild();
			
			// Third child - Options (right side, under Noclip)
			cm.beginchild("Options");
			{
				// Draw custom buttons with hover effects
				auto draw_list = GetWindowDrawList();
				
				// Button configuration
				ImVec2 button_size(230, 40);
				ImColor base_color(94, 61, 220, 255); // #5e3ddc
				ImColor hover_color(88, 55, 200, 255); // #5837c8
				
				// Teleport to Waypoint button
				PushID("teleport_waypoint");
				InvisibleButton("##teleport", button_size);
				bool teleport_hovered = IsItemHovered();
				bool teleport_clicked = IsItemClicked();
				ImVec2 teleport_min = GetItemRectMin();
				ImVec2 teleport_max = GetItemRectMax();
				PopID();
				
				teleport_hover_alpha = ImLerp(teleport_hover_alpha, teleport_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 teleport_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, teleport_hover_alpha);
				ImColor teleport_final_color(teleport_final_vec);
				
				draw_list->AddRectFilled(teleport_min, teleport_max, teleport_final_color, GImGui->Style.FrameRounding);
				ImVec2 teleport_text_size = CalcTextSize("Teleport to waypoint");
				draw_list->AddText(teleport_min + (button_size - teleport_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Teleport to waypoint");
				
				if (teleport_clicked) {
					g_Options.Misc.Exploits.LocalPlayer.TriggerTeleport = true;
				}
				
				// Heal button
				PushID("heal_button");
				InvisibleButton("##heal", button_size);
				bool heal_hovered = IsItemHovered();
				bool heal_clicked = IsItemClicked();
				ImVec2 heal_min = GetItemRectMin();
				ImVec2 heal_max = GetItemRectMax();
				PopID();
				
				heal_hover_alpha = ImLerp(heal_hover_alpha, heal_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 heal_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, heal_hover_alpha);
				ImColor heal_final_color(heal_final_vec);
				
				draw_list->AddRectFilled(heal_min, heal_max, heal_final_color, GImGui->Style.FrameRounding);
				ImVec2 heal_text_size = CalcTextSize("Heal");
				draw_list->AddText(heal_min + (button_size - heal_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Heal");
				
				if (heal_clicked) {
					g_Options.Misc.Exploits.LocalPlayer.TriggerHeal = true;
				}
				
				// Max Armor button
				PushID("max_armor_button");
				InvisibleButton("##max_armor", button_size);
				bool max_armor_hovered = IsItemHovered();
				bool max_armor_clicked = IsItemClicked();
				ImVec2 max_armor_min = GetItemRectMin();
				ImVec2 max_armor_max = GetItemRectMax();
				PopID();
				
				max_armor_hover_alpha = ImLerp(max_armor_hover_alpha, max_armor_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 max_armor_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, max_armor_hover_alpha);
				ImColor max_armor_final_color(max_armor_final_vec);
				
				draw_list->AddRectFilled(max_armor_min, max_armor_max, max_armor_final_color, GImGui->Style.FrameRounding);
				ImVec2 max_armor_text_size = CalcTextSize("Max Armor");
				draw_list->AddText(max_armor_min + (button_size - max_armor_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Max Armor");
				
				if (max_armor_clicked) {
					g_Options.Misc.Exploits.LocalPlayer.TriggerArmor = true;
				}
				
				// Fix Vehicle button
				PushID("fix_vehicle_button");
				InvisibleButton("##fix_vehicle", button_size);
				bool fix_vehicle_hovered = IsItemHovered();
				bool fix_vehicle_clicked = IsItemClicked();
				ImVec2 fix_vehicle_min = GetItemRectMin();
				ImVec2 fix_vehicle_max = GetItemRectMax();
				PopID();
				
				fix_vehicle_hover_alpha = ImLerp(fix_vehicle_hover_alpha, fix_vehicle_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 fix_vehicle_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, fix_vehicle_hover_alpha);
				ImColor fix_vehicle_final_color(fix_vehicle_final_vec);
				
				draw_list->AddRectFilled(fix_vehicle_min, fix_vehicle_max, fix_vehicle_final_color, GImGui->Style.FrameRounding);
				ImVec2 fix_vehicle_text_size = CalcTextSize("Fix Vehicle");
				draw_list->AddText(fix_vehicle_min + (button_size - fix_vehicle_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Fix Vehicle");
				
				if (fix_vehicle_clicked) {
					g_Options.Misc.Exploits.LocalPlayer.RepairVehicle = true;
				}
			}
			cm.endchild();
		}
		EndGroup();
	});

	// Players tab (index 3)
	TabsManager::get().add_page(3, [&]() {
		static char player_search[64] = "";

		BeginGroup();
		{
			cm.beginchild("Players");
			{
				auto& player_list_state = g_Options.Misc.PlayerList;
				const auto local_info = Cheat::g_Fivem.GetLocalPlayerInfo();
				const auto entities = Cheat::g_Fivem.GetEntitiyListSafe();

				struct PlayerEntry {
					int net_id;
					std::string name;
					float distance;
				};

				std::vector<PlayerEntry> available_players;
				available_players.reserve(entities.size());

				for (const auto& entity : entities) {
					const auto& info = entity.StaticInfo;
					if (info.bIsNPC || info.bIsLocalPlayer)
						continue;

					PlayerEntry entry{};
					entry.net_id = info.NetId;
					if (info.NetId != -1) {
						entry.name = Cheat::g_Fivem.GetPlayerNameByNetId(info.NetId);
					}
					if (entry.name.empty()) {
						entry.name = !info.Name.empty() ? info.Name : std::to_string(info.NetId);
					}
					entry.distance = (local_info.Ped != nullptr)
						? entity.Cordinates.DistTo(local_info.WorldPos)
						: 0.0f;

					available_players.emplace_back(std::move(entry));
				}

				std::sort(available_players.begin(), available_players.end(), [](const PlayerEntry& a, const PlayerEntry& b) {
					return a.distance < b.distance;
				});

				std::string search_text = player_search;
				std::string search_lower = search_text;
				std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
				});

				bool selection_valid = false;

				// Make the list clearly visible and bordered; compact paddings like in the reference
				PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 6.f));
				PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 6.f));
				// Bigger search field: full width of the child, height ~40 (via padding override)
				PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 12.f));
				SetNextItemWidth(GetContentRegionAvail().x);
				InputText("##PlayerSearch", player_search, IM_ARRAYSIZE(player_search));
				PopStyleVar(); // revert big padding for other controls

				// Slightly smaller list height per request
				BeginChild("PlayerList", ImVec2(0, 260.f), true);
				{
					// Increase font scale slightly for better readability
					SetWindowFontScale(1.08f); // 8% larger text - more readable but not too big
					
					// Increase padding for better clickability while keeping text clear
					PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 8.f));
					PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 8.f));
					
					bool any_displayed = false;
					for (const auto& player : available_players) {
						std::string name_lower = player.name;
						std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) {
							return static_cast<char>(std::tolower(c));
						});

						if (!search_lower.empty() && name_lower.find(search_lower) == std::string::npos)
							continue;

						const int distance_m = static_cast<int>(std::round(player.distance));
						std::string label = player.name + " (" + std::to_string(distance_m) + "m)";
						const bool is_selected = (player_list_state.SelectedPlayerNetId == player.net_id);

						PushID(player.net_id);
						// Use Selectable - check both return value and IsItemClicked to ensure clicking works
						bool selectable_clicked = Selectable(label.c_str(), is_selected);
						bool item_clicked = IsItemClicked(0); // Check for left mouse button click
						
						// Update selection if Selectable was clicked OR if item was clicked
						if (selectable_clicked || item_clicked) {
							player_list_state.SelectedPlayerNetId = player.net_id;
							player_list_state.SelectedPlayerName = player.name;
							player_list_state.SelectedPlayerDistance = player.distance;
							selection_valid = true;
						}
						PopID();

						// Update info for currently selected player
						if (is_selected) {
							player_list_state.SelectedPlayerName = player.name;
							player_list_state.SelectedPlayerDistance = player.distance;
							selection_valid = true;
						}

						any_displayed = true;
					}
					
					PopStyleVar(2); // Restore style vars
					SetWindowFontScale(1.0f); // Restore font scale

					if (!any_displayed) {
						TextDisabled("No players found");
					}

					if (!selection_valid && player_list_state.SelectedPlayerNetId != -1) {
						player_list_state.SelectedPlayerNetId = -1;
						player_list_state.SelectedPlayerName.clear();
						player_list_state.SelectedPlayerDistance = 0.0f;
					}
				}
				EndChild();
				PopStyleVar(2);
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			cm.beginchild("Selected Player Information");
			{
				auto& player_list_state = g_Options.Misc.PlayerList;
				if (player_list_state.SelectedPlayerNetId == -1) {
					Text("Select a player first!");
				} else {
					const auto entities = Cheat::g_Fivem.GetEntitiyListSafe();
					const auto local_info = Cheat::g_Fivem.GetLocalPlayerInfo();

					bool is_friend = Cheat::g_Fivem.IsFriendByNetId(player_list_state.SelectedPlayerNetId);
					bool god_mode = false;
					std::string weapon_name = "-";
					int distance_m = static_cast<int>(std::round(player_list_state.SelectedPlayerDistance));

					std::string display_name = Cheat::g_Fivem.GetPlayerNameByNetId(player_list_state.SelectedPlayerNetId);
					if (display_name.empty()) {
						display_name = player_list_state.SelectedPlayerName;
					}
					if (display_name.empty()) {
						display_name = std::to_string(player_list_state.SelectedPlayerNetId);
					}
					
					for (const auto& e : entities) {
						if (e.StaticInfo.NetId == player_list_state.SelectedPlayerNetId && !e.StaticInfo.bIsNPC) {
							if (local_info.Ped != nullptr) {
								float d = e.Cordinates.DistTo(local_info.WorldPos);
								player_list_state.SelectedPlayerDistance = d;
								distance_m = static_cast<int>(std::round(d));
							}
							if (e.StaticInfo.Ped) {
								god_mode = e.StaticInfo.Ped->IsGodMode();
								if (auto wm = e.StaticInfo.Ped->GetWeaponManager()) {
									if (auto wi = wm->GetWeaponInfo()) {
										weapon_name = wi->GetWeaponName();
									}
								}
							}
							if (player_list_state.SelectedPlayerNetId != -1) {
								std::string resolved = Cheat::g_Fivem.GetPlayerNameByNetId(player_list_state.SelectedPlayerNetId);
								if (!resolved.empty()) {
									display_name = resolved;
									player_list_state.SelectedPlayerName = resolved;
								}
							}
							break;
						}
					}

					Text("Name:      %s", display_name.c_str());
					Text("Friend:    %s", is_friend ? "True" : "False");
					Text("Net ID:    %d", player_list_state.SelectedPlayerNetId);
					Text("Distance:  %dm", distance_m);
					Text("GodMode:   %s", god_mode ? "True" : "False");
					Text("Weapon:    %s", weapon_name.c_str());
				}
			}
			cm.endchild();

			cm.beginchild("Actions");
			{
				auto& player_list_state = g_Options.Misc.PlayerList;
				if (player_list_state.SelectedPlayerNetId == -1) {
					Text("Select a player first!");
				} else {
					bool is_friend = Cheat::g_Fivem.IsFriendByNetId(player_list_state.SelectedPlayerNetId);
					// Draw custom buttons with hover effects (same style as Settings)
					auto draw_list = GetWindowDrawList();
					
					// Button configuration
					ImVec2 button_size(230, 40);
					ImColor base_color(94, 61, 220, 255); // #5e3ddc
					ImColor hover_color(88, 55, 200, 255); // #5837c8
					
					// Teleport to button
					static float teleport_player_hover_alpha = 0.0f;
					PushID("teleport_player");
					InvisibleButton("##teleport_player", button_size);
					bool teleport_player_hovered = IsItemHovered();
					bool teleport_player_clicked = IsItemClicked();
					ImVec2 teleport_player_min = GetItemRectMin();
					ImVec2 teleport_player_max = GetItemRectMax();
					PopID();
					
					teleport_player_hover_alpha = ImLerp(teleport_player_hover_alpha, teleport_player_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 teleport_player_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, teleport_player_hover_alpha);
					ImColor teleport_player_final_color(teleport_player_final_vec);
					
					draw_list->AddRectFilled(teleport_player_min, teleport_player_max, teleport_player_final_color, GImGui->Style.FrameRounding);
					ImVec2 teleport_player_text_size = CalcTextSize("Teleport to");
					draw_list->AddText(teleport_player_min + (button_size - teleport_player_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Teleport to");
					
					if (teleport_player_clicked) {
						g_Options.Misc.PlayerList.TriggerTeleportToPlayer = true;
					}
					
					// Add Friend button
					static float add_friend_hover_alpha = 0.0f;
					PushID("add_friend");
					InvisibleButton("##add_friend", button_size);
					bool add_friend_hovered = IsItemHovered();
					bool add_friend_clicked = IsItemClicked();
					ImVec2 add_friend_min = GetItemRectMin();
					ImVec2 add_friend_max = GetItemRectMax();
					PopID();
					
					add_friend_hover_alpha = ImLerp(add_friend_hover_alpha, add_friend_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 add_friend_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, add_friend_hover_alpha);
					ImColor add_friend_final_color(add_friend_final_vec);
					
					draw_list->AddRectFilled(add_friend_min, add_friend_max, add_friend_final_color, GImGui->Style.FrameRounding);
					const char* friend_label = is_friend ? "Remove Friend" : "Add Friend";
					ImVec2 add_friend_text_size = CalcTextSize(friend_label);
					draw_list->AddText(add_friend_min + (button_size - add_friend_text_size) * 0.5f, ImColor(0, 0, 0, 255), friend_label);
					
					if (add_friend_clicked) {
						g_Options.Misc.PlayerList.TriggerAddFriend = true;
					}
				}
			}
			cm.endchild();
		}
		EndGroup();
	});

	// Vehicles tab (index 4)
	TabsManager::get().add_page(4, [&]() {
		static char vehicle_search[64] = "";
		static int selected_vehicle_index = -1; // CWorld index

		BeginGroup();
		{
			cm.beginchild("Vehicles");
			{
				const auto local_info = Cheat::g_Fivem.GetLocalPlayerInfo();
				const auto vehicles = Cheat::g_Fivem.GetVehicleList();

				struct VehicleEntry {
					int index;
					std::string name;
					float distance;
				};

				std::vector<VehicleEntry> nearby;
				nearby.reserve(vehicles.size());
				for (const auto& v : vehicles) {
					if (!v.Vehicle) continue;
					
					Vector3D pos = v.Vehicle->GetCoordinate();
					float dist = (local_info.Ped != nullptr) ? pos.DistTo(local_info.WorldPos) : 0.0f;
					
					// Use vehicle name, or fallback to "Vehicle" + index if name is empty
					std::string vehicle_name = v.Name.empty() 
						? (v.iIndex >= 0 ? "Vehicle " + std::to_string(v.iIndex) : "Unknown Vehicle")
						: v.Name;
					
					nearby.push_back({ v.iIndex, vehicle_name, dist });
				}

				std::sort(nearby.begin(), nearby.end(), [](const VehicleEntry& a, const VehicleEntry& b) {
					return a.distance < b.distance;
				});

				std::string search_text = vehicle_search;
				std::string search_lower = search_text;
				std::transform(search_lower.begin(), search_lower.end(), search_lower.begin(), [](unsigned char c) {
					return static_cast<char>(std::tolower(c));
				});

				// Search field full-width, tall like the players search
				PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 12.f));
				SetNextItemWidth(GetContentRegionAvail().x);
				InputText("##VehicleSearch", vehicle_search, IM_ARRAYSIZE(vehicle_search));
				PopStyleVar();

				// Slightly smaller list height per request
				BeginChild("VehicleList", ImVec2(0, 260.f), true);
				{
					// Increase font scale slightly for better readability (same as Players)
					SetWindowFontScale(1.08f); // 8% larger text - more readable but not too big
					
					// Increase padding for better clickability while keeping text clear (same as Players)
					PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f, 8.f));
					PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.f, 8.f));
					
					bool any_displayed = false;
					
					// Show message if no vehicles in list at all
					if (nearby.empty()) {
						if (vehicles.empty()) {
							TextDisabled("No vehicles found in game");
						} else {
							TextDisabled("No valid vehicles nearby");
						}
					} else {
						for (const auto& e : nearby) {
							std::string name_lower = e.name;
							std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) {
								return static_cast<char>(std::tolower(c));
							});
							
							// Filter by search text if provided
							if (!search_lower.empty() && name_lower.find(search_lower) == std::string::npos)
								continue;

							const int m = static_cast<int>(std::round(e.distance));
							std::string label = e.name + " (" + std::to_string(m) + "m)";
							bool is_selected = (selected_vehicle_index == e.index);
							
							PushID(e.index);
							// Use Selectable - check both return value and IsItemClicked to ensure clicking works
							bool selectable_clicked = Selectable(label.c_str(), is_selected);
							bool item_clicked = IsItemClicked(0); // Check for left mouse button click
							
							// Update selection if Selectable was clicked OR if item was clicked
							if (selectable_clicked || item_clicked) {
								selected_vehicle_index = e.index;
							}
							PopID();
							any_displayed = true;
						}
						
						// Show message if search filtered out all vehicles
						if (!any_displayed && !search_lower.empty()) {
							TextDisabled("No vehicles found matching search");
						} else if (!any_displayed) {
							TextDisabled("No vehicles to display");
						}
					}
					
					PopStyleVar(2); // Restore style vars
					SetWindowFontScale(1.0f); // Restore font scale
				}
				EndChild();
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			cm.beginchild("Actions");
			{
				if (selected_vehicle_index == -1) {
					Text("Select a vehicle first!");
				}
				
				// Draw custom buttons with hover effects (same style as Settings)
				auto draw_list = GetWindowDrawList();
				
				// Button configuration
				ImVec2 button_size(230, 40);
				ImColor base_color(94, 61, 220, 255); // #5e3ddc
				ImColor hover_color(88, 55, 200, 255); // #5837c8
				
				// Select Current button
				static float select_current_hover_alpha = 0.0f;
				PushID("select_current_vehicle");
				InvisibleButton("##select_current", button_size);
				bool select_current_hovered = IsItemHovered();
				bool select_current_clicked = IsItemClicked();
				ImVec2 select_current_min = GetItemRectMin();
				ImVec2 select_current_max = GetItemRectMax();
				PopID();
				
				select_current_hover_alpha = ImLerp(select_current_hover_alpha, select_current_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 select_current_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, select_current_hover_alpha);
				ImColor select_current_final_color(select_current_final_vec);
				
				draw_list->AddRectFilled(select_current_min, select_current_max, select_current_final_color, GImGui->Style.FrameRounding);
				ImVec2 select_current_text_size = CalcTextSize("Select Current");
				draw_list->AddText(select_current_min + (button_size - select_current_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Select Current");
				
				if (select_current_clicked) {
					const auto local = Cheat::g_Fivem.GetLocalPlayerInfo();
					if (local.Ped) {
						if (auto v = local.Ped->GetLastVehicle()) {
							// Find matching index in vehicle list
							const auto vehicles = Cheat::g_Fivem.GetVehicleList();
							for (const auto& it : vehicles) {
								if (it.Vehicle == v) {
									selected_vehicle_index = it.iIndex;
									break;
								}
							}
						}
					}
				}

				if (selected_vehicle_index != -1) {
					// Teleport to selected vehicle button
					static float teleport_vehicle_hover_alpha = 0.0f;
					PushID("teleport_vehicle");
					InvisibleButton("##teleport_vehicle", button_size);
					bool teleport_vehicle_hovered = IsItemHovered();
					bool teleport_vehicle_clicked = IsItemClicked();
					ImVec2 teleport_vehicle_min = GetItemRectMin();
					ImVec2 teleport_vehicle_max = GetItemRectMax();
					PopID();
					
					teleport_vehicle_hover_alpha = ImLerp(teleport_vehicle_hover_alpha, teleport_vehicle_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 teleport_vehicle_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, teleport_vehicle_hover_alpha);
					ImColor teleport_vehicle_final_color(teleport_vehicle_final_vec);
					
					draw_list->AddRectFilled(teleport_vehicle_min, teleport_vehicle_max, teleport_vehicle_final_color, GImGui->Style.FrameRounding);
					ImVec2 teleport_vehicle_text_size = CalcTextSize("Teleport to");
					draw_list->AddText(teleport_vehicle_min + (button_size - teleport_vehicle_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Teleport to");
					
					if (teleport_vehicle_clicked) {
						const auto vehicles = Cheat::g_Fivem.GetVehicleList();
						for (const auto& it : vehicles) {
							if (it.iIndex == selected_vehicle_index && it.Vehicle) {
								Vector3D pos = it.Vehicle->GetCoordinate();
								uint64_t Object = 0, Navigation = 0, ModelInfo = 0;
								auto local = Cheat::g_Fivem.GetLocalPlayerInfo();
								if (local.Ped) {
									if (local.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle)) {
										if (auto veh = local.Ped->GetLastVehicle()) {
											Object = (uint64_t)veh;
											Navigation = (uint64_t)veh->GetNavigation();
											ModelInfo = veh->GetModelInfo();
										}
									}
									if (!Object || !Navigation || !ModelInfo) {
										Object = (uint64_t)local.Ped;
										Navigation = (uint64_t)local.Ped->GetNavigation();
										ModelInfo = local.Ped->GetModelInfo();
									}
									Cheat::g_Fivem.TeleportObject(Object, Navigation, ModelInfo, pos, pos, true);
								}
								break;
							}
						}
					}

					// Unlock door button
					static float unlock_door_hover_alpha = 0.0f;
					PushID("unlock_door");
					InvisibleButton("##unlock_door", button_size);
					bool unlock_door_hovered = IsItemHovered();
					bool unlock_door_clicked = IsItemClicked();
					ImVec2 unlock_door_min = GetItemRectMin();
					ImVec2 unlock_door_max = GetItemRectMax();
					PopID();
					
					unlock_door_hover_alpha = ImLerp(unlock_door_hover_alpha, unlock_door_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 unlock_door_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, unlock_door_hover_alpha);
					ImColor unlock_door_final_color(unlock_door_final_vec);
					
					draw_list->AddRectFilled(unlock_door_min, unlock_door_max, unlock_door_final_color, GImGui->Style.FrameRounding);
					ImVec2 unlock_door_text_size = CalcTextSize("Unlock door");
					draw_list->AddText(unlock_door_min + (button_size - unlock_door_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Unlock door");
					
					if (unlock_door_clicked) {
						const auto vehicles = Cheat::g_Fivem.GetVehicleList();
						for (const auto& it : vehicles) {
							if (it.iIndex == selected_vehicle_index && it.Vehicle) {
								it.Vehicle->SetDoorLock(1); // unlock
								break;
							}
						}
					}
					
					// Lock door button
					static float lock_door_hover_alpha = 0.0f;
					PushID("lock_door");
					InvisibleButton("##lock_door", button_size);
					bool lock_door_hovered = IsItemHovered();
					bool lock_door_clicked = IsItemClicked();
					ImVec2 lock_door_min = GetItemRectMin();
					ImVec2 lock_door_max = GetItemRectMax();
					PopID();
					
					lock_door_hover_alpha = ImLerp(lock_door_hover_alpha, lock_door_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 lock_door_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, lock_door_hover_alpha);
					ImColor lock_door_final_color(lock_door_final_vec);
					
					draw_list->AddRectFilled(lock_door_min, lock_door_max, lock_door_final_color, GImGui->Style.FrameRounding);
					ImVec2 lock_door_text_size = CalcTextSize("Lock door");
					draw_list->AddText(lock_door_min + (button_size - lock_door_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Lock door");
					
					if (lock_door_clicked) {
						const auto vehicles = Cheat::g_Fivem.GetVehicleList();
						for (const auto& it : vehicles) {
							if (it.iIndex == selected_vehicle_index && it.Vehicle) {
								it.Vehicle->SetDoorLock(2); // lock
								break;
							}
						}
					}

					// Steal Plate button
					static float steal_plate_hover_alpha = 0.0f;
					PushID("steal_plate");
					InvisibleButton("##steal_plate", button_size);
					bool steal_plate_hovered = IsItemHovered();
					bool steal_plate_clicked = IsItemClicked();
					ImVec2 steal_plate_min = GetItemRectMin();
					ImVec2 steal_plate_max = GetItemRectMax();
					PopID();
					
					steal_plate_hover_alpha = ImLerp(steal_plate_hover_alpha, steal_plate_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
					ImVec4 steal_plate_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, steal_plate_hover_alpha);
					ImColor steal_plate_final_color(steal_plate_final_vec);
					
					draw_list->AddRectFilled(steal_plate_min, steal_plate_max, steal_plate_final_color, GImGui->Style.FrameRounding);
					ImVec2 steal_plate_text_size = CalcTextSize("Steal Plate");
					draw_list->AddText(steal_plate_min + (button_size - steal_plate_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Steal Plate");
					
					if (steal_plate_clicked) {
						const auto vehicles = Cheat::g_Fivem.GetVehicleList();
						for (const auto& it : vehicles) {
							if (it.iIndex == selected_vehicle_index) {
								if (!it.Name.empty())
									ImGui::SetClipboardText(it.Name.c_str());
								break;
							}
						}
					}
				}
			}
			cm.endchild();

			cm.beginchild("Visuals");
			{
				// Enabled with color picker on the right
				widgets.Checkbox("Enabled", &g_Options.Visuals.ESP.Vehicles.Enabled);
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##VehicleESPColor", g_Options.Visuals.ESP.Vehicles.Color);

				// Remaining options
				widgets.Checkbox("Ignore Occupied", &g_Options.Visuals.ESP.Vehicles.IgnoreOccupiedVehicles);
				widgets.Checkbox("Marker", &g_Options.Visuals.ESP.Vehicles.Marker);
				widgets.Checkbox("Name", &g_Options.Visuals.ESP.Vehicles.Name);
				widgets.Checkbox("Distance", &g_Options.Visuals.ESP.Vehicles.Distance);
			}
			cm.endchild();
		}
		EndGroup();
	});

	// Settings tab (index 5)
	TabsManager::get().add_page(5, [&]() {
		BeginGroup();
		{
			// First child - Options
			cm.beginchild("Options");
			{
				// Menu Key with binder on the right
				Text("Menu Key");
				SameLine(GetWindowWidth() - 70);
				SetCursorPosY(GetCursorPosY() - 2.0f);
				widgets.Binder("##MenuKey", &g_Options.General.MenuKey);
				SetCursorPosY(GetCursorPosY() + 2.0f);
				
				widgets.Checkbox("Stream Mode", &g_Options.General.CaptureBypass);
				widgets.Checkbox("Bypass Screenshot", &g_Options.General.StreamMode);
				widgets.Checkbox("Hide Watermark", &g_Options.General.HideWatermark);
				widgets.Checkbox("Debug", &g_Options.General.Debug);
				
				widgets.SliderInt("Thread Delay", &g_Options.General.ThreadDelay, 1, 10, "%d ms");
				
				// Friend Color with color picker on the right
				Text("Friend Color");
				SameLine(GetWindowWidth() - 26);
				widgets.ColorEdit("##FriendColor", g_Options.Misc.FriendColor);
				
				// Draw custom buttons with hover effects
				auto draw_list = GetWindowDrawList();
				
				// Button configuration
				ImVec2 button_size(230, 40);
				ImColor base_color(94, 61, 220, 255); // #5e3ddc
				ImColor hover_color(88, 55, 200, 255); // #5837c8
				
				// Unload button
				PushID("unload");
				InvisibleButton("##unload", button_size);
				bool unload_hovered = IsItemHovered();
				bool unload_clicked = IsItemClicked();
				ImVec2 unload_min = GetItemRectMin();
				ImVec2 unload_max = GetItemRectMax();
				PopID();
				
				unload_hover_alpha = ImLerp(unload_hover_alpha, unload_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 unload_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, unload_hover_alpha);
				ImColor unload_final_color(unload_final_vec);
				
				draw_list->AddRectFilled(unload_min, unload_max, unload_final_color, GImGui->Style.FrameRounding);
				ImVec2 unload_text_size = CalcTextSize("Unload");
				draw_list->AddText(unload_min + (button_size - unload_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Unload");
				
				if (unload_clicked) {
					g_Options.General.ShutDown = true;
				}
				
				// Bypass button
				PushID("bypass");
				InvisibleButton("##bypass", button_size);
				bool bypass_hovered = IsItemHovered();
				bool bypass_clicked = IsItemClicked();
				ImVec2 bypass_min = GetItemRectMin();
				ImVec2 bypass_max = GetItemRectMax();
				PopID();
				
				bypass_hover_alpha = ImLerp(bypass_hover_alpha, bypass_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 bypass_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, bypass_hover_alpha);
				ImColor bypass_final_color(bypass_final_vec);
				
				draw_list->AddRectFilled(bypass_min, bypass_max, bypass_final_color, GImGui->Style.FrameRounding);
				ImVec2 bypass_text_size = CalcTextSize("Bypass");
				draw_list->AddText(bypass_min + (button_size - bypass_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Bypass");
				
				if (bypass_clicked) {
					// Bypass functionality
				}
			}
			cm.endchild();
		}
		EndGroup();

		SameLine();

		BeginGroup();
		{
			// Second child - Configs
			cm.beginchild("Configs");
			{
				// Draw custom buttons with hover effects
				auto draw_list = GetWindowDrawList();
				
				// Button configuration
				ImVec2 button_size(230, 40);
				ImColor base_color(94, 61, 220, 255); // #5e3ddc
				ImColor hover_color(88, 55, 200, 255); // #5837c8
				
				// Save Config button
				PushID("save_config");
				InvisibleButton("##save_config", button_size);
				bool save_config_hovered = IsItemHovered();
				bool save_config_clicked = IsItemClicked();
				ImVec2 save_config_min = GetItemRectMin();
				ImVec2 save_config_max = GetItemRectMax();
				PopID();
				
				save_config_hover_alpha = ImLerp(save_config_hover_alpha, save_config_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 save_config_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, save_config_hover_alpha);
				ImColor save_config_final_color(save_config_final_vec);
				
				draw_list->AddRectFilled(save_config_min, save_config_max, save_config_final_color, GImGui->Style.FrameRounding);
				ImVec2 save_config_text_size = CalcTextSize("Save Config");
				draw_list->AddText(save_config_min + (button_size - save_config_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Save Config");
				
				if (save_config_clicked) {
					g_ConfigManager.SaveConfig();
				}
				
				// Load Config button
				PushID("load_config");
				InvisibleButton("##load_config", button_size);
				bool load_config_hovered = IsItemHovered();
				bool load_config_clicked = IsItemClicked();
				ImVec2 load_config_min = GetItemRectMin();
				ImVec2 load_config_max = GetItemRectMax();
				PopID();
				
				load_config_hover_alpha = ImLerp(load_config_hover_alpha, load_config_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
				ImVec4 load_config_final_vec = ImLerp((ImVec4)base_color, (ImVec4)hover_color, load_config_hover_alpha);
				ImColor load_config_final_color(load_config_final_vec);
				
				draw_list->AddRectFilled(load_config_min, load_config_max, load_config_final_color, GImGui->Style.FrameRounding);
				ImVec2 load_config_text_size = CalcTextSize("Load Config");
				draw_list->AddText(load_config_min + (button_size - load_config_text_size) * 0.5f, ImColor(0, 0, 0, 255), "Load Config");
				
				if (load_config_clicked) {
					bool configLoaded = g_ConfigManager.LoadConfig();
					if (!configLoaded) {
						// If config file doesn't exist, ensure defaults are set
						g_ConfigManager.Setup();
					}
				}
			}
			cm.endchild();
		}
		EndGroup();
	});
}

void GUI::draw( ) {
	static bool bools[100];
	static int ints[100];
	static bool multicombo_values[4];
	static float col[100][4];
	static char buf[16];
	static char search_buf[128] = "";


	SetNextWindowSize( size );
	Begin( "GUI", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar );
	{
		BeginChild( "navbar", { 180, 0 }, 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar );
		{
			// Logo area with search box - 15px padding all around
			SetCursorPos( { 15, 15 } );
			
			// Draw border manually
			auto navbar_pos = GetWindowPos();
			GetWindowDrawList()->AddRect(
				navbar_pos + ImVec2{15, 15}, 
				navbar_pos + ImVec2{165, 53}, 
				GetColorU32(ImGuiCol_Border), 
				4.0f
			);
			
			BeginChild("search_area", { 150, 38 }, 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
			{
				SetCursorPos({ 6, 7 });
				PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
				PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(22, 6));
				PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
				
				// Draw search icon
				auto window_pos = GetWindowPos();
				GetWindowDrawList()->AddText(fonts[icons].get(14), 14, window_pos + ImVec2{8, 13}, GetColorU32(ImGuiCol_TextDisabled), i_search_md);
				
				SetNextItemWidth(138);
				InputTextWithHint("##search", "Search...", search_buf, sizeof(search_buf));
				
				PopStyleVar(2);
				PopStyleColor();
			}
			EndChild();
			
			SetCursorPosY(68);
			WidgetsManager::get( ).Separator( );
			SetCursorPosX( 14 );
			TabsManager::get( ).render_tabs( 8 );

			GetWindowDrawList( )->AddRectFilled( GetWindowPos( ) + ImVec2{ GetWindowWidth( ) - 1, 0 }, GetWindowPos( ) + GetWindowSize( ), GetColorU32( ImGuiCol_Border ) );

			SetCursorPos( { 14, GetWindowHeight( ) - 38 } );

		}
		EndChild( );

		SameLine( 0, 0 );

		if ( strlen( SearchManager::get( ).search_buf ) > 0 || SearchManager::get( ).get_anim( ) > 0.05f ) PushStyleVar( ImGuiStyleVar_WindowPadding, { 14, 14 } );
		BeginChild( "main", { 0, 0 }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
		{
			if ( strlen( SearchManager::get( ).search_buf ) > 0 || SearchManager::get( ).get_anim( ) > 0.05f ) PopStyleVar( );

			ChildManager::get( ).smoothscroll( );

			if ( strlen( SearchManager::get( ).search_buf ) == 0 && SearchManager::get( ).get_anim( ) < 0.05f ) {
				TabsManager::get( ).render_subtabs( 20 );

				auto window = GetCurrentWindow( );

				PushStyleVar( ImGuiStyleVar_Alpha, GImGui->Style.Alpha * TabsManager::get( ).get_anim( ) * GImGui->Style.Alpha );
				PushStyleVar( ImGuiStyleVar_WindowPadding, { 14, 14 } );
				BeginChild( "content", { 0, 0 }, ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse );
				{
					TabsManager::get( ).draw_page( window );
				}
				EndChild( );
				PopStyleVar( 2 );
			} else {
				SearchManager::get( ).draw( );
			}
		}
		EndChild( );
		
		PopupManager::get( ).handle( );
	}
	End( );

	// Magic Bullet confirmation popup
	if (show_magic_bullet_popup) {
		// Fade in animation
		popup_alpha = ImMin(popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		
		// Open popup automatically
		if (!IsPopupOpen("Magic Bullet Warning")) {
			OpenPopup("Magic Bullet Warning");
		}
		
		// Get screen resolution for scaling
		auto main_viewport = GetMainViewport();
		float scale_x = main_viewport->Size.x / 1920.0f;
		float scale_y = main_viewport->Size.y / 1080.0f;
		float scale = (scale_x + scale_y) * 0.5f; // Average scale
		
		// Scaled popup size
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		float popup_offset_x = 20.0f * scale;
		float popup_offset_y = 20.0f * scale;
		
		// Get main window (GUI window) position and size
		auto* gui_window = FindWindowByName("GUI");
		if (gui_window) {
			ImVec2 gui_center = ImVec2(gui_window->Pos.x + gui_window->Size.x * 0.5f, gui_window->Pos.y + gui_window->Size.y * 0.5f);
			// Popup centered in GUI window, then offset left and up
			ImVec2 popup_pos = ImVec2(gui_center.x - popup_width * 0.5f - popup_offset_x, gui_center.y - popup_height * 0.5f + popup_offset_y);
			SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
		} else {
			// Fallback to viewport center
			auto center = main_viewport->GetCenter();
			ImVec2 popup_pos = ImVec2(center.x - popup_width * 0.5f - popup_offset_x, center.y - popup_height * 0.5f + popup_offset_y);
			SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
		}
		
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleVar(ImGuiStyleVar_Alpha, popup_alpha);
		PushStyleColor(ImGuiCol_PopupBg, ImColor(19, 19, 27, 255).Value);
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImColor(0, 0, 0, int(180 * popup_alpha)).Value);
		
		if (BeginPopupModal("Magic Bullet Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			// Buttons position: 20px from bottom, 20px from left, 10px spacing (scaled)
			float btn_spacing = 10.0f * scale;
			float btn_padding_x = 20.0f * scale;
			float btn_padding_y = 20.0f * scale;
			float btn_yes_width = 100.0f * scale;
			float btn_no_width = 150.0f * scale;
			float btn_height = 40.0f * scale;
			
			SetCursorPos(ImVec2(btn_padding_x, popup_height - btn_height - btn_padding_y));
			
			auto* draw_list = GetWindowDrawList();
			
			// YES button - 100x40, #8f8cff, black text
			PushID("yes_button");
			InvisibleButton("##yes_btn", ImVec2(btn_yes_width, btn_height));
			bool yes_hovered = IsItemHovered();
			bool yes_clicked = IsItemClicked();
			ImVec2 yes_button_min = GetItemRectMin();
			ImVec2 yes_button_max = GetItemRectMax();
			PopID();
			
			ImColor yes_base_color(143, 140, 255, 255);
			ImColor yes_hover_color(123, 121, 228, 255); // #7b79e4
			
			// Smooth hover fade animation
			yes_button_hover_alpha = ImLerp(yes_button_hover_alpha, yes_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
			ImVec4 yes_final_vec = ImLerp((ImVec4)yes_base_color, (ImVec4)yes_hover_color, yes_button_hover_alpha);
			ImColor yes_final_color(yes_final_vec);
			
			// Draw YES button
			draw_list->AddRectFilled(yes_button_min, yes_button_max, yes_final_color, GImGui->Style.FrameRounding);
			ImVec2 yes_text_size = CalcTextSize("YES");
			draw_list->AddText(yes_button_min + (yes_button_max - yes_button_min - yes_text_size) * 0.5f, ImColor(0, 0, 0, 255), "YES");
			
			if (yes_clicked) {
				g_Options.LegitBot.SilentAim.MagicBullet = true;
				show_magic_bullet_popup = false;
				popup_alpha = 0.0f;
				CloseCurrentPopup();
			}
			
			// Position NO button next to YES button (same Y position)
			SetCursorPos(ImVec2(btn_padding_x + btn_yes_width + btn_spacing, popup_height - btn_height - btn_padding_y));
			
			// NO button - 150x40, #1a1a24, white text
			PushID("no_button");
			InvisibleButton("##no_btn", ImVec2(btn_no_width, btn_height));
			bool no_hovered = IsItemHovered();
			bool no_clicked = IsItemClicked();
			ImVec2 no_button_min = GetItemRectMin();
			ImVec2 no_button_max = GetItemRectMax();
			PopID();
			
			ImColor no_base_color(26, 26, 36, 255);
			ImColor no_hover_color(29, 30, 40, 255); // #1d1e28
			
			// Smooth hover fade animation
			no_button_hover_alpha = ImLerp(no_button_hover_alpha, no_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
			ImVec4 no_final_vec = ImLerp((ImVec4)no_base_color, (ImVec4)no_hover_color, no_button_hover_alpha);
			ImColor no_final_color(no_final_vec);
			
			// Draw NO button
			draw_list->AddRectFilled(no_button_min, no_button_max, no_final_color, GImGui->Style.FrameRounding);
			ImVec2 no_text_size = CalcTextSize("NO");
			draw_list->AddText(no_button_min + (no_button_max - no_button_min - no_text_size) * 0.5f, ImColor(255, 255, 255, 255), "NO");
			
			if (no_clicked) {
				g_Options.LegitBot.SilentAim.MagicBullet = false;
				show_magic_bullet_popup = false;
				popup_alpha = 0.0f;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		
		// If popup was closed, reset state
		if (!IsPopupOpen("Magic Bullet Warning") && show_magic_bullet_popup) {
			show_magic_bullet_popup = false;
			popup_alpha = 0.0f;
		}
		
		PopStyleColor(2);
		PopStyleVar();
	}

	// No Recoil confirmation popup 
	if (show_no_recoil_popup) {
		popup_alpha = ImMin(popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		if (!IsPopupOpen("No Recoil Warning")) {
			OpenPopup("No Recoil Warning");
		}
		
		auto main_viewport = GetMainViewport();
		float scale = (main_viewport->Size.x / 1920.0f + main_viewport->Size.y / 1080.0f) * 0.5f;
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		
		auto center = main_viewport->GetCenter();
		SetNextWindowPos(ImVec2(center.x - popup_width * 0.5f, center.y - popup_height * 0.5f), ImGuiCond_Appearing);
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.102f, 0.102f, 0.114f, popup_alpha * 0.98f));
		PushStyleColor(ImGuiCol_Border, ImVec4(0.353f, 0.353f, 0.373f, popup_alpha * 0.8f));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		
		if (BeginPopupModal("No Recoil Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			ImVec2 text_size = CalcTextSize("Enable No Recoil?");
			SetCursorPos(ImVec2((popup_width - text_size.x) * 0.5f, 30.0f * scale));
			Text("Enable No Recoil?");
			
			ImVec2 warning_size = CalcTextSize("This may be detected by anti-cheat!");
			SetCursorPos(ImVec2((popup_width - warning_size.x) * 0.5f, 55.0f * scale));
			TextColored(ImColor(255, 80, 80, 255), "This may be detected by anti-cheat!");
			
			float btn_width = 100.0f * scale;
			float btn_height = 40.0f * scale;
			SetCursorPos(ImVec2((popup_width - btn_width * 2 - 10.0f * scale) * 0.5f, popup_height - btn_height - 20.0f * scale));
			
			if (Button("YES", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoRecoil = true;
				show_no_recoil_popup = false;
				CloseCurrentPopup();
			}
			
			SameLine();
			if (Button("NO", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoRecoil = false;
				show_no_recoil_popup = false;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		if (!IsPopupOpen("No Recoil Warning")) show_no_recoil_popup = false;
		PopStyleColor(2);
		PopStyleVar();
	}
	
	// No Spread confirmation popup
	if (show_no_spread_popup) {
		popup_alpha = ImMin(popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		if (!IsPopupOpen("No Spread Warning")) {
			OpenPopup("No Spread Warning");
		}
		
		auto main_viewport = GetMainViewport();
		float scale = (main_viewport->Size.x / 1920.0f + main_viewport->Size.y / 1080.0f) * 0.5f;
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		
		auto center = main_viewport->GetCenter();
		SetNextWindowPos(ImVec2(center.x - popup_width * 0.5f, center.y - popup_height * 0.5f), ImGuiCond_Appearing);
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.102f, 0.102f, 0.114f, popup_alpha * 0.98f));
		PushStyleColor(ImGuiCol_Border, ImVec4(0.353f, 0.353f, 0.373f, popup_alpha * 0.8f));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		
		if (BeginPopupModal("No Spread Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			ImVec2 text_size = CalcTextSize("Enable No Spread?");
			SetCursorPos(ImVec2((popup_width - text_size.x) * 0.5f, 30.0f * scale));
			Text("Enable No Spread?");
			
			ImVec2 warning_size = CalcTextSize("This may be detected by anti-cheat!");
			SetCursorPos(ImVec2((popup_width - warning_size.x) * 0.5f, 55.0f * scale));
			TextColored(ImColor(255, 80, 80, 255), "This may be detected by anti-cheat!");
			
			float btn_width = 100.0f * scale;
			float btn_height = 40.0f * scale;
			SetCursorPos(ImVec2((popup_width - btn_width * 2 - 10.0f * scale) * 0.5f, popup_height - btn_height - 20.0f * scale));
			
			if (Button("YES", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoSpread = true;
				show_no_spread_popup = false;
				CloseCurrentPopup();
			}
			
			SameLine();
			if (Button("NO", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoSpread = false;
				show_no_spread_popup = false;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		if (!IsPopupOpen("No Spread Warning")) show_no_spread_popup = false;
		PopStyleColor(2);
		PopStyleVar();
	}
	
	// No Reload confirmation popup
	if (show_no_reload_popup) {
		popup_alpha = ImMin(popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		if (!IsPopupOpen("No Reload Warning")) {
			OpenPopup("No Reload Warning");
		}
		
		auto main_viewport = GetMainViewport();
		float scale = (main_viewport->Size.x / 1920.0f + main_viewport->Size.y / 1080.0f) * 0.5f;
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		
		auto center = main_viewport->GetCenter();
		SetNextWindowPos(ImVec2(center.x - popup_width * 0.5f, center.y - popup_height * 0.5f), ImGuiCond_Appearing);
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.102f, 0.102f, 0.114f, popup_alpha * 0.98f));
		PushStyleColor(ImGuiCol_Border, ImVec4(0.353f, 0.353f, 0.373f, popup_alpha * 0.8f));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		
		if (BeginPopupModal("No Reload Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			ImVec2 text_size = CalcTextSize("Enable No Reload?");
			SetCursorPos(ImVec2((popup_width - text_size.x) * 0.5f, 30.0f * scale));
			Text("Enable No Reload?");
			
			ImVec2 warning_size = CalcTextSize("This may be detected by anti-cheat!");
			SetCursorPos(ImVec2((popup_width - warning_size.x) * 0.5f, 55.0f * scale));
			TextColored(ImColor(255, 80, 80, 255), "This may be detected by anti-cheat!");
			
			float btn_width = 100.0f * scale;
			float btn_height = 40.0f * scale;
			SetCursorPos(ImVec2((popup_width - btn_width * 2 - 10.0f * scale) * 0.5f, popup_height - btn_height - 20.0f * scale));
			
			if (Button("YES", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoReload = true;
				show_no_reload_popup = false;
				CloseCurrentPopup();
			}
			
			SameLine();
			if (Button("NO", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.Weapon.NoReload = false;
				show_no_reload_popup = false;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		if (!IsPopupOpen("No Reload Warning")) show_no_reload_popup = false;
		PopStyleColor(2);
		PopStyleVar();
	}
	
	// Anti Headshot confirmation popup
	if (show_anti_headshot_popup) {
		popup_alpha = ImMin(popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		if (!IsPopupOpen("Anti Headshot Warning")) {
			OpenPopup("Anti Headshot Warning");
		}
		
		auto main_viewport = GetMainViewport();
		float scale = (main_viewport->Size.x / 1920.0f + main_viewport->Size.y / 1080.0f) * 0.5f;
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		
		auto center = main_viewport->GetCenter();
		SetNextWindowPos(ImVec2(center.x - popup_width * 0.5f, center.y - popup_height * 0.5f), ImGuiCond_Appearing);
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.102f, 0.102f, 0.114f, popup_alpha * 0.98f));
		PushStyleColor(ImGuiCol_Border, ImVec4(0.353f, 0.353f, 0.373f, popup_alpha * 0.8f));
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		
		if (BeginPopupModal("Anti Headshot Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			ImVec2 text_size = CalcTextSize("Enable Anti Headshot?");
			SetCursorPos(ImVec2((popup_width - text_size.x) * 0.5f, 30.0f * scale));
			Text("Enable Anti Headshot?");
			
			ImVec2 warning_size = CalcTextSize("This may be detected by anti-cheat!");
			SetCursorPos(ImVec2((popup_width - warning_size.x) * 0.5f, 55.0f * scale));
			TextColored(ImColor(255, 80, 80, 255), "This may be detected by anti-cheat!");
			
			float btn_width = 100.0f * scale;
			float btn_height = 40.0f * scale;
			SetCursorPos(ImVec2((popup_width - btn_width * 2 - 10.0f * scale) * 0.5f, popup_height - btn_height - 20.0f * scale));
			
			if (Button("YES", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.LocalPlayer.AntiHeadshot = true;
				show_anti_headshot_popup = false;
				CloseCurrentPopup();
			}
			
			SameLine();
			if (Button("NO", ImVec2(btn_width, btn_height))) {
				g_Options.Misc.Exploits.LocalPlayer.AntiHeadshot = false;
				show_anti_headshot_popup = false;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		if (!IsPopupOpen("Anti Headshot Warning")) show_anti_headshot_popup = false;
		PopStyleColor(2);
		PopStyleVar();
	}
	
	// Noclip confirmation popup
	if (show_noclip_popup) {
		// Fade in animation
		noclip_popup_alpha = ImMin(noclip_popup_alpha + GetIO().DeltaTime * 8.0f, 1.0f);
		
		// Open popup automatically
		if (!IsPopupOpen("Noclip Warning")) {
			OpenPopup("Noclip Warning");
		}
		
		// Get screen resolution for scaling
		auto main_viewport = GetMainViewport();
		float scale_x = main_viewport->Size.x / 1920.0f;
		float scale_y = main_viewport->Size.y / 1080.0f;
		float scale = (scale_x + scale_y) * 0.5f; // Average scale
		
		// Scaled popup size
		float popup_width = 300.0f * scale;
		float popup_height = 150.0f * scale;
		float popup_offset_x = 20.0f * scale;
		float popup_offset_y = 20.0f * scale;
		
		// Get main window (GUI window) position and size
		auto* gui_window = FindWindowByName("GUI");
		if (gui_window) {
			ImVec2 gui_center = ImVec2(gui_window->Pos.x + gui_window->Size.x * 0.5f, gui_window->Pos.y + gui_window->Size.y * 0.5f);
			// Popup centered in GUI window, then offset left and up
			ImVec2 popup_pos = ImVec2(gui_center.x - popup_width * 0.5f - popup_offset_x, gui_center.y - popup_height * 0.5f + popup_offset_y);
			SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
		} else {
			// Fallback to viewport center
			auto center = main_viewport->GetCenter();
			ImVec2 popup_pos = ImVec2(center.x - popup_width * 0.5f - popup_offset_x, center.y - popup_height * 0.5f + popup_offset_y);
			SetNextWindowPos(popup_pos, ImGuiCond_Appearing);
		}
		
		SetNextWindowSize(ImVec2(popup_width, popup_height), ImGuiCond_Always);
		
		PushStyleVar(ImGuiStyleVar_Alpha, noclip_popup_alpha);
		PushStyleColor(ImGuiCol_PopupBg, ImColor(19, 19, 27, 255).Value);
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImColor(0, 0, 0, int(180 * noclip_popup_alpha)).Value);
		
		if (BeginPopupModal("Noclip Warning", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar)) {
			// Buttons position: 20px from bottom, 20px from left, 10px spacing (scaled)
			float btn_spacing = 10.0f * scale;
			float btn_padding_x = 20.0f * scale;
			float btn_padding_y = 20.0f * scale;
			float btn_yes_width = 100.0f * scale;
			float btn_no_width = 150.0f * scale;
			float btn_height = 40.0f * scale;
			
			SetCursorPos(ImVec2(btn_padding_x, popup_height - btn_height - btn_padding_y));
			
			auto* draw_list = GetWindowDrawList();
			
			// YES button - 100x40, #8f8cff, black text
			PushID("yes_button_noclip");
			InvisibleButton("##yes_btn_noclip", ImVec2(btn_yes_width, btn_height));
			bool yes_hovered = IsItemHovered();
			bool yes_clicked = IsItemClicked();
			ImVec2 yes_button_min = GetItemRectMin();
			ImVec2 yes_button_max = GetItemRectMax();
			PopID();
			
			ImColor yes_base_color(143, 140, 255, 255);
			ImColor yes_hover_color(123, 121, 228, 255); // #7b79e4
			
			// Smooth hover fade animation
			yes_button_hover_alpha_noclip = ImLerp(yes_button_hover_alpha_noclip, yes_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
			ImVec4 yes_final_vec = ImLerp((ImVec4)yes_base_color, (ImVec4)yes_hover_color, yes_button_hover_alpha_noclip);
			ImColor yes_final_color(yes_final_vec);
			
			// Draw YES button
			draw_list->AddRectFilled(yes_button_min, yes_button_max, yes_final_color, GImGui->Style.FrameRounding);
			ImVec2 yes_text_size = CalcTextSize("YES");
			draw_list->AddText(yes_button_min + (yes_button_max - yes_button_min - yes_text_size) * 0.5f, ImColor(0, 0, 0, 255), "YES");
			
			if (yes_clicked) {
				g_Options.Misc.Exploits.LocalPlayer.Noclip = true;
				show_noclip_popup = false;
				noclip_popup_alpha = 0.0f;
				CloseCurrentPopup();
			}
			
			// Position NO button next to YES button (same Y position)
			SetCursorPos(ImVec2(btn_padding_x + btn_yes_width + btn_spacing, popup_height - btn_height - btn_padding_y));
			
			// NO button - 150x40, #1a1a24, white text
			PushID("no_button_noclip");
			InvisibleButton("##no_btn_noclip", ImVec2(btn_no_width, btn_height));
			bool no_hovered = IsItemHovered();
			bool no_clicked = IsItemClicked();
			ImVec2 no_button_min = GetItemRectMin();
			ImVec2 no_button_max = GetItemRectMax();
			PopID();
			
			ImColor no_base_color(26, 26, 36, 255);
			ImColor no_hover_color(29, 30, 40, 255); // #1d1e28
			
			// Smooth hover fade animation
			no_button_hover_alpha_noclip = ImLerp(no_button_hover_alpha_noclip, no_hovered ? 1.0f : 0.0f, GetIO().DeltaTime * 12.0f);
			ImVec4 no_final_vec = ImLerp((ImVec4)no_base_color, (ImVec4)no_hover_color, no_button_hover_alpha_noclip);
			ImColor no_final_color(no_final_vec);
			
			// Draw NO button
			draw_list->AddRectFilled(no_button_min, no_button_max, no_final_color, GImGui->Style.FrameRounding);
			ImVec2 no_text_size = CalcTextSize("NO");
			draw_list->AddText(no_button_min + (no_button_max - no_button_min - no_text_size) * 0.5f, ImColor(255, 255, 255, 255), "NO");
			
			if (no_clicked) {
				g_Options.Misc.Exploits.LocalPlayer.Noclip = false;
				show_noclip_popup = false;
				noclip_popup_alpha = 0.0f;
				CloseCurrentPopup();
			}
			
			EndPopup();
		}
		
		// If popup was closed, reset state
		if (!IsPopupOpen("Noclip Warning") && show_noclip_popup) {
			show_noclip_popup = false;
			noclip_popup_alpha = 0.0f;
		}
		
		PopStyleColor(2);
		PopStyleVar();
	}

	NotifyManager::get( ).draw( );
}