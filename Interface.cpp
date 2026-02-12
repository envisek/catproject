#include "Interface.hpp"
#include "Overlay.hpp"

#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <wincodec.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <memory>

#include "../Security/LazyImporter.hpp"
#include "../Security/XorStr.hpp"
#include "../Security/LoginSystem.hpp"
#include "../Utilities/Memory.hpp"

#include <Cheat/Cheat.hpp>
#include <Cheat/ConfigSystem.hpp>
#include <Cheat/Options.hpp>
#include "Cheat/Features/Bypass/bajpikc.h"
#include "Cheat/Features/Visuals/PlayerESP.hpp"
#include "Cheat/Features/Misc/Exploits.hpp"
#include "Cheat/FivemSDK/Fivem.hpp"

#include "../Dependencies/ImGui/imgui.h"
#include "../Dependencies/ImGui/imgui_impl_dx11.h"
#include "../Dependencies/ImGui/imgui_impl_win32.h"
#include "manrope_semibold.h"
#include "bytes.h"

#include "../Dependencies/ImGui/imgui_internal.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib, "..\\Dependencies\\GLFW\\glfw3.lib")

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "windowscodecs.lib")

using namespace Cheat;

extern std::unique_ptr<KrakunAPIClient> g_API;

namespace
{
    void SyncOptionsToMenu();
    void SyncMenuToOptions();
        struct MenuState;

        static ID3D11Device* g_MenuDevice = nullptr;
        static ID3D11DeviceContext* g_MenuDeviceContext = nullptr;
        static HWND g_MenuWindow = nullptr;

        static ImFont* g_ManropeFont = nullptr;
        static bool g_FontsInitialized = false;
        static bool g_TexturesInitialized = false;

        static ImTextureID g_logoTexture = nullptr;
        static float g_logoWidth = 0.0f;
        static float g_logoHeight = 0.0f;
        static ImTextureID g_crosshairTexture = nullptr;
        static ImTextureID g_eyeTexture = nullptr;
        static ImTextureID g_zapTexture = nullptr;
        static ImTextureID g_paletteTexture = nullptr;
        static ImTextureID g_userroundTexture = nullptr;
        static ImTextureID g_settingsTexture = nullptr;
        static ImTextureID g_keybindsTexture = nullptr;
        static ImTextureID g_dominikaTexture = nullptr;

        static float g_crosshairWidth = 0.0f;
        static float g_crosshairHeight = 0.0f;
        static float g_eyeWidth = 0.0f;
        static float g_eyeHeight = 0.0f;
        static float g_zapWidth = 0.0f;
        static float g_zapHeight = 0.0f;
        static float g_paletteWidth = 0.0f;
        static float g_paletteHeight = 0.0f;
        static float g_userroundWidth = 0.0f;
        static float g_userroundHeight = 0.0f;
        static float g_settingsWidth = 0.0f;
        static float g_settingsHeight = 0.0f;
        static float g_keybindsWidth = 0.0f;
        static float g_keybindsHeight = 0.0f;
        static float g_dominikaWidth = 0.0f;
        static float g_dominikaHeight = 0.0f;

        static bool g_IsDragging = false;
        static POINT g_DragStartMouse{};
        static float g_DragStartOffsetX = 0.0f;
        static float g_DragStartOffsetY = 0.0f;
        static float g_MenuOffsetX = 0.0f;
        static float g_MenuOffsetY = 0.0f;
        static float g_MenuOffsetTargetX = 0.0f;
        static float g_MenuOffsetTargetY = 0.0f;
        static const float g_DragSmoothness = 0.15f;

        static bool g_IsLoading = true;
        static float g_LoadingStartTime = 0.0f;
        static const float g_LoadingDuration = 5.0f;
        static float g_MenuFadeInAlpha = 0.0f;
        static float g_MenuScale = 0.95f;
        static const float g_FadeInDuration = 0.8f;
        static bool g_MenuHasRenderedOnce = false;

        static bool g_ShowSilentBypassModal = false;
        static bool g_ShowLoadConfigModal = false;
        static bool g_ShowSaveConfigModal = false;

        static Cheat::ConfigManager g_ConfigManager;

        struct MenuState
        {
            char activeTab[32] = "silent";

            bool silentEnable = false;
            char silentKeybind[32] = "None";
            bool silentShowFov = false;
            bool silentPrediction = false;
            bool silentForceDriver = false;
            bool silentAliveOnly = false;
            bool silentVisibleOnly = false;
            bool silentTargetPed = false;
            bool silentAlwaysOn = false;
            bool silentBypass = false;
            bool silentBypassV2 = false;
            bool silentMagicBullet = false;
            float silentFov = 0.0f;
            float silentDistance = 0.0f;
            float silentPredictionTime = 0.01f;
            float silentMisschance = 0.0f;
            int silentHitBoxIndex = 0;

            bool aimbotEnable = false;
            char aimbotKeybind[32] = "None";
            bool aimbotShowFov = false;
            bool aimbotTargetPeds = false;
            bool aimbotVisibleOnly = false;
            bool aimbotAliveOnly = false;
            float aimbotDistance = 0.0f;
            float aimbotFovMax = 1.0f;
            float aimbotFovMin = 0.0f;
            float aimbotSmoothHorizontal = 0.0f;
            float aimbotSmoothVertical = 0.0f;
            int aimbotHitBoxIndex = 0;

            bool triggerEnable = false;
            char triggerKeybind[32] = "None";
            bool triggerTargetPed = false;
            bool triggerVisibleCheck = false;
            float triggerMaxDistance = 0.0f;
            float triggerReactionTime = 0.0f;

            bool visualsEnable = false;
            bool visualsBoxes = false;
            bool visualsSkeleton = false;
            bool visualsPlayerNames = false;
            bool visualsHealthbar = false;
            bool visualsWeapon = false;
            bool visualsDistance = false;
            bool visualsIndicateHeadshot = false;
            float visualsRenderDistance = 0.0f;
            bool visualsVisibleOnly = false;
            bool visualsShowLocalPlayer = false;
            bool visualsShowNPCs = false;

            char playerSearch[256] = "";
            std::vector<std::string> playersList = { "Player 1 - 150m", "Player 2 - 250m", "Player 3 - 100m" };
            int selectedPlayerIndex = 0;
            std::vector<std::string> friendsList = { "Friend 1", "Friend 2" };
            int selectedFriendIndex = 0;

            char menuKey[32] = "INSERT";
            char panicKey[32] = "None";
            bool streamMode = false;
            bool bypassDestructMode = false;
            float updateDelay = 0.0f;
            char friendColor[32] = "#4a9eff";
            bool noclip = false;
            char noclipKey[32] = "None";
            float noclipSpeed = 0.0f;

            bool miscNoRecoil = false;
            bool miscNoSpread = false;
            bool miscNoReload = false;
            bool miscStartHealth = false;
            int miscHealthAmount = 200;

            std::vector<std::string> configList = { "Config 1", "Config 2", "Config 3", "Default Config" };
            int selectedConfigIndex = 0;
            char saveConfigName[256] = "";
            int selectedConfigIndex = 0;
            char saveConfigName[256] = "";

            // Colors
            char boxColor[32] = "#00ff00";
            char boxGlowColor[32] = "#00ff00";
            char skeletonColor[32] = "#00ff00";
            char nameColor[32] = "#ffffff";
            char weaponColor[32] = "#ffff00";
            char distanceColor[32] = "#cccccc";
            char snaplineColor[32] = "#00ff00";
            char aimbotFovColor[32] = "#ff0000";
            char silentFovColor[32] = "#00ff00";
            char healthBarColor[32] = "#00ff00";
            char healthBarLowColor[32] = "#ff0000";
            char armorBarColor[32] = "#0080ff";
            char vehicleColor[32] = "#00ff00";

        static MenuState g_MenuState;

        struct KeybindCaptureState
        {
            bool active = false;
            char* buffer = nullptr;
            size_t bufferSize = 0;
            int activationFrame = 0;
            int suppressedVk = 0;
            bool waitForRelease = false;
            int ignoreNextVk = 0;
        };

        static KeybindCaptureState g_KeybindCaptureState;
        static char* g_LastChangedKeybind = nullptr;

        void CancelKeybindCapture()
        {
            g_KeybindCaptureState = {};
        }

        void BeginKeybindCapture(char* buffer, size_t buffer_size, bool suppress_initial_mouse)
        {
            if (!buffer || buffer_size == 0)
                return;

            g_KeybindCaptureState.active = true;
            g_KeybindCaptureState.buffer = buffer;
            g_KeybindCaptureState.bufferSize = buffer_size;
            g_KeybindCaptureState.activationFrame = ImGui::GetFrameCount();
            g_KeybindCaptureState.waitForRelease = false;
            g_KeybindCaptureState.suppressedVk = 0;
            g_KeybindCaptureState.ignoreNextVk = 0;
            g_LastChangedKeybind = nullptr;

            if (suppress_initial_mouse)
            {
                g_KeybindCaptureState.suppressedVk = VK_LBUTTON;
                g_KeybindCaptureState.waitForRelease = true;
            }
        }

        bool IsCapturingKeybind(const char* buffer)
        {
            return g_KeybindCaptureState.active && g_KeybindCaptureState.buffer == buffer;
        }

        bool ConsumeKeybindChanged(char* buffer)
        {
            if (g_LastChangedKeybind == buffer)
            {
                g_LastChangedKeybind = nullptr;
                return true;
            }
            return false;
        }

        void UpdateKeybindCapture();

        inline ImVec4 hexToVec4(const char* hex)
        {
            int r = 0, g = 0, b = 0;
            const char* ptr = hex;
            if (hex[0] == '#')
                ptr = hex + 1;
            std::stringstream ss;
            ss << std::hex << std::setfill('0');
            ss.str(std::string(ptr, ptr + 2));
            ss >> r;
            ss.clear();
            ss.str(std::string(ptr + 2, ptr + 4));
            ss >> g;
            ss.clear();
            ss.str(std::string(ptr + 4, ptr + 6));
            ss >> b;
            return ImVec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
        }

        static ImVec4 background_color = hexToVec4("020202");
        static ImVec4 card_color = hexToVec4("13151C");
        static ImVec4 input_field_color = hexToVec4("1A1D26");
        static ImVec4 button_color = hexToVec4("1C1F28");
        static ImVec4 button_hover_color = hexToVec4("16181f");
        static ImVec4 checkbox_bg = hexToVec4("1a1d2e");
        static ImVec4 accent_color = hexToVec4("4a9eff");
        static ImVec4 text_primary = hexToVec4("ffffff");
        static ImVec4 text_secondary = hexToVec4("e8e9eb");
        static ImVec4 text_muted = hexToVec4("b8b8b8");
        static ImVec4 border_color = hexToVec4("1f222d");
        static ImVec4 border_color_hover = hexToVec4("2a2d3a");
        static ImVec4 panel_bg = hexToVec4("0f1117");
        static ImVec4 sidebar_bg = hexToVec4("12141c");

        inline ImVec2 RoundToPixel(ImVec2 pos)
        {
            return ImVec2(std::floor(pos.x + 0.5f), std::floor(pos.y + 0.5f));
        }

        inline void DrawSectionTitle(ImDrawList* draw_list, ImVec2 pos, const char* title)
        {
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(pos), ImColor(text_primary), title);
                ImGui::PopFont();
            }
        }

        inline bool IsAnyModalOpen()
        {
            return g_ShowSilentBypassModal || g_ShowLoadConfigModal || g_ShowSaveConfigModal;
        }

        bool CustomCheckbox(const char* label, bool* checked)
        {
            if (IsAnyModalOpen())
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                ImVec2 checkbox_min = cursor_pos;
                ImVec2 checkbox_max = ImVec2(cursor_pos.x + 14.0f, cursor_pos.y + 14.0f);

                ImVec4 bg_color = *checked ? accent_color : checkbox_bg;
                draw_list->AddRectFilled(checkbox_min, checkbox_max, ImColor(bg_color), 8.0f, ImDrawFlags_RoundCornersAll);
                ImVec4 border_col = *checked ? accent_color : border_color_hover;
                draw_list->AddRect(checkbox_min, checkbox_max, ImColor(border_col), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

                if (*checked)
                {
                    ImVec2 checkbox_center = ImVec2((checkbox_min.x + checkbox_max.x) * 0.5f, (checkbox_min.y + checkbox_max.y) * 0.5f);
                    float scale = 1.4f;
                    ImVec2 p1 = ImVec2(checkbox_center.x + (2.0f - 5.0f) * scale, checkbox_center.y + (5.0f - 5.0f) * scale);
                    ImVec2 p2 = ImVec2(checkbox_center.x + (4.5f - 5.0f) * scale, checkbox_center.y + (7.5f - 5.0f) * scale);
                    ImVec2 p3 = ImVec2(checkbox_center.x + (8.0f - 5.0f) * scale, checkbox_center.y + (2.5f - 5.0f) * scale);
                    draw_list->AddLine(p1, p2, ImColor(255, 255, 255, 255), 2.0f);
                    draw_list->AddLine(p2, p3, ImColor(255, 255, 255, 255), 2.0f);
                }

                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    ImVec2 label_pos = RoundToPixel(ImVec2(checkbox_max.x + 8.0f, checkbox_min.y));
                    draw_list->AddText(g_ManropeFont, 12.0f, label_pos, ImColor(text_secondary), label);
                    ImGui::PopFont();
                }
                return false;
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 checkbox_min = cursor_pos;
            ImVec2 checkbox_max = ImVec2(cursor_pos.x + 14.0f, cursor_pos.y + 14.0f);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= checkbox_min.x && io.MousePos.x <= checkbox_max.x &&
                               io.MousePos.y >= checkbox_min.y && io.MousePos.y <= checkbox_max.y);

            bool clicked = false;
            if (is_hovered && ImGui::IsMouseClicked(0))
            {
                *checked = !*checked;
                clicked = true;
            }

            ImVec4 bg_color = *checked ? accent_color : checkbox_bg;
            if (is_hovered && !*checked)
                bg_color = hexToVec4("1f222d");
            draw_list->AddRectFilled(checkbox_min, checkbox_max, ImColor(bg_color), 8.0f, ImDrawFlags_RoundCornersAll);

            ImVec4 border_col = *checked ? accent_color : border_color_hover;
            draw_list->AddRect(checkbox_min, checkbox_max, ImColor(border_col), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (*checked)
            {
                ImVec2 checkbox_center = ImVec2((checkbox_min.x + checkbox_max.x) * 0.5f, (checkbox_min.y + checkbox_max.y) * 0.5f);
                float scale = 1.4f;
                ImVec2 p1 = ImVec2(checkbox_center.x + (2.0f - 5.0f) * scale, checkbox_center.y + (5.0f - 5.0f) * scale);
                ImVec2 p2 = ImVec2(checkbox_center.x + (4.5f - 5.0f) * scale, checkbox_center.y + (7.5f - 5.0f) * scale);
                ImVec2 p3 = ImVec2(checkbox_center.x + (8.0f - 5.0f) * scale, checkbox_center.y + (2.5f - 5.0f) * scale);
                draw_list->AddLine(p1, p2, ImColor(255, 255, 255, 255), 2.5f);
                draw_list->AddLine(p2, p3, ImColor(255, 255, 255, 255), 2.5f);
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 text_pos = RoundToPixel(ImVec2(checkbox_max.x + 6.0f, checkbox_min.y + (14.0f - 12.0f) * 0.5f));
                draw_list->AddText(g_ManropeFont, 12.0f, text_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            return clicked;
        }

        bool CustomCheckboxWithKeybind(const char* label, bool* checked, char* keybind_value, size_t keybind_size, float keybind_width, float column_width)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
            ImVec2 checkbox_min = cursor_pos;
            ImVec2 checkbox_max = ImVec2(cursor_pos.x + 14.0f, cursor_pos.y + 14.0f);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= checkbox_min.x && io.MousePos.x <= checkbox_max.x &&
                               io.MousePos.y >= checkbox_min.y && io.MousePos.y <= checkbox_max.y);

            bool clicked = false;
            if (!IsAnyModalOpen() && is_hovered && ImGui::IsMouseClicked(0))
            {
                *checked = !*checked;
                clicked = true;
            }

            ImVec4 bg_color = *checked ? accent_color : checkbox_bg;
            if (is_hovered && !*checked)
                bg_color = hexToVec4("1f222d");
            draw_list->AddRectFilled(checkbox_min, checkbox_max, ImColor(bg_color), 8.0f, ImDrawFlags_RoundCornersAll);

            ImVec4 border_col = *checked ? accent_color : border_color_hover;
            draw_list->AddRect(checkbox_min, checkbox_max, ImColor(border_col), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (*checked)
            {
                ImVec2 checkbox_center = ImVec2((checkbox_min.x + checkbox_max.x) * 0.5f, (checkbox_min.y + checkbox_max.y) * 0.5f);
                float scale = 1.6f;
                ImVec2 p1 = ImVec2(checkbox_center.x + (2.0f - 5.0f) * scale, checkbox_center.y + (5.0f - 5.0f) * scale);
                ImVec2 p2 = ImVec2(checkbox_center.x + (4.5f - 5.0f) * scale, checkbox_center.y + (7.5f - 5.0f) * scale);
                ImVec2 p3 = ImVec2(checkbox_center.x + (8.0f - 5.0f) * scale, checkbox_center.y + (2.5f - 5.0f) * scale);
                draw_list->AddLine(p1, p2, ImColor(255, 255, 255, 255), 2.0f);
                draw_list->AddLine(p2, p3, ImColor(255, 255, 255, 255), 2.0f);
            }

            float label_x = checkbox_max.x + 8.0f;
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 text_pos = RoundToPixel(ImVec2(label_x, checkbox_min.y + (16.0f - 14.0f) * 0.5f));
                draw_list->AddText(g_ManropeFont, 12.0f, text_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            float keybind_height = 23.0f;
            float available_width = (column_width > 0.0f) ? column_width : ImGui::GetContentRegionAvail().x;
            float keybind_x = cursor_pos.x + available_width - keybind_width - 20.0f;
            ImVec2 keybind_field_min = ImVec2(keybind_x, cursor_pos.y);
            ImVec2 keybind_field_max = ImVec2(keybind_x + keybind_width, cursor_pos.y + keybind_height);

            bool keybind_hovered = (io.MousePos.x >= keybind_field_min.x && io.MousePos.x <= keybind_field_max.x &&
                                    io.MousePos.y >= keybind_field_min.y && io.MousePos.y <= keybind_field_max.y);

            if (!IsAnyModalOpen() && keybind_hovered && ImGui::IsMouseClicked(0))
            {
                BeginKeybindCapture(keybind_value, keybind_size, true);
            }

            bool capturing = IsCapturingKeybind(keybind_value);

            ImVec4 keybind_bg_color = sidebar_bg;
            ImVec4 keybind_border_col = border_color;
            if (capturing)
            {
                keybind_bg_color = accent_color;
                keybind_border_col = accent_color;
            }
            else if (keybind_hovered)
            {
                keybind_bg_color = button_hover_color;
                keybind_border_col = border_color_hover;
            }

            draw_list->AddRectFilled(keybind_field_min, keybind_field_max, ImColor(keybind_bg_color), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(keybind_field_min, keybind_field_max, ImColor(keybind_border_col), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            float icon_size = 14.0f;
            float icon_x = keybind_field_min.x + 10.0f;
            float icon_y = keybind_field_min.y + keybind_height * 0.5f;

            if (g_keybindsTexture)
            {
                ImVec2 icon_min = ImVec2(icon_x, icon_y - icon_size * 0.5f);
                ImVec2 icon_max = ImVec2(icon_x + icon_size, icon_y + icon_size * 0.5f);
                draw_list->AddImage(g_keybindsTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                const char* display_value = (keybind_value && keybind_value[0]) ? keybind_value : "None";
                if (capturing)
                    display_value = "Waiting...";
                ImVec4 text_col = capturing ? text_primary : text_secondary;
                ImVec2 value_pos = RoundToPixel(ImVec2(keybind_field_min.x + 10.0f + icon_size + 6.0f, keybind_field_min.y + 6.0f));
                draw_list->AddText(g_ManropeFont, 12.0f, value_pos, ImColor(text_col), display_value);
                ImGui::PopFont();
            }

            bool changed = clicked;
            if (ConsumeKeybindChanged(keybind_value))
                changed = true;

            return changed;
        }

        bool CustomSlider(const char* label, float* value, float min, float max, const char* suffix, float step, float width)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float slider_width = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
            float slider_container_height = 16.0f;
            float label_font_size = 11.0f;
            float label_margin_bottom = 6.0f;
            float label_total_height = label_font_size + label_margin_bottom;
            ImVec2 slider_min = cursor_pos;
            ImVec2 slider_max = ImVec2(cursor_pos.x + slider_width, cursor_pos.y + label_total_height + slider_container_height);

            float percentage = ((*value - min) / (max - min)) * 100.0f;

            float container_start_y = slider_min.y + label_total_height;
            float track_center_y = container_start_y + (slider_container_height * 0.5f);
            float track_height = 6.0f;
            ImVec2 track_min = ImVec2(slider_min.x, track_center_y - track_height * 0.5f);
            ImVec2 track_max = ImVec2(slider_min.x + slider_width, track_center_y + track_height * 0.5f);
            draw_list->AddRectFilled(track_min, track_max, ImColor(sidebar_bg), 6.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(track_min, track_max, ImColor(border_color), 6.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            ImVec2 filled_max = ImVec2(track_min.x + (percentage / 100.0f) * (track_max.x - track_min.x), track_max.y);
            draw_list->AddRectFilled(track_min, filled_max, ImColor(accent_color), 6.0f, ImDrawFlags_RoundCornersAll);

            ImVec2 handle_center = ImVec2(track_min.x + (percentage / 100.0f) * (track_max.x - track_min.x), track_center_y);
            draw_list->AddCircleFilled(handle_center, 8.0f, ImColor(panel_bg), 64);
            draw_list->AddCircleFilled(handle_center, 6.0f, ImColor(accent_color), 64);

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(slider_min.x, slider_min.y));
                draw_list->AddText(g_ManropeFont, 11.0f, label_pos, ImColor(text_secondary), label);

                char value_text[256];
            if (suffix && suffix[0])
                    sprintf_s(value_text, "%.0f%s", *value, suffix);
                else
                    sprintf_s(value_text, "%.0f", *value);
                ImVec2 value_size = g_ManropeFont->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, value_text);
                ImVec2 value_pos = RoundToPixel(ImVec2(slider_max.x - value_size.x, slider_min.y));
                draw_list->AddText(g_ManropeFont, 11.0f, value_pos, ImColor(text_secondary), value_text);
                ImGui::PopFont();
            }

            bool changed = false;
            ImGuiIO& io = ImGui::GetIO();
            if (!IsAnyModalOpen() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0)))
            {
                if (io.MousePos.x >= slider_min.x && io.MousePos.x <= slider_max.x &&
                    io.MousePos.y >= container_start_y - 5.0f && io.MousePos.y <= container_start_y + slider_container_height + 5.0f)
                {
                    float percent = (io.MousePos.x - track_min.x) / (track_max.x - track_min.x);
                    percent = std::clamp(percent, 0.0f, 1.0f);
                    *value = min + percent * (max - min);
                    if (step > 0.0f)
                        *value = std::round(*value / step) * step;
                    changed = true;
                }
            }

            return changed;
        }

        bool CustomButton(const char* label, bool danger, float width)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float button_width = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
            float button_height = 23.0f;
            ImVec2 button_min = cursor_pos;
            ImVec2 button_max = ImVec2(cursor_pos.x + button_width, cursor_pos.y + button_height);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= button_min.x && io.MousePos.x <= button_max.x &&
                               io.MousePos.y >= button_min.y && io.MousePos.y <= button_max.y);

            ImVec4 bg_color = danger ? hexToVec4("ef4444") : sidebar_bg;
            ImVec4 hover_bg = danger ? hexToVec4("dc2626") : button_hover_color;
            ImVec4 border_col = danger ? (is_hovered ? hexToVec4("dc2626") : hexToVec4("ef4444")) : (is_hovered ? border_color_hover : border_color);

            draw_list->AddRectFilled(button_min, button_max, ImColor(is_hovered ? hover_bg : bg_color), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(button_min, button_max, ImColor(border_col), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 text_size = g_ManropeFont->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, label);
                ImVec2 text_pos = RoundToPixel(ImVec2(button_min.x + (button_width - text_size.x) * 0.5f,
                                                      button_min.y + (button_height - text_size.y) * 0.5f));
                draw_list->AddText(g_ManropeFont, 12.0f, text_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            bool clicked = false;
            if (!IsAnyModalOpen() && is_hovered && ImGui::IsMouseClicked(0))
                clicked = true;

            return clicked;
        }

        bool CustomDropdown(const char* label, const char* current_value, const char* items[], int item_count, int* selected_index, float width)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float dropdown_width = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
            float dropdown_height = 23.0f;
            ImVec2 dropdown_min = cursor_pos;
            ImVec2 dropdown_max = ImVec2(cursor_pos.x + dropdown_width, cursor_pos.y + dropdown_height);

            static bool dropdown_open = false;
            static const char* open_dropdown_id = nullptr;

            bool is_this_dropdown = (open_dropdown_id == label);
            bool is_open = (is_this_dropdown && dropdown_open);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= dropdown_min.x && io.MousePos.x <= dropdown_max.x &&
                               io.MousePos.y >= dropdown_min.y && io.MousePos.y <= dropdown_max.y);

            bool has_label = (label && label[0] != '\0');
            if (g_ManropeFont && has_label)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(cursor_pos.x, cursor_pos.y - 16.0f));
                draw_list->AddText(g_ManropeFont, 12.0f, label_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            ImVec2 field_min = ImVec2(cursor_pos.x, cursor_pos.y);
            ImVec2 field_max = ImVec2(cursor_pos.x + dropdown_width, cursor_pos.y + dropdown_height);

            ImVec4 bg_col = is_hovered ? button_hover_color : sidebar_bg;
            ImVec4 border_col = is_hovered ? border_color_hover : border_color;
            draw_list->AddRectFilled(field_min, field_max, ImColor(bg_col), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(field_min, field_max, ImColor(border_col), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 value_pos = RoundToPixel(ImVec2(field_min.x + 8.0f, field_min.y + 5.0f));
                draw_list->AddText(g_ManropeFont, 12.0f, value_pos, ImColor(text_secondary), current_value);
                ImGui::PopFont();
            }

            float chevron_size = 12.0f;
            float chevron_right_padding = 10.0f;
            ImVec2 chevron_center = ImVec2(field_max.x - chevron_right_padding - chevron_size * 0.5f,
                                           field_min.y + dropdown_height * 0.5f);

            if (is_open)
            {
                ImVec2 bottom = ImVec2(chevron_center.x, chevron_center.y + chevron_size * 0.25f);
                ImVec2 left = ImVec2(chevron_center.x - chevron_size * 0.3f, chevron_center.y);
                ImVec2 right = ImVec2(chevron_center.x + chevron_size * 0.3f, chevron_center.y);
                draw_list->AddLine(bottom, left, ImColor(0x6b, 0x72, 0x80, 255), 2.0f);
                draw_list->AddLine(bottom, right, ImColor(0x6b, 0x72, 0x80, 255), 2.0f);
            }
            else
            {
                ImVec2 top = ImVec2(chevron_center.x, chevron_center.y - chevron_size * 0.25f);
                ImVec2 left = ImVec2(chevron_center.x - chevron_size * 0.3f, chevron_center.y);
                ImVec2 right = ImVec2(chevron_center.x + chevron_size * 0.3f, chevron_center.y);
                draw_list->AddLine(top, left, ImColor(0x6b, 0x72, 0x80, 255), 2.0f);
                draw_list->AddLine(top, right, ImColor(0x6b, 0x72, 0x80, 255), 2.0f);
            }

            bool field_hovered = (io.MousePos.x >= field_min.x && io.MousePos.x <= field_max.x &&
                                  io.MousePos.y >= field_min.y && io.MousePos.y <= field_max.y);
            if (!IsAnyModalOpen() && field_hovered && ImGui::IsMouseClicked(0))
            {
                dropdown_open = !dropdown_open;
                open_dropdown_id = label;
            }

            if (!IsAnyModalOpen() && dropdown_open && !field_hovered && ImGui::IsMouseClicked(0))
            {
                dropdown_open = false;
                open_dropdown_id = nullptr;
            }

            if (!IsAnyModalOpen() && is_this_dropdown && dropdown_open && ImGui::IsMouseClicked(0))
            {
                if (!(io.MousePos.x >= field_min.x && io.MousePos.x <= field_max.x &&
                      io.MousePos.y >= field_min.y && io.MousePos.y <= field_max.y))
                {
                    float list_height = item_count * 24.0f;
                    ImVec2 list_min = ImVec2(field_min.x, field_max.y + 4.0f);
                    ImVec2 list_max = ImVec2(field_max.x, field_max.y + 4.0f + list_height);
                    if (!(io.MousePos.x >= list_min.x && io.MousePos.x <= list_max.x &&
                          io.MousePos.y >= list_min.y && io.MousePos.y <= list_max.y))
                    {
                        dropdown_open = false;
                        open_dropdown_id = nullptr;
                    }
                }
            }

            bool selection_changed = false;

            if (is_this_dropdown && dropdown_open)
            {
                float list_height = item_count * 24.0f;
                ImVec2 list_min = ImVec2(field_min.x, field_max.y + 4.0f);
                ImVec2 list_max = ImVec2(field_max.x, field_max.y + 4.0f + list_height);

                ImDrawList* foreground_draw_list = ImGui::GetForegroundDrawList();

                ImVec2 shadow_min = ImVec2(list_min.x + 2.0f, list_min.y + 2.0f);
                ImVec2 shadow_max = ImVec2(list_max.x + 2.0f, list_max.y + 2.0f);
                foreground_draw_list->AddRectFilled(shadow_min, shadow_max, ImColor(0, 0, 0, 100), 12.0f);

                foreground_draw_list->AddRectFilled(list_min, list_max, ImColor(sidebar_bg), 12.0f, ImDrawFlags_RoundCornersAll);
                foreground_draw_list->AddRect(list_min, list_max, ImColor(border_color), 12.0f, ImDrawFlags_RoundCornersAll, 1.0f);

                for (int i = 0; i < item_count; ++i)
                {
                    ImVec2 item_min = ImVec2(list_min.x, list_min.y + i * 24.0f);
                    ImVec2 item_max = ImVec2(list_max.x, list_min.y + (i + 1) * 24.0f);

                    bool item_hovered = (io.MousePos.x >= item_min.x && io.MousePos.x <= item_max.x &&
                                         io.MousePos.y >= item_min.y && io.MousePos.y <= item_max.y);

                    if (i == *selected_index)
                        foreground_draw_list->AddRectFilled(item_min, item_max, ImColor(0x4a, 0x9e, 0xff, 51), 8.0f, ImDrawFlags_RoundCornersAll);
                    else if (item_hovered)
                        foreground_draw_list->AddRectFilled(item_min, item_max, ImColor(button_hover_color), 8.0f, ImDrawFlags_RoundCornersAll);

                    if (g_ManropeFont)
                    {
                        ImGui::PushFont(g_ManropeFont);
                        ImVec4 text_col = (i == *selected_index) ? accent_color : text_secondary;
                        ImVec2 item_text_pos = RoundToPixel(ImVec2(item_min.x + 8.0f, item_min.y + 5.0f));
                        foreground_draw_list->AddText(g_ManropeFont, 12.0f, item_text_pos, ImColor(text_col), items[i]);
                        ImGui::PopFont();
                    }

                    if (!IsAnyModalOpen() && item_hovered && ImGui::IsMouseClicked(0))
                    {
                        if (selected_index && *selected_index != i)
                        {
                            *selected_index = i;
                            selection_changed = true;
                        }
                        else if (selected_index && *selected_index == i)
                        {
                            // No change in selection but still close dropdown
                        }

                        dropdown_open = false;
                        open_dropdown_id = nullptr;
                    }
                }

                if (!IsAnyModalOpen() && ImGui::IsMouseClicked(0))
                {
                    if (!(io.MousePos.x >= list_min.x && io.MousePos.x <= list_max.x &&
                          io.MousePos.y >= list_min.y && io.MousePos.y <= list_max.y) &&
                        !(io.MousePos.x >= dropdown_min.x && io.MousePos.x <= dropdown_max.x &&
                          io.MousePos.y >= dropdown_min.y && io.MousePos.y <= dropdown_max.y))
                    {
                        dropdown_open = false;
                        open_dropdown_id = nullptr;
                    }
                }
            }

            return selection_changed;
        }

        bool CustomKeybind(const char* label, char* value, size_t value_size)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float keybind_width = 90.0f;
            float keybind_height = 23.0f;

            float label_height = 15.0f;
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(cursor_pos.x, cursor_pos.y - label_height));
                draw_list->AddText(g_ManropeFont, 11.0f, label_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            ImVec2 field_min = ImVec2(cursor_pos.x, cursor_pos.y);
            ImVec2 field_max = ImVec2(cursor_pos.x + keybind_width, cursor_pos.y + keybind_height);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= field_min.x && io.MousePos.x <= field_max.x &&
                               io.MousePos.y >= field_min.y && io.MousePos.y <= field_max.y);

            if (!IsAnyModalOpen() && is_hovered && ImGui::IsMouseClicked(0))
            {
                BeginKeybindCapture(value, value_size, true);
            }

            bool capturing = IsCapturingKeybind(value);

            ImVec4 bg_color = sidebar_bg;
            ImVec4 border_col = border_color;
            if (capturing)
            {
                bg_color = accent_color;
                border_col = accent_color;
            }
            else if (is_hovered)
            {
                bg_color = button_hover_color;
                border_col = border_color_hover;
            }

            draw_list->AddRectFilled(field_min, field_max, ImColor(bg_color), 10.0f);
            draw_list->AddRect(field_min, field_max, ImColor(border_col), 10.0f, 0, 1.0f);

            float icon_size = 14.0f;
            float icon_x = field_min.x + 10.0f;
            float icon_y = field_min.y + keybind_height * 0.5f;

            if (g_keybindsTexture)
            {
                ImVec2 icon_min = ImVec2(icon_x, icon_y - icon_size * 0.5f);
                ImVec2 icon_max = ImVec2(icon_x + icon_size, icon_y + icon_size * 0.5f);
                draw_list->AddImage(g_keybindsTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                const char* display_value = (value && value[0]) ? value : "None";
                if (capturing)
                    display_value = "Waiting...";
                ImVec4 text_col = capturing ? text_primary : text_secondary;
                ImVec2 value_pos = RoundToPixel(ImVec2(field_min.x + 8.0f + icon_size + 5.0f, field_min.y + 5.0f));
                draw_list->AddText(g_ManropeFont, 12.0f, value_pos, ImColor(text_col), display_value);
                ImGui::PopFont();
            }

            return ConsumeKeybindChanged(value);
        }

        bool CustomInputText(const char* label, char* buffer, size_t buffer_size, const char* placeholder, float width)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float input_width = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
            float input_height = 19.0f;

            float label_height = 16.0f;
            if (g_ManropeFont && label && label[0] != '\0')
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(cursor_pos.x, cursor_pos.y - label_height));
                draw_list->AddText(g_ManropeFont, 12.0f, label_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            ImVec2 field_min = ImVec2(cursor_pos.x, cursor_pos.y);
            ImVec2 field_max = ImVec2(cursor_pos.x + input_width, cursor_pos.y + input_height);

            ImGuiIO& io = ImGui::GetIO();
            bool is_hovered = (io.MousePos.x >= field_min.x && io.MousePos.x <= field_max.x &&
                               io.MousePos.y >= field_min.y && io.MousePos.y <= field_max.y);

            ImVec4 bg_color = is_hovered ? button_hover_color : sidebar_bg;
            ImVec4 border_col = is_hovered ? border_color_hover : border_color;

            draw_list->AddRectFilled(field_min, field_max, ImColor(bg_color), 10.0f);
            draw_list->AddRect(field_min, field_max, ImColor(border_col), 10.0f, 0, 1.0f);

            static bool input_focused = false;
            static char* focused_input = nullptr;

            bool is_focused = (focused_input == buffer);
            bool was_clicked = false;

            if (!IsAnyModalOpen() && is_hovered && ImGui::IsMouseClicked(0))
            {
                input_focused = true;
                focused_input = buffer;
                was_clicked = true;
            }

            if (is_focused)
            {
                for (int n = 0; n < io.InputQueueCharacters.Size && n < IM_ARRAYSIZE(io.InputQueueCharacters.Data); ++n)
                {
                    unsigned int c = static_cast<unsigned int>(io.InputQueueCharacters.Data[n]);
                    if (c > 0 && c < 0x10000)
                    {
                        size_t len = std::strlen(buffer);
                        if (len < buffer_size - 1)
                        {
                            buffer[len] = static_cast<char>(c);
                            buffer[len + 1] = '\0';
                        }
                    }
                }

                if (ImGui::IsKeyDown(ImGuiKey_Backspace) && std::strlen(buffer) > 0)
                {
                    static float last_backspace_time = 0.0f;
                    float current_time = ImGui::GetTime();
                    if ((current_time - last_backspace_time) > 0.05f)
                    {
                        buffer[std::strlen(buffer) - 1] = '\0';
                        last_backspace_time = current_time;
                    }
                }

                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    input_focused = false;
                    focused_input = nullptr;
                }
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                const char* display = (std::strlen(buffer) > 0) ? buffer : (placeholder ? placeholder : "");
                ImVec2 text_size = g_ManropeFont->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, display);
                float text_y = field_min.y + (input_height - text_size.y) * 0.5f;
                ImVec2 text_pos = RoundToPixel(ImVec2(field_min.x + 10.0f, text_y));
                ImU32 col = (std::strlen(buffer) > 0) ? ImColor(text_secondary) : ImColor(text_muted);
                draw_list->AddText(g_ManropeFont, 12.0f, text_pos, col, display);

                if (is_focused)
                {
                    float cursor_x = field_min.x + 10.0f;
                    if (std::strlen(buffer) > 0)
                    {
                        ImVec2 buffer_sz = g_ManropeFont->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, buffer);
                        cursor_x += buffer_sz.x;
                    }
                    if (std::fmod(ImGui::GetTime(), 1.0f) < 0.5f)
                        draw_list->AddLine(ImVec2(cursor_x, text_y), ImVec2(cursor_x, text_y + text_size.y), ImColor(text_secondary), 1.0f);
                }
                ImGui::PopFont();
            }

            return was_clicked;
        }

        bool CustomColorPicker(const char* label, char* color_buffer, size_t buffer_size)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float colorpicker_width = ImGui::GetContentRegionAvail().x;

            float label_height = 16.0f;
            if (g_ManropeFont && label)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(cursor_pos.x, cursor_pos.y - label_height));
                draw_list->AddText(g_ManropeFont, 12.0f, label_pos, ImColor(text_secondary), label);
                ImGui::PopFont();
            }

            float color_input_width = 32.0f;
            float color_input_height = 24.0f;
            float gap = 8.0f;
            float text_input_height = 23.0f;
            float text_input_width = colorpicker_width - color_input_width - gap;

            ImVec2 color_input_min = ImVec2(cursor_pos.x, cursor_pos.y);
            ImVec2 color_input_max = ImVec2(cursor_pos.x + color_input_width, cursor_pos.y + color_input_height);

            ImGuiIO& io = ImGui::GetIO();
            bool color_input_hovered = (io.MousePos.x >= color_input_min.x && io.MousePos.x <= color_input_max.x &&
                                        io.MousePos.y >= color_input_min.y && io.MousePos.y <= color_input_max.y);

            ImVec4 color_vec = hexToVec4(color_buffer);
            draw_list->AddRectFilled(color_input_min, color_input_max, ImColor(color_vec), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(color_input_min, color_input_max, ImColor(border_color), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            ImVec2 text_input_min = ImVec2(cursor_pos.x + color_input_width + gap, cursor_pos.y);
            ImVec2 text_input_max = ImVec2(cursor_pos.x + colorpicker_width, cursor_pos.y + text_input_height);

            bool text_input_hovered = (io.MousePos.x >= text_input_min.x && io.MousePos.x <= text_input_max.x &&
                                       io.MousePos.y >= text_input_min.y && io.MousePos.y <= text_input_max.y);

            ImVec4 bg_color = text_input_hovered ? button_hover_color : sidebar_bg;
            ImVec4 border_col = text_input_hovered ? border_color_hover : border_color;

            draw_list->AddRectFilled(text_input_min, text_input_max, ImColor(bg_color), 6.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(text_input_min, text_input_max, ImColor(border_col), 6.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            static bool text_focused = false;
            static char* focused_text_input = nullptr;

            bool is_focused = (focused_text_input == color_buffer);
            bool was_clicked = false;

            if (text_input_hovered && ImGui::IsMouseClicked(0))
            {
                text_focused = true;
                focused_text_input = color_buffer;
                was_clicked = true;
            }

            if (is_focused)
            {
                for (int n = 0; n < io.InputQueueCharacters.Size && n < IM_ARRAYSIZE(io.InputQueueCharacters.Data); ++n)
                {
                    unsigned int c = static_cast<unsigned int>(io.InputQueueCharacters.Data[n]);
                    if (c > 0 && c < 0x10000)
                    {
                        size_t len = std::strlen(color_buffer);
                        if (len < buffer_size - 1)
                        {
                            color_buffer[len] = static_cast<char>(c);
                            color_buffer[len + 1] = '\0';
                        }
                    }
                }

                if (ImGui::IsKeyDown(ImGuiKey_Backspace) && std::strlen(color_buffer) > 0)
                {
                    static float last_backspace_time = 0.0f;
                    float current_time = ImGui::GetTime();
                    if ((current_time - last_backspace_time) > 0.05f)
                    {
                        color_buffer[std::strlen(color_buffer) - 1] = '\0';
                        last_backspace_time = current_time;
                    }
                }

                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    text_focused = false;
                    focused_text_input = nullptr;
                }
            }

            if (color_input_hovered && ImGui::IsMouseClicked(0))
            {
                text_focused = true;
                focused_text_input = color_buffer;
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 text_pos = RoundToPixel(ImVec2(text_input_min.x + 8.0f, text_input_min.y + 5.0f));
                if (std::strlen(color_buffer) > 0)
                    draw_list->AddText(g_ManropeFont, 12.0f, text_pos, ImColor(text_secondary), color_buffer);
                else
                    draw_list->AddText(g_ManropeFont, 12.0f, text_pos, ImColor(text_secondary), "#000000");

                if (is_focused)
                {
                    float cursor_x = text_input_min.x + 10.0f;
                    if (std::strlen(color_buffer) > 0)
                    {
                        ImVec2 text_size = g_ManropeFont->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, color_buffer);
                        cursor_x = text_input_min.x + 10.0f + text_size.x;
                    }
                    if (std::fmod(ImGui::GetTime(), 1.0f) < 0.5f)
                        draw_list->AddLine(ImVec2(cursor_x, text_input_min.y + 5.0f), ImVec2(cursor_x, text_input_min.y + 15.0f), ImColor(text_secondary), 1.0f);
                }
                ImGui::PopFont();
            }

            return was_clicked;
        }

        int CustomListBox(const char* label, const std::vector<std::string>& items, int selected_index, float width, float font_size = 12.0f)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

            float list_width = (width > 0.0f) ? width : ImGui::GetContentRegionAvail().x;
            float list_height = 120.0f;

            float label_height = 15.0f;
            if (g_ManropeFont && label && label[0] != '\0')
            {
                ImGui::PushFont(g_ManropeFont);
                ImVec2 label_pos = RoundToPixel(ImVec2(cursor_pos.x, cursor_pos.y - label_height));
                draw_list->AddText(g_ManropeFont, 11.0f, label_pos, ImColor(text_muted), label);
                ImGui::PopFont();
            }

            ImVec2 list_min = ImVec2(cursor_pos.x, cursor_pos.y);
            ImVec2 list_max = ImVec2(cursor_pos.x + list_width, cursor_pos.y + list_height);
            draw_list->AddRectFilled(list_min, list_max, ImColor(sidebar_bg), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(list_min, list_max, ImColor(border_color), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            ImGuiIO& io = ImGui::GetIO();
            int new_selected = selected_index;

            ImVec2 sample_text_size = g_ManropeFont ? g_ManropeFont->CalcTextSizeA(font_size, FLT_MAX, 0.0f, "Sample") : ImVec2(0, font_size);
            float item_height = 6.0f + sample_text_size.y + 6.0f;
            size_t visible_count = (size_t)(list_height / item_height);

            for (size_t i = 0; i < items.size() && i < visible_count; ++i)
            {
                ImVec2 item_min = ImVec2(list_min.x + 4.0f, list_min.y + 4.0f + i * item_height);
                ImVec2 item_max = ImVec2(list_max.x - 4.0f, list_min.y + 4.0f + (i + 1) * item_height);

                bool item_hovered = (io.MousePos.x >= item_min.x && io.MousePos.x <= item_max.x &&
                                     io.MousePos.y >= item_min.y && io.MousePos.y <= item_max.y);

                if ((int)i == selected_index)
                    draw_list->AddRectFilled(item_min, item_max, IM_COL32(0x4a, 0x9e, 0xff, 51), 8.0f, ImDrawFlags_RoundCornersAll);
                else if (item_hovered)
                    draw_list->AddRectFilled(item_min, item_max, ImColor(button_hover_color), 8.0f, ImDrawFlags_RoundCornersAll);

                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    ImVec4 text_col = ((int)i == selected_index) ? accent_color : text_secondary;
                    ImVec2 text_size = g_ManropeFont->CalcTextSizeA(font_size, FLT_MAX, 0.0f, items[i].c_str());
                    float text_y = item_min.y + (item_height - text_size.y) * 0.5f;
                    ImVec2 item_text_pos = RoundToPixel(ImVec2(item_min.x + 8.0f, text_y));
                    draw_list->AddText(g_ManropeFont, font_size, item_text_pos, ImColor(text_col), items[i].c_str());
                    ImGui::PopFont();
                }

                if (!IsAnyModalOpen() && item_hovered && ImGui::IsMouseClicked(0))
                    new_selected = static_cast<int>(i);
            }

            return new_selected;
        }

        void DrawIconCrosshair(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            draw_list->AddLine(ImVec2(center.x - half * 0.6f, center.y), ImVec2(center.x + half * 0.6f, center.y), color, line_width);
            draw_list->AddLine(ImVec2(center.x, center.y - half * 0.6f), ImVec2(center.x, center.y + half * 0.6f), color, line_width);
            draw_list->AddCircle(center, half * 0.4f, color, 64, 1.5f);
        }

        void DrawIconEye(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            draw_list->AddBezierCubic(ImVec2(center.x - half * 0.7f, center.y),
                                      ImVec2(center.x - half * 0.35f, center.y - half * 0.5f),
                                      ImVec2(center.x + half * 0.35f, center.y - half * 0.5f),
                                      ImVec2(center.x + half * 0.7f, center.y), color, line_width, 8);
            draw_list->AddBezierCubic(ImVec2(center.x + half * 0.7f, center.y),
                                      ImVec2(center.x + half * 0.35f, center.y + half * 0.5f),
                                      ImVec2(center.x - half * 0.35f, center.y + half * 0.5f),
                                      ImVec2(center.x - half * 0.7f, center.y), color, line_width, 8);
            draw_list->AddCircleFilled(center, half * 0.3f, color, 64);
        }

        void DrawIconZap(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            ImVec2 p1 = ImVec2(center.x - half * 0.3f, center.y - half * 0.7f);
            ImVec2 p2 = ImVec2(center.x + half * 0.2f, center.y - half * 0.2f);
            ImVec2 p3 = ImVec2(center.x - half * 0.1f, center.y);
            ImVec2 p4 = ImVec2(center.x + half * 0.3f, center.y + half * 0.7f);
            draw_list->AddLine(p1, p2, color, line_width);
            draw_list->AddLine(p2, p3, color, line_width);
            draw_list->AddLine(p3, p4, color, line_width);
        }

        void DrawIconPalette(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            ImVec2 min = ImVec2(center.x - half * 0.7f, center.y - half * 0.5f);
            ImVec2 max = ImVec2(center.x + half * 0.7f, center.y + half * 0.5f);
            draw_list->AddRect(min, max, color, 4.0f, 0, line_width);
            draw_list->AddLine(ImVec2(max.x, center.y), ImVec2(max.x + half * 0.3f, center.y + half * 0.3f), color, line_width);
            draw_list->AddCircleFilled(ImVec2(center.x - half * 0.3f, center.y - half * 0.2f), 2.0f, color, 32);
            draw_list->AddCircleFilled(ImVec2(center.x, center.y), 2.0f, color, 32);
            draw_list->AddCircleFilled(ImVec2(center.x + half * 0.3f, center.y - half * 0.2f), 2.0f, color, 32);
        }

        void DrawIconUsers2(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            draw_list->AddCircle(center, half * 0.35f, color, 64, line_width);
            draw_list->AddCircle(center, half * 0.2f, color, 64, line_width);
            draw_list->AddCircle(ImVec2(center.x + half * 0.45f, center.y - half * 0.2f), half * 0.25f, color, 64, line_width);
            draw_list->AddCircle(ImVec2(center.x + half * 0.45f, center.y - half * 0.2f), half * 0.12f, color, 64, line_width);
        }

        void DrawIconCog(ImDrawList* draw_list, ImVec2 center, ImColor color, float size = 14.0f)
        {
            float half = size * 0.5f;
            float line_width = 1.5f;
            draw_list->AddCircle(center, half * 0.4f, color, 32, line_width);
            draw_list->AddCircle(center, half * 0.2f, color, 32, line_width);
            for (int i = 0; i < 8; ++i)
            {
                float angle = (float)i / 8.0f * 3.14159265f * 2.0f;
                ImVec2 dir(std::cos(angle), std::sin(angle));
                draw_list->AddLine(ImVec2(center.x + dir.x * half * 0.4f, center.y + dir.y * half * 0.4f),
                                   ImVec2(center.x + dir.x * half * 0.6f, center.y + dir.y * half * 0.6f), color, line_width);
            }
        }

        void RenderBackground()
        {
            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            int screenHeight = GetSystemMetrics(SM_CYSCREEN);

            ImVec2 vp = ImGui::GetMainViewport()->Pos;
            ImVec2 vs = ImGui::GetMainViewport()->Size;

            ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

            int rects_x = (screenWidth / static_cast<int>(vs.x)) + 2;
            int rects_y = (screenHeight / static_cast<int>(vs.y)) + 2;

            for (int y = -1; y <= rects_y; ++y)
            {
                for (int x = -1; x <= rects_x; ++x)
                {
                    ImVec2 rect_min = ImVec2(static_cast<float>(x) * vs.x - vp.x, static_cast<float>(y) * vs.y - vp.y);
                    ImVec2 rect_max = ImVec2(static_cast<float>(x + 1) * vs.x - vp.x, static_cast<float>(y + 1) * vs.y - vp.y);

                    draw_list->AddRectFilled(rect_min, rect_max, ImColor(0, 0, 0, 153));

                    for (int layer = 0; layer < 2; ++layer)
                    {
                        float alpha = 0.02f * (2 - layer);
                        ImU32 blurColor = IM_COL32(0, 0, 0, static_cast<int>(alpha * 255.0f));
                        draw_list->AddRectFilled(rect_min, rect_max, blurColor);
                    }
                }
            }
        }

        void RenderLoadingScreen()
        {
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 vp = viewport->Pos;
            ImVec2 vs = viewport->Size;

            float base_width = 900.0f;
            float base_height = 550.0f;
            float scaled_width = base_width * g_MenuScale;
            float scaled_height = base_height * g_MenuScale;

            ImVec2 menu_pos = ImVec2(vp.x + (vs.x - scaled_width) * 0.5f + g_MenuOffsetX,
                                     vp.y + (vs.y - scaled_height) * 0.5f + g_MenuOffsetY);

            ImGui::SetNextWindowPos(menu_pos);
            ImGui::SetNextWindowSize(ImVec2(scaled_width, scaled_height));
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                     ImGuiWindowFlags_NoNav;

            if (ImGui::Begin("##LoadingScreen", nullptr, flags))
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                ImVec2 bg_min = menu_pos;
                ImVec2 bg_max = ImVec2(menu_pos.x + scaled_width, menu_pos.y + scaled_height);
                float rounding = 18.0f * g_MenuScale;
                ImVec2 shadow_offset = ImVec2(0.0f, 14.0f * g_MenuScale);
                ImVec2 shadow_min = ImVec2(bg_min.x + shadow_offset.x, bg_min.y + shadow_offset.y);
                ImVec2 shadow_max = ImVec2(bg_max.x + shadow_offset.x, bg_max.y + shadow_offset.y);
                draw_list->AddRectFilled(shadow_min, shadow_max, IM_COL32(0, 0, 0, 65), rounding + 6.0f, ImDrawFlags_RoundCornersAll);
                draw_list->AddRectFilled(bg_min, bg_max, ImColor(hexToVec4("040404")), rounding, ImDrawFlags_RoundCornersAll);
                draw_list->AddRect(bg_min, bg_max, ImColor(border_color), rounding, ImDrawFlags_RoundCornersAll, 1.0f);

                float spinner_radius = 36.0f * g_MenuScale;
                ImVec2 center = RoundToPixel(ImVec2(menu_pos.x + scaled_width * 0.5f,
                                                    menu_pos.y + scaled_height * 0.5f - 16.0f * g_MenuScale));

                float current_time = ImGui::GetTime();
                float rotation_speed = 0.6f;
                float angle_offset = current_time * rotation_speed * 2.0f * 3.14159f;

                float line_width = 4.5f * g_MenuScale;
                float arc_length = 3.14159f * 1.4f;
                int num_segments = 512;

                draw_list->AddCircle(center, spinner_radius, IM_COL32(255, 255, 255, 30), num_segments, line_width * 0.33f);

                float start_angle = angle_offset;
                float end_angle = angle_offset + arc_length;

                int gradient_steps = 512;
                float step_angle = arc_length / static_cast<float>(gradient_steps);
                int segments_per_step = 32;

                for (int i = 0; i < gradient_steps; ++i)
                {
                    float t = static_cast<float>(i) / static_cast<float>(gradient_steps);
                    float alpha = 1.0f - t;
                    alpha = powf(alpha, 1.5f);
                    alpha = std::clamp(alpha, 0.0f, 1.0f);

                    float seg_start = start_angle + i * step_angle;
                    float seg_end = start_angle + (i + 1) * step_angle;

                    draw_list->PathClear();
                    draw_list->PathArcTo(center, spinner_radius, seg_start, seg_end, segments_per_step);
                    draw_list->PathStroke(IM_COL32(255, 255, 255, static_cast<int>(alpha * 255.0f)), ImDrawFlags_None, line_width);
                }

                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    const char* wait_text = "Wait...";
                    ImVec2 text_size = g_ManropeFont->CalcTextSizeA(12.0f * g_MenuScale, FLT_MAX, 0.0f, wait_text);
                    ImVec2 text_pos = RoundToPixel(ImVec2(center.x - text_size.x * 0.5f,
                                                          center.y + spinner_radius + 25.0f * g_MenuScale));
                    draw_list->AddText(g_ManropeFont, 12.0f * g_MenuScale, text_pos, ImColor(text_secondary), wait_text);
                    ImGui::PopFont();
                }

                ImGui::End();
            }
        }

        void RenderModal(const char* title, const char* message, bool* is_open, bool* confirmed)
        {
            if (!*is_open)
                return;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 viewport_pos = viewport->Pos;
            ImVec2 viewport_size = viewport->Size;

            std::string window_id = std::string("##NMET_MODAL_") + title;

            ImGui::SetNextWindowPos(viewport_pos);
            ImGui::SetNextWindowSize(viewport_size);
            ImGui::SetNextWindowBgAlpha(0.0f);

            ImGuiWindowFlags modal_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;

            if (!ImGui::Begin(window_id.c_str(), nullptr, modal_flags))
            {
                ImGui::End();
                return;
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(viewport_pos, ImVec2(viewport_pos.x + viewport_size.x, viewport_pos.y + viewport_size.y), ImColor(0, 0, 0, 150));

            float scale = g_MenuScale;
            float modal_width = 380.0f * scale;
            float modal_height = 210.0f * scale;
            ImVec2 modal_pos = ImVec2(viewport_pos.x + (viewport_size.x - modal_width) * 0.5f,
                                      viewport_pos.y + (viewport_size.y - modal_height) * 0.5f);
            ImVec2 modal_min = modal_pos;
            ImVec2 modal_max = ImVec2(modal_pos.x + modal_width, modal_pos.y + modal_height);

            float rounding = 12.0f * scale;
            draw_list->AddRectFilled(modal_min, modal_max, ImColor(card_color), rounding, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(modal_min, modal_max, ImColor(border_color), rounding, ImDrawFlags_RoundCornersAll, 1.0f);

            const float content_padding = 24.0f * scale;
            const float button_height = 32.0f * scale;
            const float button_width = 120.0f * scale;
            const float button_spacing = 12.0f * scale;

            ImVec2 content_pos = ImVec2(modal_min.x + content_padding, modal_min.y + content_padding);
            ImVec2 content_max = ImVec2(modal_max.x - content_padding, modal_max.y - content_padding);

            if (g_ManropeFont)
                ImGui::PushFont(g_ManropeFont);
            ImGui::PushStyleColor(ImGuiCol_Text, text_primary);
            ImGui::SetCursorScreenPos(content_pos);
            ImGui::TextUnformatted(title);
            ImGui::PopStyleColor();

            ImVec2 after_title_cursor = ImGui::GetCursorScreenPos();
            after_title_cursor.y += 6.0f * scale;
            draw_list->AddLine(ImVec2(content_pos.x, after_title_cursor.y), ImVec2(content_max.x, after_title_cursor.y), ImColor(border_color), 1.0f);

            ImVec2 message_pos = ImVec2(content_pos.x, after_title_cursor.y + 12.0f * scale);
            ImGui::SetCursorScreenPos(message_pos);
            ImGui::PushStyleColor(ImGuiCol_Text, text_secondary);
            float wrap_pos = content_max.x - ImGui::GetWindowPos().x;
            ImGui::PushTextWrapPos(wrap_pos);
            ImGui::TextWrapped("%s", message);
            ImGui::PopTextWrapPos();
            ImGui::PopStyleColor();

            ImVec2 current_cursor = ImGui::GetCursorScreenPos();
            float buttons_y = std::max(current_cursor.y, content_max.y - button_height);

            ImGuiIO& io = ImGui::GetIO();

            const char* confirm_label = "Confirm";
            const char* cancel_label = "Cancel";
            ImFont* button_font = g_ManropeFont ? g_ManropeFont : ImGui::GetFont();
            float base_button_font_size = g_ManropeFont ? 12.0f : button_font->FontSize;
            float button_font_size = base_button_font_size * scale;

            ImVec2 confirm_min = ImVec2(content_max.x - button_width, buttons_y);
            ImVec2 confirm_max = ImVec2(content_max.x, buttons_y + button_height);
            bool confirm_hovered = io.MousePos.x >= confirm_min.x && io.MousePos.x <= confirm_max.x &&
                                   io.MousePos.y >= confirm_min.y && io.MousePos.y <= confirm_max.y;
            ImVec4 confirm_color = confirm_hovered ? button_hover_color : accent_color;
            draw_list->AddRectFilled(confirm_min, confirm_max, ImColor(confirm_color), 8.0f * scale, ImDrawFlags_RoundCornersAll);
            ImVec2 confirm_text_size = button_font->CalcTextSizeA(button_font_size, FLT_MAX, 0.0f, confirm_label);
            ImVec2 confirm_text_pos = RoundToPixel(ImVec2(confirm_min.x + (button_width - confirm_text_size.x) * 0.5f,
                                                          confirm_min.y + (button_height - confirm_text_size.y) * 0.5f));
            draw_list->AddText(button_font, button_font_size, confirm_text_pos, ImColor(text_primary), confirm_label);

            ImVec2 cancel_min = ImVec2(confirm_min.x - button_spacing - button_width, buttons_y);
            ImVec2 cancel_max = ImVec2(cancel_min.x + button_width, buttons_y + button_height);
            bool cancel_hovered = io.MousePos.x >= cancel_min.x && io.MousePos.x <= cancel_max.x &&
                                  io.MousePos.y >= cancel_min.y && io.MousePos.y <= cancel_max.y;
            ImVec4 cancel_bg = cancel_hovered ? button_hover_color : sidebar_bg;
            draw_list->AddRectFilled(cancel_min, cancel_max, ImColor(cancel_bg), 8.0f * scale, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(cancel_min, cancel_max, ImColor(border_color), 8.0f * scale, ImDrawFlags_RoundCornersAll, 1.0f);
            ImVec2 cancel_text_size = button_font->CalcTextSizeA(button_font_size, FLT_MAX, 0.0f, cancel_label);
            ImVec2 cancel_text_pos = RoundToPixel(ImVec2(cancel_min.x + (button_width - cancel_text_size.x) * 0.5f,
                                                         cancel_min.y + (button_height - cancel_text_size.y) * 0.5f));
            draw_list->AddText(button_font, button_font_size, cancel_text_pos, ImColor(text_secondary), cancel_label);

            bool confirm_clicked = confirm_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
            bool cancel_clicked = cancel_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);

            if (ImGui::IsKeyPressed(ImGuiKey_Enter))
                confirm_clicked = true;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                cancel_clicked = true;

            if (confirm_clicked)
            {
                if (confirmed)
                    *confirmed = true;
                *is_open = false;
            }
            else if (cancel_clicked)
            {
                if (confirmed)
                    *confirmed = false;
                *is_open = false;
            }

            if (g_ManropeFont)
                ImGui::PopFont();

            ImGui::End();
        }

        void RenderLoadConfigModal(bool* is_open, int* selected_index)
        {
            if (!*is_open)
                return;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("##NMET_LOAD_CONFIG", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 window_pos = viewport->Pos;
            ImVec2 window_size = viewport->Size;
            draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(0, 0, 0, 128));

            float modal_width = 400.0f;
            float modal_height = 350.0f;
            ImVec2 modal_pos = ImVec2(window_pos.x + (window_size.x - modal_width) * 0.5f,
                                      window_pos.y + (window_size.y - modal_height) * 0.5f);
            ImVec2 modal_min = modal_pos;
            ImVec2 modal_max = ImVec2(modal_pos.x + modal_width, modal_pos.y + modal_height);
            draw_list->AddRectFilled(modal_min, modal_max, ImColor(card_color), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(modal_min, modal_max, ImColor(border_color), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(modal_min.x + 16.0f, modal_min.y + 16.0f)), ImColor(text_primary), "Load Config");
                ImGui::PopFont();
            }

            float list_start_y = modal_min.y + 45.0f;
            float list_height = 220.0f;
            ImVec2 list_min = ImVec2(modal_min.x + 16.0f, list_start_y);
            ImVec2 list_max = ImVec2(modal_max.x - 16.0f, list_start_y + list_height);
            draw_list->AddRectFilled(list_min, list_max, ImColor(sidebar_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(list_min, list_max, ImColor(border_color), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            ImGuiIO& io = ImGui::GetIO();
            float item_height = 24.0f;
            for (size_t i = 0; i < g_MenuState.configList.size(); ++i)
            {
                ImVec2 item_min = ImVec2(list_min.x + 4.0f, list_min.y + 4.0f + i * item_height);
                ImVec2 item_max = ImVec2(list_max.x - 4.0f, list_min.y + 4.0f + (i + 1) * item_height);
                bool hovered = (io.MousePos.x >= item_min.x && io.MousePos.x <= item_max.x &&
                                io.MousePos.y >= item_min.y && io.MousePos.y <= item_max.y);
                if (static_cast<int>(i) == *selected_index)
                    draw_list->AddRectFilled(item_min, item_max, IM_COL32(0x4a, 0x9e, 0xff, 51), 8.0f, ImDrawFlags_RoundCornersAll);
                else if (hovered)
                    draw_list->AddRectFilled(item_min, item_max, ImColor(button_hover_color), 8.0f, ImDrawFlags_RoundCornersAll);
                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(item_min.x + 8.0f, item_min.y + 5.0f)), ImColor(text_secondary), g_MenuState.configList[i].c_str());
                    ImGui::PopFont();
                }
                if (hovered && ImGui::IsMouseClicked(0))
                    *selected_index = static_cast<int>(i);
            }

            float button_width = 90.0f;
            float button_height = 30.0f;
            float button_y = modal_max.y - button_height - 16.0f;
            float button_gap = 10.0f;

            ImVec2 load_min = ImVec2(modal_max.x - button_width * 2 - button_gap - 16.0f, button_y);
            ImVec2 load_max = ImVec2(load_min.x + button_width, load_min.y + button_height);
            bool load_hovered = (io.MousePos.x >= load_min.x && io.MousePos.x <= load_max.x &&
                                 io.MousePos.y >= load_min.y && io.MousePos.y <= load_max.y);
            ImVec4 load_bg = load_hovered ? button_hover_color : accent_color;
            draw_list->AddRectFilled(load_min, load_max, ImColor(load_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(load_min.x + 26.0f, load_min.y + 7.0f)), ImColor(text_primary), "Load");
                ImGui::PopFont();
            }

            ImVec2 cancel_min = ImVec2(modal_max.x - button_width - 16.0f, button_y);
            ImVec2 cancel_max = ImVec2(cancel_min.x + button_width, cancel_min.y + button_height);
            bool cancel_hovered = (io.MousePos.x >= cancel_min.x && io.MousePos.x <= cancel_max.x &&
                                   io.MousePos.y >= cancel_min.y && io.MousePos.y <= cancel_max.y);
            ImVec4 cancel_bg = cancel_hovered ? button_hover_color : sidebar_bg;
            draw_list->AddRectFilled(cancel_min, cancel_max, ImColor(cancel_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(cancel_min, cancel_max, ImColor(border_color), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(cancel_min.x + 16.0f, cancel_min.y + 7.0f)), ImColor(text_secondary), "Cancel");
                ImGui::PopFont();
            }

            if (load_hovered && ImGui::IsMouseClicked(0))
            {
                if (*selected_index >= 0 && *selected_index < static_cast<int>(g_MenuState.configList.size()))
                {
                    g_ConfigManager.LoadConfig();
                    SyncOptionsToMenu();
                }
                *is_open = false;
            }
            if (cancel_hovered && ImGui::IsMouseClicked(0))
                *is_open = false;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                *is_open = false;

            ImGui::End();
        }

        void RenderSaveConfigModal(bool* is_open, char* config_name, size_t name_size)
        {
            if (!*is_open)
                return;

            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("##NMET_SAVE_CONFIG", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                                      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 window_pos = viewport->Pos;
            ImVec2 window_size = viewport->Size;
            draw_list->AddRectFilled(window_pos, ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y), ImColor(0, 0, 0, 128));

            float modal_width = 350.0f;
            float modal_height = 180.0f;
            ImVec2 modal_pos = ImVec2(window_pos.x + (window_size.x - modal_width) * 0.5f,
                                      window_pos.y + (window_size.y - modal_height) * 0.5f);
            ImVec2 modal_min = modal_pos;
            ImVec2 modal_max = ImVec2(modal_pos.x + modal_width, modal_pos.y + modal_height);
            draw_list->AddRectFilled(modal_min, modal_max, ImColor(card_color), 10.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(modal_min, modal_max, ImColor(border_color), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(modal_min.x + 16.0f, modal_min.y + 16.0f)), ImColor(text_primary), "Save Config");
                ImGui::PopFont();
            }

            ImGuiIO& io = ImGui::GetIO();
            float input_y = modal_min.y + 60.0f;
            float input_height = 30.0f;
            ImVec2 input_min = ImVec2(modal_min.x + 16.0f, input_y);
            ImVec2 input_max = ImVec2(modal_max.x - 16.0f, input_y + input_height);
            bool input_hovered = (io.MousePos.x >= input_min.x && io.MousePos.x <= input_max.x &&
                                  io.MousePos.y >= input_min.y && io.MousePos.y <= input_max.y);
            ImVec4 input_bg = input_hovered ? button_hover_color : sidebar_bg;
            draw_list->AddRectFilled(input_min, input_max, ImColor(input_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(input_min, input_max, ImColor(border_color), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);

            static bool input_focused = false;
            if (input_hovered && ImGui::IsMouseClicked(0))
            {
                input_focused = true;
            }
            if (input_focused)
            {
                size_t len = strlen(config_name);

                if (io.InputQueueCharacters.Size > 0)
                {
                    for (int n = 0; n < io.InputQueueCharacters.Size; ++n)
                    {
                        unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                        if (c < 32 || c >= 127)
                            continue;
                        if (len + 1 < name_size)
                        {
                            config_name[len++] = static_cast<char>(c);
                            config_name[len] = '\0';
                        }
                    }
                    io.ClearInputCharacters();
                }

                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_V))
                {
                    if (const char* clip = ImGui::GetClipboardText())
                    {
                        while (*clip && len + 1 < name_size)
                        {
                            char c = *clip++;
                            if (c < 32 || c >= 127)
                                continue;
                            config_name[len++] = c;
                        }
                        config_name[len] = '\0';
                    }
                }

                if (ImGui::IsKeyPressed(ImGuiKey_Backspace, true) && len > 0)
                {
                    config_name[--len] = '\0';
                }

                if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_Escape))
                {
                    input_focused = false;
                }
            }

            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                const char* display = strlen(config_name) > 0 ? config_name : "Enter config name...";
                ImVec4 col = strlen(config_name) > 0 ? text_secondary : text_muted;
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(input_min.x + 8.0f, input_min.y + 7.0f)), ImColor(col), display);
                if (input_focused)
                {
                    ImVec2 text_size = g_ManropeFont->CalcTextSizeA(12.0f, FLT_MAX, 0.0f, config_name);
                    float cursor_x = input_min.x + 10.0f + text_size.x;
                    if (fmod(ImGui::GetTime(), 1.0f) < 0.5f)
                        draw_list->AddLine(ImVec2(cursor_x, input_min.y + 6.0f), ImVec2(cursor_x, input_min.y + input_height - 6.0f), ImColor(text_secondary), 1.0f);
                }
                ImGui::PopFont();
            }

            float button_width = 90.0f;
            float button_height = 30.0f;
            float button_y = modal_max.y - button_height - 16.0f;
            float button_gap = 10.0f;

            ImVec2 save_min = ImVec2(modal_max.x - button_width * 2 - button_gap - 16.0f, button_y);
            ImVec2 save_max = ImVec2(save_min.x + button_width, save_min.y + button_height);
            bool save_hovered = (io.MousePos.x >= save_min.x && io.MousePos.x <= save_max.x &&
                                 io.MousePos.y >= save_min.y && io.MousePos.y <= save_max.y);
            ImVec4 save_bg = save_hovered ? button_hover_color : accent_color;
            draw_list->AddRectFilled(save_min, save_max, ImColor(save_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(save_min.x + 22.0f, save_min.y + 7.0f)), ImColor(text_primary), "Save");
                ImGui::PopFont();
            }

            ImVec2 cancel_min = ImVec2(modal_max.x - button_width - 16.0f, button_y);
            ImVec2 cancel_max = ImVec2(cancel_min.x + button_width, cancel_min.y + button_height);
            bool cancel_hovered = (io.MousePos.x >= cancel_min.x && io.MousePos.x <= cancel_max.x &&
                                   io.MousePos.y >= cancel_min.y && io.MousePos.y <= cancel_max.y);
            ImVec4 cancel_bg = cancel_hovered ? button_hover_color : sidebar_bg;
            draw_list->AddRectFilled(cancel_min, cancel_max, ImColor(cancel_bg), 8.0f, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(cancel_min, cancel_max, ImColor(border_color), 8.0f, ImDrawFlags_RoundCornersAll, 1.0f);
            if (g_ManropeFont)
            {
                ImGui::PushFont(g_ManropeFont);
                draw_list->AddText(g_ManropeFont, 12.0f, RoundToPixel(ImVec2(cancel_min.x + 16.0f, cancel_min.y + 7.0f)), ImColor(text_secondary), "Cancel");
                ImGui::PopFont();
            }

            if (save_hovered && ImGui::IsMouseClicked(0))
            {
                if (strlen(config_name) > 0)
                {
                    SyncMenuToOptions();
                    g_ConfigManager.SaveConfig();
                }
                *is_open = false;
            }
            if (cancel_hovered && ImGui::IsMouseClicked(0))
                *is_open = false;
            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                *is_open = false;

            ImGui::End();
        }

        std::string VKCodeToString(int vkCode)
        {
            if (vkCode == 0) return "None";
            if (vkCode == VK_LBUTTON) return "LMB";
            if (vkCode == VK_RBUTTON) return "RMB";
            if (vkCode == VK_MBUTTON) return "MMB";
            if (vkCode == VK_XBUTTON1) return "X1";
            if (vkCode == VK_XBUTTON2) return "X2";
            if (vkCode >= VK_F1 && vkCode <= VK_F24)
            {
                char buf[8];
                sprintf_s(buf, "F%d", vkCode - VK_F1 + 1);
                return buf;
            }
            if (vkCode >= '0' && vkCode <= '9') return std::string(1, static_cast<char>(vkCode));
            if (vkCode >= 'A' && vkCode <= 'Z') return std::string(1, static_cast<char>(vkCode));
            if (vkCode == VK_SPACE) return "SPACE";
            if (vkCode == VK_RETURN) return "ENTER";
            if (vkCode == VK_BACK) return "BACKSPACE";
            if (vkCode == VK_TAB) return "TAB";
            if (vkCode == VK_LSHIFT) return "LSHIFT";
            if (vkCode == VK_RSHIFT) return "RSHIFT";
            if (vkCode == VK_SHIFT) return "SHIFT";
            if (vkCode == VK_LCONTROL) return "LCTRL";
            if (vkCode == VK_RCONTROL) return "RCTRL";
            if (vkCode == VK_CONTROL) return "CTRL";
            if (vkCode == VK_LMENU) return "LALT";
            if (vkCode == VK_RMENU) return "RALT";
            if (vkCode == VK_MENU) return "ALT";
            if (vkCode == VK_PAUSE) return "PAUSE";
            if (vkCode == VK_CAPITAL) return "CAPS";
            if (vkCode == VK_ESCAPE) return "ESC";
            if (vkCode == VK_PRIOR) return "PAGE UP";
            if (vkCode == VK_NEXT) return "PAGE DOWN";
            if (vkCode == VK_END) return "END";
            if (vkCode == VK_HOME) return "HOME";
            if (vkCode == VK_LEFT) return "LEFT";
            if (vkCode == VK_UP) return "UP";
            if (vkCode == VK_RIGHT) return "RIGHT";
            if (vkCode == VK_DOWN) return "DOWN";
            if (vkCode == VK_INSERT) return "INSERT";
            if (vkCode == VK_DELETE) return "DELETE";
            if (vkCode == VK_LWIN) return "LWIN";
            if (vkCode == VK_RWIN) return "RWIN";
            if (vkCode == VK_NUMPAD0) return "NUM 0";
            if (vkCode == VK_NUMPAD1) return "NUM 1";
            if (vkCode == VK_NUMPAD2) return "NUM 2";
            if (vkCode == VK_NUMPAD3) return "NUM 3";
            if (vkCode == VK_NUMPAD4) return "NUM 4";
            if (vkCode == VK_NUMPAD5) return "NUM 5";
            if (vkCode == VK_NUMPAD6) return "NUM 6";
            if (vkCode == VK_NUMPAD7) return "NUM 7";
            if (vkCode == VK_NUMPAD8) return "NUM 8";
            if (vkCode == VK_NUMPAD9) return "NUM 9";
            if (vkCode == VK_MULTIPLY) return "NUM *";
            if (vkCode == VK_ADD) return "NUM +";
            if (vkCode == VK_SUBTRACT) return "NUM -";
            if (vkCode == VK_DECIMAL) return "NUM .";
            if (vkCode == VK_DIVIDE) return "NUM /";
            char buf[16];
            sprintf_s(buf, "VK_%d", vkCode);
            return buf;
        }

        int StringToVKCode(const char* str)
        {
            if (!str || strcmp(str, "None") == 0) return 0;
            if (strcmp(str, "LMB") == 0) return VK_LBUTTON;
            if (strcmp(str, "RMB") == 0) return VK_RBUTTON;
            if (strcmp(str, "MMB") == 0) return VK_MBUTTON;
            if (strcmp(str, "X1") == 0) return VK_XBUTTON1;
            if (strcmp(str, "X2") == 0) return VK_XBUTTON2;
            if (str[0] == 'F' && strlen(str) > 1)
            {
                int fNum = atoi(str + 1);
                if (fNum >= 1 && fNum <= 24) return VK_F1 + fNum - 1;
            }
            if (strlen(str) == 1)
            {
                char c = str[0];
                if (c >= '0' && c <= '9') return c;
                if (c >= 'A' && c <= 'Z') return c;
            }
            if (strcmp(str, "SPACE") == 0) return VK_SPACE;
            if (strcmp(str, "ENTER") == 0) return VK_RETURN;
            if (strcmp(str, "BACKSPACE") == 0) return VK_BACK;
            if (strcmp(str, "TAB") == 0) return VK_TAB;
            if (strcmp(str, "LSHIFT") == 0) return VK_LSHIFT;
            if (strcmp(str, "RSHIFT") == 0) return VK_RSHIFT;
            if (strcmp(str, "SHIFT") == 0) return VK_SHIFT;
            if (strcmp(str, "LCTRL") == 0) return VK_LCONTROL;
            if (strcmp(str, "RCTRL") == 0) return VK_RCONTROL;
            if (strcmp(str, "CTRL") == 0) return VK_CONTROL;
            if (strcmp(str, "LALT") == 0) return VK_LMENU;
            if (strcmp(str, "RALT") == 0) return VK_RMENU;
            if (strcmp(str, "ALT") == 0) return VK_MENU;
            if (strcmp(str, "PAUSE") == 0) return VK_PAUSE;
            if (strcmp(str, "CAPS") == 0) return VK_CAPITAL;
            if (strcmp(str, "ESC") == 0) return VK_ESCAPE;
            if (strcmp(str, "PAGE UP") == 0) return VK_PRIOR;
            if (strcmp(str, "PAGE DOWN") == 0) return VK_NEXT;
            if (strcmp(str, "END") == 0) return VK_END;
            if (strcmp(str, "HOME") == 0) return VK_HOME;
            if (strcmp(str, "LEFT") == 0) return VK_LEFT;
            if (strcmp(str, "UP") == 0) return VK_UP;
            if (strcmp(str, "RIGHT") == 0) return VK_RIGHT;
            if (strcmp(str, "DOWN") == 0) return VK_DOWN;
            if (strcmp(str, "INSERT") == 0) return VK_INSERT;
            if (strcmp(str, "DELETE") == 0) return VK_DELETE;
            if (strcmp(str, "LWIN") == 0) return VK_LWIN;
            if (strcmp(str, "RWIN") == 0) return VK_RWIN;
            if (strcmp(str, "NUM 0") == 0) return VK_NUMPAD0;
            if (strcmp(str, "NUM 1") == 0) return VK_NUMPAD1;
            if (strcmp(str, "NUM 2") == 0) return VK_NUMPAD2;
            if (strcmp(str, "NUM 3") == 0) return VK_NUMPAD3;
            if (strcmp(str, "NUM 4") == 0) return VK_NUMPAD4;
            if (strcmp(str, "NUM 5") == 0) return VK_NUMPAD5;
            if (strcmp(str, "NUM 6") == 0) return VK_NUMPAD6;
            if (strcmp(str, "NUM 7") == 0) return VK_NUMPAD7;
            if (strcmp(str, "NUM 8") == 0) return VK_NUMPAD8;
            if (strcmp(str, "NUM 9") == 0) return VK_NUMPAD9;
            if (strcmp(str, "NUM *") == 0) return VK_MULTIPLY;
            if (strcmp(str, "NUM +") == 0) return VK_ADD;
            if (strcmp(str, "NUM -") == 0) return VK_SUBTRACT;
            if (strcmp(str, "NUM .") == 0) return VK_DECIMAL;
            if (strcmp(str, "NUM /") == 0) return VK_DIVIDE;
            return 0;
        }

        void UpdateKeybindCapture()
        {
            if (!g_KeybindCaptureState.active || !g_KeybindCaptureState.buffer)
                return;

            if (ImGui::GetFrameCount() == g_KeybindCaptureState.activationFrame)
                return;

            if (g_KeybindCaptureState.waitForRelease && g_KeybindCaptureState.suppressedVk != 0)
            {
                SHORT state = GetAsyncKeyState(g_KeybindCaptureState.suppressedVk);
                if ((state & 0x8000) == 0)
                {
                    g_KeybindCaptureState.waitForRelease = false;
                    g_KeybindCaptureState.ignoreNextVk = g_KeybindCaptureState.suppressedVk;
                    g_KeybindCaptureState.suppressedVk = 0;
                }
                else
                {
                    return;
                }
            }

            for (int vk = 1; vk <= 255; ++vk)
            {
                if (vk == g_KeybindCaptureState.ignoreNextVk)
                {
                    g_KeybindCaptureState.ignoreNextVk = 0;
                    continue;
                }

                SHORT state = GetAsyncKeyState(vk);
                if ((state & 0x1) == 0)
                    continue;

                if (vk == VK_DELETE)
                {
                    strncpy_s(g_KeybindCaptureState.buffer, g_KeybindCaptureState.bufferSize, "None", _TRUNCATE);
                    g_LastChangedKeybind = g_KeybindCaptureState.buffer;
                    CancelKeybindCapture();
                    return;
                }

                std::string key_name = VKCodeToString(vk);
                if (key_name.empty())
                    continue;

                strncpy_s(g_KeybindCaptureState.buffer, g_KeybindCaptureState.bufferSize, key_name.c_str(), _TRUNCATE);
                g_LastChangedKeybind = g_KeybindCaptureState.buffer;
                CancelKeybindCapture();
                return;
            }
        }

        void UpdatePlayersList()
        {
            if (!g_Fivem.IsInitialized())
                return;

            std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
            g_MenuState.playersList.clear();
            g_MenuState.friendsList.clear();

            Cheat::LocalPEDInfo localPlayer = g_Fivem.GetLocalPlayerInfo();
            std::string searchLower = g_MenuState.playerSearch;
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

            for (const auto& entity : entities)
            {
                if (entity.StaticInfo.bIsLocalPlayer)
                    continue;

                if (!g_MenuState.visualsShowNPCs && entity.StaticInfo.bIsNPC)
                    continue;

                std::string playerName = entity.StaticInfo.Name.empty() ? "Unknown" : entity.StaticInfo.Name;
                float distance = 0.0f;
                if (localPlayer.Ped && entity.StaticInfo.Ped)
                {
                    Vector3D localPos = localPlayer.WorldPos;
                    Vector3D entityPos = entity.Cordinates;
                    float dx = entityPos.x - localPos.x;
                    float dy = entityPos.y - localPos.y;
                    float dz = entityPos.z - localPos.z;
                    distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                }

                std::string displayName = playerName + " - " + std::to_string(static_cast<int>(distance)) + "m";
                std::string displayLower = displayName;
                std::transform(displayLower.begin(), displayLower.end(), displayLower.begin(), ::tolower);

                if (searchLower.empty() || displayLower.find(searchLower) != std::string::npos)
                {
                    g_MenuState.playersList.push_back(displayName);
                }

                if (g_Fivem.IsFriendByNetId(entity.StaticInfo.NetId))
                {
                    g_MenuState.friendsList.push_back(playerName);
                }
            }

            if (g_MenuState.selectedPlayerIndex >= static_cast<int>(g_MenuState.playersList.size()))
                g_MenuState.selectedPlayerIndex = 0;
            if (g_MenuState.selectedFriendIndex >= static_cast<int>(g_MenuState.friendsList.size()))
                g_MenuState.selectedFriendIndex = 0;
        }

        void RenderTabContent(ImVec2 panel_pos, ImVec2 panel_size)
        {
            auto applyChange = [](bool changed)
            {
                if (changed)
                    SyncMenuToOptions();
            };

            const char* hitBoxOptions[] = { "Head", "Neck", "Chest", "Pelvis" };
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            float edge_padding_left = 12.0f;
            float edge_padding_right = 40.0f;
            float edge_padding_top = 20.0f;
            float bottom_padding = 32.0f;
            ImVec2 safe_panel_pos = ImVec2(panel_pos.x + edge_padding_left, panel_pos.y + edge_padding_top);
            ImVec2 safe_panel_size = ImVec2(panel_size.x - edge_padding_left - edge_padding_right, panel_size.y - edge_padding_top * 2.0f - bottom_padding);

            auto resetCursor = [](float x, float y)
            {
                ImGui::SetCursorScreenPos(ImVec2(x, y));
            };

            if (strcmp(g_MenuState.activeTab, "silent") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "General");
                current_y += 18.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckboxWithKeybind("Enable SilentAim", &g_MenuState.silentEnable, g_MenuState.silentKeybind, sizeof(g_MenuState.silentKeybind), 100.0f, column_width));
                current_y += 35.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Show Fov", &g_MenuState.silentShowFov));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Prediction", &g_MenuState.silentPrediction));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Force Driver", &g_MenuState.silentForceDriver));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Alive Only", &g_MenuState.silentAliveOnly));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Visible Only", &g_MenuState.silentVisibleOnly));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Target Ped", &g_MenuState.silentTargetPed));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Always On", &g_MenuState.silentAlwaysOn));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Magic Bullet", &g_MenuState.silentMagicBullet));
                current_y += 28.0f;

                resetCursor(left_x, current_y);
                if (g_ShowSilentBypassModal)
                    g_MenuState.silentBypass = false;

                bool silentBypassChanged = CustomCheckbox("Silent Bypass", &g_MenuState.silentBypass);
                applyChange(silentBypassChanged);

                if (silentBypassChanged && g_MenuState.silentBypass && !g_ShowSilentBypassModal)
                {
                    g_ShowSilentBypassModal = true;
                    g_MenuState.silentBypass = false;
                }

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Settings");
                current_y += 18.0f + 18.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomDropdown("HitBox", hitBoxOptions[g_MenuState.silentHitBoxIndex], hitBoxOptions, 4, &g_MenuState.silentHitBoxIndex, column_width * 0.8f));
                current_y += 50.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Fov", &g_MenuState.silentFov, 0.0f, 300.0f, "px", 1.0f, column_width * 0.8f));
                current_y += 47.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Distance", &g_MenuState.silentDistance, 0.0f, 4000.0f, "m", 1.0f, column_width * 0.8f));
                current_y += 47.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Prediction Time", &g_MenuState.silentPredictionTime, 0.01f, 1.0f, "s", 0.01f, column_width * 0.8f));
                current_y += 47.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Misschance", &g_MenuState.silentMisschance, 0.0f, 100.0f, "%", 1.0f, column_width * 0.8f));
            }
            else if (strcmp(g_MenuState.activeTab, "aimbot") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "General");
                current_y += 18.0f;

                resetCursor(left_x, current_y);
                applyChange(CustomCheckboxWithKeybind("Enable Aimbot", &g_MenuState.aimbotEnable, g_MenuState.aimbotKeybind, sizeof(g_MenuState.aimbotKeybind), 100.0f, column_width));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Show Fov", &g_MenuState.aimbotShowFov));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Target Peds", &g_MenuState.aimbotTargetPeds));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Visible Only", &g_MenuState.aimbotVisibleOnly));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Alive Only", &g_MenuState.aimbotAliveOnly));

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Settings");
                current_y += 18.0f;

                resetCursor(right_x, current_y);
                applyChange(CustomDropdown("HitBox", hitBoxOptions[g_MenuState.aimbotHitBoxIndex], hitBoxOptions, 4, &g_MenuState.aimbotHitBoxIndex, column_width * 0.8f));
                current_y += 50.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Distance", &g_MenuState.aimbotDistance, 0.0f, 4000.0f, "m", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Fov Max", &g_MenuState.aimbotFovMax, 1.0f, 300.0f, "px", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Fov Min", &g_MenuState.aimbotFovMin, 0.0f, 180.0f, "px", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Smooth Horizontal", &g_MenuState.aimbotSmoothHorizontal, 0.0f, 100.0f, "%", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Smooth Vertical", &g_MenuState.aimbotSmoothVertical, 0.0f, 100.0f, "%", 1.0f, column_width * 0.8f));
            }
            else if (strcmp(g_MenuState.activeTab, "triggerbot") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "General");
                current_y += 18.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckboxWithKeybind("Enable TriggerBot", &g_MenuState.triggerEnable, g_MenuState.triggerKeybind, sizeof(g_MenuState.triggerKeybind), 100.0f, column_width));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Target Ped", &g_MenuState.triggerTargetPed));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Visible Check", &g_MenuState.triggerVisibleCheck));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomSlider("Max Distance", &g_MenuState.triggerMaxDistance, 0.0f, 4000.0f, "m", 1.0f, column_width * 0.8f));

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Settings");
                current_y += 18.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Reaction Time", &g_MenuState.triggerReactionTime, 0.0f, 300.0f, "ms", 1.0f, column_width * 0.8f));
            }
            else if (strcmp(g_MenuState.activeTab, "visuals") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "General");
                current_y += 18.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Enable ESP", &g_MenuState.visualsEnable));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Boxes", &g_MenuState.visualsBoxes));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Skeleton", &g_MenuState.visualsSkeleton));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Player Names", &g_MenuState.visualsPlayerNames));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Healthbar", &g_MenuState.visualsHealthbar));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Weapon", &g_MenuState.visualsWeapon));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Distance", &g_MenuState.visualsDistance));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Indicate Headshot", &g_MenuState.visualsIndicateHeadshot));

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Settings");
                current_y += 18.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Render Distance", &g_MenuState.visualsRenderDistance, 0.0f, 6000.0f, "m", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("Visible Only", &g_MenuState.visualsVisibleOnly));
                current_y += 28.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("Show Local Player", &g_MenuState.visualsShowLocalPlayer));
                current_y += 28.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("Show NPCs", &g_MenuState.visualsShowNPCs));
            }
            else if (strcmp(g_MenuState.activeTab, "players") == 0)
            {
                UpdatePlayersList();

                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "Player List");
                current_y += 18.0f;

                resetCursor(left_x, current_y);
                CustomInputText("", g_MenuState.playerSearch, sizeof(g_MenuState.playerSearch), "Search player...", column_width);
                current_y += 31.0f;

                resetCursor(left_x, current_y);
                g_MenuState.selectedPlayerIndex = CustomListBox("", g_MenuState.playersList, g_MenuState.selectedPlayerIndex, column_width);
                current_y += 120.0f + 12.0f;

                float button_width = (column_width - 6.0f) / 2.0f;
                resetCursor(left_x, current_y);
                if (CustomButton("Add to Friends", false, button_width))
                {
                    if (g_MenuState.selectedPlayerIndex >= 0 && g_MenuState.selectedPlayerIndex < static_cast<int>(g_MenuState.playersList.size()))
                    {
                        std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
                        int index = 0;
                        for (const auto& entity : entities)
                        {
                            if (entity.StaticInfo.bIsLocalPlayer)
                                continue;
                            if (!g_MenuState.visualsShowNPCs && entity.StaticInfo.bIsNPC)
                                continue;
                            if (index == g_MenuState.selectedPlayerIndex)
                            {
                                g_Fivem.AddFriendByNetId(entity.StaticInfo.NetId);
                                break;
                            }
                            ++index;
                        }
                    }
                }
                resetCursor(left_x + button_width + 6.0f, current_y);
                if (CustomButton("Remove Friend", false, button_width))
                {
                    if (g_MenuState.selectedPlayerIndex >= 0 && g_MenuState.selectedPlayerIndex < static_cast<int>(g_MenuState.playersList.size()))
                    {
                        std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
                        int index = 0;
                        for (const auto& entity : entities)
                        {
                            if (entity.StaticInfo.bIsLocalPlayer)
                                continue;
                            if (!g_MenuState.visualsShowNPCs && entity.StaticInfo.bIsNPC)
                                continue;
                            if (index == g_MenuState.selectedPlayerIndex)
                            {
                                g_Fivem.RemoveFriendByNetId(entity.StaticInfo.NetId);
                                break;
                            }
                            ++index;
                        }
                    }
                }
                current_y += 23.0f + 12.0f;

                resetCursor(left_x, current_y);
                if (CustomButton("Teleport", false, column_width))
                {
                    if (g_MenuState.selectedPlayerIndex >= 0 && g_MenuState.selectedPlayerIndex < static_cast<int>(g_MenuState.playersList.size()))
                    {
                        std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
                        int index = 0;
                        for (const auto& entity : entities)
                        {
                            if (entity.StaticInfo.bIsLocalPlayer)
                                continue;
                            if (!g_MenuState.visualsShowNPCs && entity.StaticInfo.bIsNPC)
                                continue;
                            if (index == g_MenuState.selectedPlayerIndex)
                            {
                                Cheat::Exploits::TeleportToPlayer(entity);
                                break;
                            }
                            ++index;
                        }
                    }
                }

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Friends List");
                current_y += 18.0f;

                resetCursor(right_x, current_y);
                g_MenuState.selectedFriendIndex = CustomListBox("", g_MenuState.friendsList, g_MenuState.selectedFriendIndex, column_width);
                current_y += 120.0f + 12.0f;

                resetCursor(right_x, current_y);
                if (CustomButton("Teleport to Friend", false, column_width))
                {
                    if (g_MenuState.selectedFriendIndex >= 0 && g_MenuState.selectedFriendIndex < static_cast<int>(g_MenuState.friendsList.size()))
                    {
                        std::string friendName = g_MenuState.friendsList[g_MenuState.selectedFriendIndex];
                        std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
                        for (const auto& entity : entities)
                        {
                            if (entity.StaticInfo.Name == friendName && g_Fivem.IsFriendByNetId(entity.StaticInfo.NetId))
                            {
                                Cheat::Exploits::TeleportToPlayer(entity);
                                break;
                            }
                        }
                    }
                }
                current_y += 23.0f + 12.0f;

                resetCursor(right_x, current_y);
                if (CustomButton("Remove Friend", false, column_width))
                {
                    if (g_MenuState.selectedFriendIndex >= 0 && g_MenuState.selectedFriendIndex < static_cast<int>(g_MenuState.friendsList.size()))
                    {
                        std::string friendName = g_MenuState.friendsList[g_MenuState.selectedFriendIndex];
                        std::vector<Cheat::Entity> entities = g_Fivem.GetEntitiyListSafe();
                        for (const auto& entity : entities)
                        {
                            if (entity.StaticInfo.Name == friendName && g_Fivem.IsFriendByNetId(entity.StaticInfo.NetId))
                            {
                                g_Fivem.RemoveFriendByNetId(entity.StaticInfo.NetId);
                                break;
                            }
                        }
                    }
                }
            }
            else if (strcmp(g_MenuState.activeTab, "colors") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "ESP Colors");
                current_y += 18.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Box Color", g_MenuState.boxColor, sizeof(g_MenuState.boxColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Box Glow Color", g_MenuState.boxGlowColor, sizeof(g_MenuState.boxGlowColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Skeleton Color", g_MenuState.skeletonColor, sizeof(g_MenuState.skeletonColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Name Color", g_MenuState.nameColor, sizeof(g_MenuState.nameColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Weapon Color", g_MenuState.weaponColor, sizeof(g_MenuState.weaponColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Distance Color", g_MenuState.distanceColor, sizeof(g_MenuState.distanceColor)));
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomColorPicker("Snapline Color", g_MenuState.snaplineColor, sizeof(g_MenuState.snaplineColor)));

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Misc Colors");
                current_y += 18.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Aimbot FOV Color", g_MenuState.aimbotFovColor, sizeof(g_MenuState.aimbotFovColor)));
                current_y += 35.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Silent Aim FOV Color", g_MenuState.silentFovColor, sizeof(g_MenuState.silentFovColor)));
                current_y += 35.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Health Bar Color", g_MenuState.healthBarColor, sizeof(g_MenuState.healthBarColor)));
                current_y += 35.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Health Bar Low Color", g_MenuState.healthBarLowColor, sizeof(g_MenuState.healthBarLowColor)));
                current_y += 35.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Armor Bar Color", g_MenuState.armorBarColor, sizeof(g_MenuState.armorBarColor)));
                current_y += 35.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomColorPicker("Vehicle Color", g_MenuState.vehicleColor, sizeof(g_MenuState.vehicleColor)));
            }
            else if (strcmp(g_MenuState.activeTab, "settings") == 0)
            {
                float panel_padding = 16.0f;
                float column_gap = 12.0f;
                float column_width = (safe_panel_size.x - panel_padding * 2 - column_gap) / 2.0f;
                float start_y = safe_panel_pos.y + panel_padding;
                float current_y = start_y;

                float left_x = safe_panel_pos.x + panel_padding;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "Config System");
                current_y += 18.0f;
                resetCursor(left_x, current_y);
                if (CustomButton("Load Config", false, column_width))
                    g_ShowLoadConfigModal = true;
                current_y += 35.0f;
                resetCursor(left_x, current_y);
                if (CustomButton("Save Config", false, column_width))
                {
                    g_ShowSaveConfigModal = true;
                    g_MenuState.saveConfigName[0] = '\0';
                }
                current_y += 47.0f;
                DrawSectionTitle(draw_list, ImVec2(left_x, current_y), "Extra");
                current_y += 18.0f;
                resetCursor(left_x, current_y + 15.0f);
                applyChange(CustomKeybind("Menu Key", g_MenuState.menuKey, sizeof(g_MenuState.menuKey)));
                current_y += 50.0f;
                resetCursor(left_x, current_y + 15.0f);
                applyChange(CustomKeybind("Panic Key", g_MenuState.panicKey, sizeof(g_MenuState.panicKey)));
                current_y += 50.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Stream Mode", &g_MenuState.streamMode));
                current_y += 28.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomSlider("Update Delay", &g_MenuState.updateDelay, 0.0f, 100.0f, "ms", 1.0f, column_width * 0.8f));
                current_y += 47.0f;
                resetCursor(left_x, current_y);
                applyChange(CustomCheckbox("Destruct Mode", &g_MenuState.bypassDestructMode));
                current_y += 28.0f;
                float button_width = (column_width - 12.0f) / 2.0f;
                resetCursor(left_x, current_y);
                if (CustomButton("Unload", true, button_width))
                {
                    g_Options.Misc.Bypass.Destruct = g_MenuState.bypassDestructMode;
                    if (!Bypass::g_BypassInProgress)
                        Bypass::RunAllBypass();
                }
                resetCursor(left_x + button_width + 12.0f, current_y);
                if (CustomButton("Unload + Destruct", true, button_width))
                {
                    g_MenuState.bypassDestructMode = true;
                    g_Options.Misc.Bypass.Destruct = true;
                    if (!Bypass::g_BypassInProgress)
                        Bypass::RunAllBypass();
                }

                float right_x = safe_panel_pos.x + panel_padding + column_width + column_gap;
                current_y = start_y;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Miscellaneous");
                current_y += 18.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("Noclip", &g_MenuState.noclip));
                current_y += 28.0f;
                resetCursor(right_x, current_y + 15.0f);
                applyChange(CustomKeybind("Noclip Key", g_MenuState.noclipKey, sizeof(g_MenuState.noclipKey)));
                current_y += 50.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomSlider("Noclip Speed", &g_MenuState.noclipSpeed, 0.0f, 100.0f, "m/s", 1.0f, column_width * 0.8f));
                
                current_y += 47.0f;
                DrawSectionTitle(draw_list, ImVec2(right_x, current_y), "Weapon Config");
                current_y += 18.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("No Recoil", &g_MenuState.miscNoRecoil));
                current_y += 28.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("No Spread", &g_MenuState.miscNoSpread));
                current_y += 28.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("No Reload", &g_MenuState.miscNoReload));
                current_y += 28.0f;
                resetCursor(right_x, current_y);
                applyChange(CustomCheckbox("Start Health", &g_MenuState.miscStartHealth));
                current_y += 28.0f;
                // Using float slider/drag for health amount, casting to int in sync
                static float healthAmt = 200.0f;
                healthAmt = static_cast<float>(g_MenuState.miscHealthAmount);
                resetCursor(right_x, current_y);
                if (CustomSlider("Health Amount", &healthAmt, 0.0f, 200.0f, "hp", 1.0f, column_width * 0.8f)) {
                     g_MenuState.miscHealthAmount = static_cast<int>(healthAmt);
                     applyChange(true);
                }
                current_y += 47.0f;
            }
        }

        void RenderMenu()
        {
            ImVec2 vp = ImGui::GetMainViewport()->Pos;
            ImVec2 vs = ImGui::GetMainViewport()->Size;

            UpdateKeybindCapture();

            float menu_width = 950.0f; // Slightly increased for balance
            float menu_height = 600.0f;

            float scaled_width = menu_width * g_MenuScale;
            float scaled_height = menu_height * g_MenuScale;

            ImVec2 menu_pos = ImVec2((vs.x - scaled_width) * 0.5f + g_MenuOffsetX,
                                     (vs.y - scaled_height) * 0.5f + g_MenuOffsetY);

            ImGui::SetNextWindowPos(menu_pos);
            ImGui::SetNextWindowSize(ImVec2(scaled_width, scaled_height));

            ImGuiWindowFlags menu_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                           ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                           ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                           ImGuiWindowFlags_NoNav;

            if (ImGui::Begin("##menu", nullptr, menu_flags))
            {
                ImGui::SetWindowFontScale(1.10f); // Increased to 1.10f per request
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 window_pos = ImGui::GetWindowPos();
                ImVec2 window_size = ImGui::GetWindowSize();

                ImVec2 container_min = window_pos;
                ImVec2 container_max = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
                float container_rounding = 18.0f * g_MenuScale;

                ImVec2 shadow_offset = ImVec2(0.0f, 14.0f * g_MenuScale);
                ImVec2 shadow_min = ImVec2(container_min.x + shadow_offset.x, container_min.y + shadow_offset.y);
                ImVec2 shadow_max = ImVec2(container_max.x + shadow_offset.x, container_max.y + shadow_offset.y);
                draw_list->AddRectFilled(shadow_min, shadow_max, IM_COL32(0, 0, 0, 65), container_rounding + 6.0f, ImDrawFlags_RoundCornersAll);

                draw_list->AddRectFilled(container_min, container_max, ImColor(hexToVec4("040404")), container_rounding, ImDrawFlags_RoundCornersAll);
                draw_list->AddRect(container_min, container_max, ImColor(border_color), container_rounding, ImDrawFlags_RoundCornersAll, 1.0f);

                ImVec2 header_min = container_min;
                float header_height = 30.0f;
                ImVec2 header_max = ImVec2(container_max.x, container_min.y + header_height);
                draw_list->AddRectFilled(header_min, header_max, ImColor(sidebar_bg), container_rounding, ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersTopRight);
                draw_list->AddLine(ImVec2(header_min.x + 1.0f, header_max.y), ImVec2(header_max.x - 1.0f, header_max.y), ImColor(border_color), 1.0f);

                float header_center_y = header_min.y + header_height * 0.5f;
                float logo_size = 18.0f;
                float text_y = header_min.y + (header_height - 12.0f) * 0.5f + 1.5f;
                float current_x = header_min.x + 14.0f;

                if (g_logoTexture)
                {
                    float logo_display_size = 22.0f; // Reduced from 28.0f (was too huge)
                    float logo_y = header_center_y - logo_display_size * 0.5f + 3.0f; // Added + 2.0f offset downwards
                    ImVec2 logo_pos = RoundToPixel(ImVec2(current_x, logo_y));
                    ImVec2 logo_end = ImVec2(logo_pos.x + logo_display_size, logo_pos.y + logo_display_size);
                    draw_list->AddImage(g_logoTexture, logo_pos, logo_end);
                    current_x += logo_display_size + 12.0f;
                }

                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    ImVec2 text_pos = RoundToPixel(ImVec2(current_x, text_y));
                    // Reduced to 18.0f to be proportional to validation
                    draw_list->AddText(g_ManropeFont, 18.0f, RoundToPixel(ImVec2(text_pos.x, text_pos.y - 1.0f)), ImColor(text_primary), "krakun.ai"); 
                    ImGui::PopFont();
                }

                ImVec2 sidebar_min = ImVec2(container_min.x, header_max.y);
                float sidebar_width_val = 210.0f; // Balanced width
                ImVec2 sidebar_max = ImVec2(container_min.x + sidebar_width_val, container_max.y);
                draw_list->AddRectFilled(sidebar_min, sidebar_max, ImColor(sidebar_bg), container_rounding, ImDrawFlags_RoundCornersBottomLeft);
                draw_list->AddLine(ImVec2(sidebar_max.x, sidebar_min.y + 1.0f), ImVec2(sidebar_max.x, sidebar_max.y - 1.0f), ImColor(border_color), 1.0f);

                const char* tab_labels[] = { "Silent", "Aimbot", "TriggerBot", "Visuals", "Players", "Colors", "Settings" };
                const char* tab_ids[] = { "silent", "aimbot", "triggerbot", "visuals", "players", "colors", "settings" };

                float tab_start_y = sidebar_min.y + 12.0f;
                float tab_width = sidebar_width_val - 16.0f;
                float tab_height = 28.0f;
                float tab_gap = 3.0f;

                for (int i = 0; i < 7; ++i)
                {
                    bool is_active = (strcmp(g_MenuState.activeTab, tab_ids[i]) == 0);

                    ImVec2 tab_min = ImVec2(sidebar_min.x + 8.0f, tab_start_y + i * (tab_height + tab_gap));
                    ImVec2 tab_max = ImVec2(tab_min.x + tab_width, tab_min.y + tab_height);

		ImGuiIO& io = ImGui::GetIO();
                    bool is_hovered = (io.MousePos.x >= tab_min.x && io.MousePos.x <= tab_max.x &&
                                       io.MousePos.y >= tab_min.y && io.MousePos.y <= tab_max.y);

                    if (is_active)
                    {
                        draw_list->AddRectFilled(tab_min, tab_max, ImColor(button_hover_color), 10.0f, ImDrawFlags_RoundCornersAll);
                        draw_list->AddRect(tab_min, tab_max, ImColor(border_color_hover), 10.0f, ImDrawFlags_RoundCornersAll, 1.0f);
                    }
                    else if (is_hovered)
                    {
                        draw_list->AddRectFilled(tab_min, tab_max, ImColor(button_hover_color), 10.0f, ImDrawFlags_RoundCornersAll);
                    }

                    if (is_active)
                    {
                        ImVec2 indicator_min = tab_min;
                        ImVec2 indicator_max = ImVec2(tab_min.x + 3.0f, tab_max.y);
                        int gradient_sections = 30;
                        float section_height = (indicator_max.y - indicator_min.y) / gradient_sections;

                        for (int s = 0; s < gradient_sections; ++s)
                        {
                            float t = static_cast<float>(s) / static_cast<float>(gradient_sections - 1);
                            ImU32 col = IM_COL32(
                                static_cast<int>(0x4a + (0x1e - 0x4a) * t),
                                static_cast<int>(0x9e + (0x40 - 0x9e) * t),
                                static_cast<int>(0xff + (0xaf - 0xff) * t),
                                255);

                            ImVec2 section_min = ImVec2(indicator_min.x, indicator_min.y + s * section_height);
                            ImVec2 section_max = ImVec2(indicator_max.x, indicator_min.y + (s + 1) * section_height);

                            ImDrawFlags round_flags = ImDrawFlags_None;
                            if (s == 0)
                                round_flags = ImDrawFlags_RoundCornersTopRight;
                            else if (s == gradient_sections - 1)
                                round_flags = ImDrawFlags_RoundCornersBottomRight;

                            if (round_flags != ImDrawFlags_None)
                                draw_list->AddRectFilled(section_min, section_max, col, 10.0f, round_flags);
                            else
                                draw_list->AddRectFilled(section_min, section_max, col);
                        }
                    }

                    float icon_x = tab_min.x + 10.0f;
                    float icon_center_y = tab_min.y + tab_height * 0.5f;
                    ImVec2 icon_center = ImVec2(icon_x + 7.0f, icon_center_y);

                    if (strcmp(tab_ids[i], "silent") == 0)
                    {
                        if (g_crosshairTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_crosshairTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconCrosshair(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "aimbot") == 0)
                    {
                        if (g_eyeTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_eyeTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconEye(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "triggerbot") == 0)
                    {
                        if (g_zapTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_zapTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconZap(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "visuals") == 0)
                    {
                        if (g_paletteTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_paletteTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconPalette(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "players") == 0)
                    {
                        if (g_userroundTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_userroundTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconUsers2(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "settings") == 0)
                    {
                        if (g_settingsTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_settingsTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconCog(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }
                    else if (strcmp(tab_ids[i], "colors") == 0)
                    {
                        if (g_paletteTexture)
                        {
                            float icon_size = 14.0f;
                            ImVec2 icon_min = ImVec2(icon_center.x - icon_size * 0.5f, icon_center.y - icon_size * 0.5f);
                            ImVec2 icon_max = ImVec2(icon_center.x + icon_size * 0.5f, icon_center.y + icon_size * 0.5f);
                            draw_list->AddImage(g_paletteTexture, icon_min, icon_max, ImVec2(0, 0), ImVec2(1, 1), IM_COL32_WHITE);
                        }
                        else
                        {
                            DrawIconPalette(draw_list, icon_center, ImColor(text_secondary), 14.0f);
                        }
                    }

                    if (g_ManropeFont)
                    {
                        ImGui::PushFont(g_ManropeFont);
                        ImVec4 text_col;
                        if (is_active)
                            text_col = accent_color;
                        else if (is_hovered)
                            text_col = hexToVec4("d1d5db");
                        else
                            text_col = text_muted;

                        float text_size = 14.0f;
                        float text_y = tab_min.y + (tab_height - text_size) * 0.5f;
                        float text_x = icon_x + 14.0f + 8.0f;
                        ImVec2 tab_text_pos = RoundToPixel(ImVec2(text_x, text_y));
                        draw_list->AddText(g_ManropeFont, text_size, tab_text_pos, ImColor(text_col), tab_labels[i]);
                        ImGui::PopFont();
                    }

                    if (is_hovered && ImGui::IsMouseClicked(0))
                        strncpy_s(g_MenuState.activeTab, tab_ids[i], _TRUNCATE);
                }

                // License Info at bottom of sidebar
                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    
                    int licenseDaysRemaining = 0;
                    std::string username = "User";
                    if (g_API) {
                        const auto& session = g_API->GetCurrentSession();
                        if (session.license_days_remaining >= 0) {
                            licenseDaysRemaining = session.license_days_remaining;
                        }
                        if (!session.username.empty()) {
                            username = session.username;
                        }
                    }

                    // Draw Username
                    float user_y = container_max.y - 45.0f;
                    float padding_x = 16.0f;
                    ImVec2 user_pos = RoundToPixel(ImVec2(sidebar_min.x + padding_x, user_y));
                    draw_list->AddText(g_ManropeFont, 16.0f, user_pos, ImColor(text_primary), username.c_str());

                    // Draw Expiry
                    char expiresValue[32];
                    sprintf_s(expiresValue, "%d days left", licenseDaysRemaining);
                    ImVec2 expires_pos = RoundToPixel(ImVec2(sidebar_min.x + padding_x, user_y + 18.0f)); // Adjusted Y for larger text
                    draw_list->AddText(g_ManropeFont, 14.0f, expires_pos, ImColor(text_muted), expiresValue);

                    ImGui::PopFont();
                }

                ImVec2 panel_min = ImVec2(sidebar_max.x, header_max.y);
                ImVec2 panel_max = ImVec2(container_max.x, container_max.y);
                draw_list->AddRectFilled(panel_min, panel_max, ImColor(panel_bg));

                RenderTabContent(panel_min, ImVec2(panel_max.x - panel_min.x, panel_max.y - panel_min.y));

                if (g_ShowSilentBypassModal)
                {
                    static bool modal_confirmed = false;
                    RenderModal("Silent Bypass", "Enable this bypass only after you are fully loaded into the server. Confirm to proceed or cancel to keep it disabled.", &g_ShowSilentBypassModal, &modal_confirmed);
                    if (modal_confirmed && !g_ShowSilentBypassModal)
                    {
                        g_MenuState.silentBypass = true;
                        SyncMenuToOptions();
                        modal_confirmed = false;
                    }
                }

                if (g_ShowLoadConfigModal)
                    RenderLoadConfigModal(&g_ShowLoadConfigModal, &g_MenuState.selectedConfigIndex);

        if (g_ShowSaveConfigModal)
                    RenderSaveConfigModal(&g_ShowSaveConfigModal, g_MenuState.saveConfigName, sizeof(g_MenuState.saveConfigName));
            }
            ImGui::End();
        }

        void SyncOptionsToMenu()
        {
            auto& opts = g_Options;

            g_MenuState.silentEnable = opts.LegitBot.SilentAim.Enabled;
            g_MenuState.silentShowFov = opts.Misc.Screen.ShowSilentAimFov;
            g_MenuState.silentPrediction = opts.LegitBot.SilentAim.PredictionEnabled;
            g_MenuState.silentForceDriver = opts.LegitBot.SilentAim.ForceDriver;
            g_MenuState.silentAliveOnly = opts.LegitBot.SilentAim.AliveOnly;
            g_MenuState.silentVisibleOnly = opts.LegitBot.SilentAim.VisibleCheck;
            g_MenuState.silentTargetPed = opts.LegitBot.SilentAim.ShotNPC;
            g_MenuState.silentAlwaysOn = opts.LegitBot.SilentAim.AlwaysOn;
            g_MenuState.silentBypass = opts.LegitBot.SilentAim.SilentBypass;
            g_MenuState.silentBypassV2 = opts.LegitBot.SilentAim.SilentBypassV2;
            g_MenuState.silentMagicBullet = opts.LegitBot.SilentAim.MagicBullet;
            g_MenuState.silentFov = static_cast<float>(opts.LegitBot.SilentAim.Fov);
            g_MenuState.silentDistance = static_cast<float>(opts.LegitBot.SilentAim.MaxDistance);
            g_MenuState.silentPredictionTime = opts.LegitBot.SilentAim.PredictionTime;
            g_MenuState.silentMisschance = static_cast<float>(opts.LegitBot.SilentAim.MissChance);
            g_MenuState.silentHitBoxIndex = opts.LegitBot.SilentAim.HitBox;

            g_MenuState.aimbotEnable = opts.LegitBot.AimBot.Enabled;
            g_MenuState.aimbotShowFov = opts.Misc.Screen.ShowAimbotFov;
            g_MenuState.aimbotTargetPeds = opts.LegitBot.AimBot.TargetNPC;
            g_MenuState.aimbotVisibleOnly = opts.LegitBot.AimBot.VisibleCheck;
            g_MenuState.aimbotDistance = static_cast<float>(opts.LegitBot.AimBot.MaxDistance);
            g_MenuState.aimbotFovMax = static_cast<float>(opts.LegitBot.AimBot.FOV);
            // g_MenuState.aimbotFovMin = static_cast<float>(opts.LegitBot.AimBot.FOV); // Decoupled per user request
            g_MenuState.aimbotSmoothHorizontal = static_cast<float>(opts.LegitBot.AimBot.SmoothHorizontal);
            g_MenuState.aimbotSmoothVertical = static_cast<float>(opts.LegitBot.AimBot.SmoothVertical);
            g_MenuState.aimbotHitBoxIndex = opts.LegitBot.AimBot.HitBox;

            g_MenuState.triggerEnable = opts.LegitBot.Trigger.Enabled;
            g_MenuState.triggerTargetPed = opts.LegitBot.Trigger.ShotNPC;
            g_MenuState.triggerVisibleCheck = opts.LegitBot.Trigger.VisibleCheck;
            g_MenuState.triggerMaxDistance = static_cast<float>(opts.LegitBot.Trigger.MaxDistance);
            g_MenuState.triggerReactionTime = static_cast<float>(opts.LegitBot.Trigger.ReactionTime);

            g_MenuState.visualsEnable = opts.Visuals.ESP.Players.Enabled;
            g_MenuState.visualsBoxes = opts.Visuals.ESP.Players.Box;
            g_MenuState.visualsSkeleton = opts.Visuals.ESP.Players.Skeleton;
            g_MenuState.visualsPlayerNames = opts.Visuals.ESP.Players.Name;
            g_MenuState.visualsHealthbar = opts.Visuals.ESP.Players.HealthBar;
            g_MenuState.visualsWeapon = opts.Visuals.ESP.Players.WeaponName;
            g_MenuState.visualsDistance = opts.Visuals.ESP.Players.Distance;
            g_MenuState.visualsRenderDistance = static_cast<float>(opts.Visuals.ESP.Players.RenderDistance);
            g_MenuState.visualsVisibleOnly = opts.Visuals.ESP.Players.VisibleOnly;
            g_MenuState.visualsShowLocalPlayer = opts.Visuals.ESP.Players.ShowLocalPlayer;
            g_MenuState.visualsShowNPCs = opts.Visuals.ESP.Players.ShowNPCs;

            g_MenuState.noclip = opts.Misc.Exploits.LocalPlayer.Noclip;
            g_MenuState.noclipSpeed = static_cast<float>(opts.Misc.Exploits.LocalPlayer.NoclipSpeed);
            g_MenuState.streamMode = opts.General.CaptureBypass;
            g_MenuState.streamMode = opts.General.CaptureBypass;
            g_MenuState.bypassDestructMode = opts.Misc.Bypass.Destruct;
            
            g_MenuState.miscNoRecoil = opts.Misc.Exploits.LocalPlayer.norecoil;
            g_MenuState.miscNoSpread = opts.Misc.Exploits.LocalPlayer.nospread;
            g_MenuState.miscNoReload = opts.Misc.Exploits.LocalPlayer.noreload;
            g_MenuState.miscStartHealth = opts.Misc.Exploits.LocalPlayer.Start_Health;
            g_MenuState.miscHealthAmount = opts.Misc.Exploits.LocalPlayer.health_ammount;

            auto vecToHex = [](float* col) -> std::string {
                char hex[32];
                sprintf_s(hex, "#%02x%02x%02x",
                    (int)(col[0] * 255.0f),
                    (int)(col[1] * 255.0f),
                    (int)(col[2] * 255.0f));
                return std::string(hex);
            };

            strcpy_s(g_MenuState.boxColor, vecToHex(opts.Visuals.ESP.Players.BoxColor).c_str());
            strcpy_s(g_MenuState.boxGlowColor, vecToHex(opts.Visuals.ESP.Players.BoxGlowColor).c_str());
            strcpy_s(g_MenuState.skeletonColor, vecToHex(opts.Visuals.ESP.Players.SkeletonColor).c_str());
            strcpy_s(g_MenuState.nameColor, vecToHex(opts.Visuals.ESP.Players.NameColor).c_str());
            strcpy_s(g_MenuState.weaponColor, vecToHex(opts.Visuals.ESP.Players.WeaponNameColor).c_str());
            strcpy_s(g_MenuState.distanceColor, vecToHex(opts.Visuals.ESP.Players.DistanceColor).c_str());
            strcpy_s(g_MenuState.snaplineColor, vecToHex(opts.Visuals.ESP.Players.SnaplinesColor).c_str());
            strcpy_s(g_MenuState.aimbotFovColor, vecToHex(opts.Misc.Screen.AimbotFovColor).c_str());
            strcpy_s(g_MenuState.silentFovColor, vecToHex(opts.Misc.Screen.SilentFovColor).c_str());
            strcpy_s(g_MenuState.healthBarColor, vecToHex(opts.Visuals.ESP.Players.HealthBarColor).c_str());
            strcpy_s(g_MenuState.healthBarLowColor, vecToHex(opts.Visuals.ESP.Players.HealthBarLowColor).c_str());
            strcpy_s(g_MenuState.armorBarColor, vecToHex(opts.Visuals.ESP.Players.ArmorBarColor).c_str());
            strcpy_s(g_MenuState.vehicleColor, vecToHex(opts.Visuals.ESP.Vehicles.Color).c_str());

            strcpy_s(g_MenuState.menuKey, sizeof(g_MenuState.menuKey), VKCodeToString(opts.General.MenuKey == 0 ? VK_INSERT : opts.General.MenuKey).c_str());
            strcpy_s(g_MenuState.panicKey, sizeof(g_MenuState.panicKey), VKCodeToString(opts.General.PanicKey).c_str());
            strcpy_s(g_MenuState.noclipKey, sizeof(g_MenuState.noclipKey), VKCodeToString(opts.Misc.Exploits.LocalPlayer.NoclipBind).c_str());
        }

        void SyncMenuToOptions()
        {
            auto& opts = g_Options;

            opts.LegitBot.SilentAim.Enabled = g_MenuState.silentEnable;
            opts.Misc.Screen.ShowSilentAimFov = g_MenuState.silentShowFov;
            opts.LegitBot.SilentAim.PredictionEnabled = g_MenuState.silentPrediction;
            opts.LegitBot.SilentAim.ForceDriver = g_MenuState.silentForceDriver;
            opts.LegitBot.SilentAim.AliveOnly = g_MenuState.silentAliveOnly;
            opts.LegitBot.SilentAim.VisibleCheck = g_MenuState.silentVisibleOnly;
            opts.LegitBot.SilentAim.ShotNPC = g_MenuState.silentTargetPed;
            opts.LegitBot.SilentAim.AlwaysOn = g_MenuState.silentAlwaysOn;
            opts.LegitBot.SilentAim.SilentBypass = g_MenuState.silentBypass;
            opts.LegitBot.SilentAim.SilentBypassV2 = g_MenuState.silentBypassV2;
            opts.LegitBot.SilentAim.MagicBullet = g_MenuState.silentMagicBullet;
            opts.LegitBot.SilentAim.Fov = static_cast<int>(std::lround(g_MenuState.silentFov));
            opts.LegitBot.SilentAim.MaxDistance = static_cast<int>(std::lround(g_MenuState.silentDistance));
            opts.LegitBot.SilentAim.PredictionTime = g_MenuState.silentPredictionTime;
            opts.LegitBot.SilentAim.MissChance = static_cast<int>(std::lround(g_MenuState.silentMisschance));
            opts.LegitBot.SilentAim.HitBox = g_MenuState.silentHitBoxIndex;

            opts.LegitBot.AimBot.Enabled = g_MenuState.aimbotEnable;
            opts.Misc.Screen.ShowAimbotFov = g_MenuState.aimbotShowFov;
            opts.LegitBot.AimBot.TargetNPC = g_MenuState.aimbotTargetPeds;
            opts.LegitBot.AimBot.VisibleCheck = g_MenuState.aimbotVisibleOnly;
            opts.LegitBot.AimBot.MaxDistance = static_cast<int>(std::lround(g_MenuState.aimbotDistance));
            opts.LegitBot.AimBot.FOV = static_cast<int>(std::lround(g_MenuState.aimbotFovMax));
            opts.LegitBot.AimBot.SmoothHorizontal = static_cast<int>(std::lround(g_MenuState.aimbotSmoothHorizontal));
            opts.LegitBot.AimBot.SmoothVertical = static_cast<int>(std::lround(g_MenuState.aimbotSmoothVertical));
            opts.LegitBot.AimBot.HitBox = g_MenuState.aimbotHitBoxIndex;

            opts.LegitBot.Trigger.Enabled = g_MenuState.triggerEnable;
            opts.LegitBot.Trigger.ShotNPC = g_MenuState.triggerTargetPed;
            opts.LegitBot.Trigger.VisibleCheck = g_MenuState.triggerVisibleCheck;
            opts.LegitBot.Trigger.MaxDistance = static_cast<int>(std::lround(g_MenuState.triggerMaxDistance));
            opts.LegitBot.Trigger.ReactionTime = static_cast<int>(std::lround(g_MenuState.triggerReactionTime));

            opts.Visuals.ESP.Players.Enabled = g_MenuState.visualsEnable;
            opts.Visuals.ESP.Players.Box = g_MenuState.visualsBoxes;
            opts.Visuals.ESP.Players.Skeleton = g_MenuState.visualsSkeleton;
            opts.Visuals.ESP.Players.Name = g_MenuState.visualsPlayerNames;
            opts.Visuals.ESP.Players.HealthBar = g_MenuState.visualsHealthbar;
            opts.Visuals.ESP.Players.WeaponName = g_MenuState.visualsWeapon;
            opts.Visuals.ESP.Players.Distance = g_MenuState.visualsDistance;
            opts.Visuals.ESP.Players.RenderDistance = static_cast<int>(std::lround(g_MenuState.visualsRenderDistance));
            opts.Visuals.ESP.Players.VisibleOnly = g_MenuState.visualsVisibleOnly;
            opts.Visuals.ESP.Players.ShowLocalPlayer = g_MenuState.visualsShowLocalPlayer;
            opts.Visuals.ESP.Players.ShowNPCs = g_MenuState.visualsShowNPCs;

            opts.Misc.Exploits.LocalPlayer.Noclip = g_MenuState.noclip;
            opts.Misc.Exploits.LocalPlayer.NoclipSpeed = static_cast<int>(std::lround(g_MenuState.noclipSpeed));
            opts.General.CaptureBypass = g_MenuState.streamMode; // Sync Stream Mode
            opts.Misc.Bypass.Destruct = g_MenuState.bypassDestructMode;

            opts.Misc.Exploits.LocalPlayer.norecoil = g_MenuState.miscNoRecoil;
            opts.Misc.Exploits.LocalPlayer.nospread = g_MenuState.miscNoSpread;
            opts.Misc.Exploits.LocalPlayer.noreload = g_MenuState.miscNoReload;
            opts.Misc.Exploits.LocalPlayer.Start_Health = g_MenuState.miscStartHealth;
            opts.Misc.Exploits.LocalPlayer.health_ammount = g_MenuState.miscHealthAmount;

            auto hexToColorArr = [](const char* hex, float* out) {
                ImVec4 col = hexToVec4(hex);
                out[0] = col.x; out[1] = col.y; out[2] = col.z; out[3] = 1.0f;
            };

            // Sync colors
            hexToColorArr(g_MenuState.boxColor, opts.Visuals.ESP.Players.BoxColor);
            ImVec4 glow = hexToVec4(g_MenuState.boxGlowColor);
            opts.Visuals.ESP.Players.BoxGlowColor[0] = glow.x;
            opts.Visuals.ESP.Players.BoxGlowColor[1] = glow.y;
            opts.Visuals.ESP.Players.BoxGlowColor[2] = glow.z;
            opts.Visuals.ESP.Players.BoxGlowColor[3] = 0.5f; // Keep alpha constant for glow

            hexToColorArr(g_MenuState.skeletonColor, opts.Visuals.ESP.Players.SkeletonColor);
            hexToColorArr(g_MenuState.nameColor, opts.Visuals.ESP.Players.NameColor);
            hexToColorArr(g_MenuState.weaponColor, opts.Visuals.ESP.Players.WeaponNameColor);
            hexToColorArr(g_MenuState.distanceColor, opts.Visuals.ESP.Players.DistanceColor);
            hexToColorArr(g_MenuState.snaplineColor, opts.Visuals.ESP.Players.SnaplinesColor);
            hexToColorArr(g_MenuState.aimbotFovColor, opts.Misc.Screen.AimbotFovColor);
            hexToColorArr(g_MenuState.silentFovColor, opts.Misc.Screen.SilentFovColor);
            hexToColorArr(g_MenuState.healthBarColor, opts.Visuals.ESP.Players.HealthBarColor);
            hexToColorArr(g_MenuState.healthBarLowColor, opts.Visuals.ESP.Players.HealthBarLowColor);
            hexToColorArr(g_MenuState.armorBarColor, opts.Visuals.ESP.Players.ArmorBarColor);
            hexToColorArr(g_MenuState.vehicleColor, opts.Visuals.ESP.Vehicles.Color);

            int menuKey = StringToVKCode(g_MenuState.menuKey);
            opts.General.MenuKey = (menuKey == 0) ? VK_INSERT : menuKey;
            opts.General.PanicKey = StringToVKCode(g_MenuState.panicKey);
            opts.Misc.Exploits.LocalPlayer.NoclipBind = StringToVKCode(g_MenuState.noclipKey);
            opts.General.CaptureBypass = g_MenuState.streamMode;
            opts.Misc.Bypass.Destruct = g_MenuState.bypassDestructMode;
}

        void UpdateOverlayInteraction(HWND window, bool menuOpen)
        {
            if (!window)
                return;

            LONG style = GetWindowLongW(window, GWL_EXSTYLE);
            if (menuOpen)
                style &= ~WS_EX_TRANSPARENT;
            else
                style |= WS_EX_TRANSPARENT;
            SetWindowLongW(window, GWL_EXSTYLE, style);
        }

        void ApplyMenuStyle()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowBorderSize = 0.0f;
            style.FrameBorderSize = 0.0f;
            style.PopupBorderSize = 0.0f;
            style.ChildBorderSize = 0.0f;
            style.TabBorderSize = 0.0f;
            style.WindowRounding = 18.0f;
            style.ChildRounding = 14.0f;
            style.FrameRounding = 12.0f;
            style.PopupRounding = 16.0f;
            style.ScrollbarRounding = 12.0f;
            style.GrabRounding = 12.0f;
            style.TabRounding = 10.0f;
            style.WindowPadding = ImVec2(0.0f, 0.0f);
            style.FramePadding = ImVec2(0.0f, 0.0f);
            style.ItemSpacing = ImVec2(0.0f, 0.0f);
            style.ItemInnerSpacing = ImVec2(0.0f, 0.0f);
            style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0, 0, 0, 0);
            style.WindowMenuButtonPosition = ImGuiDir_None;
            style.AntiAliasedLines = true;
            style.AntiAliasedFill = true;
        }

        void EnsureFonts()
        {
            if (g_FontsInitialized)
                return;

		ImGuiIO& io = ImGui::GetIO();
            ImFontConfig config;
            config.FontDataOwnedByAtlas = false;
            config.OversampleH = 3;
            config.OversampleV = 3;
            config.RasterizerMultiply = 1.05f;
            config.PixelSnapH = false;

            io.Fonts->TexDesiredWidth = 4096;
            io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
            io.FontGlobalScale = 1.0f;
            io.FontAllowUserScaling = false;

            g_ManropeFont = io.Fonts->AddFontFromMemoryTTF((void*)fonto_data, fonto_font_size, 20.0f, &config);
            if (!g_ManropeFont)
                g_ManropeFont = io.FontDefault;

            ImGui_ImplDX11_InvalidateDeviceObjects();
            ImGui_ImplDX11_CreateDeviceObjects();

            g_FontsInitialized = true;
        }

        bool CreateTextureFromPNGMemoryInternal(const unsigned char* data, size_t size, ID3D11ShaderResourceView** out_srv, UINT* out_w, UINT* out_h, bool convert_black_to_white = false)
        {
            if (!g_MenuDevice || !data || size == 0 || !out_srv)
                return false;

		IWICImagingFactory* factory = nullptr;
		IWICStream* stream = nullptr;
		IWICBitmapDecoder* decoder = nullptr;
		IWICBitmapFrameDecode* frame = nullptr;
		IWICFormatConverter* converter = nullptr;

            bool coInit = false;
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
            if (hr == S_OK)
                coInit = true;

            UINT w = 0;
            UINT h = 0;
            UINT stride = 0;
            UINT bufSize = 0;
            std::vector<BYTE> buffer;
            D3D11_TEXTURE2D_DESC desc{};
            D3D11_SUBRESOURCE_DATA sub{};
            ID3D11Texture2D* texture = nullptr;
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};

		hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
            if (FAILED(hr)) goto cleanup;

		hr = factory->CreateStream(&stream);
            if (FAILED(hr)) goto cleanup;

		hr = stream->InitializeFromMemory((BYTE*)data, (DWORD)size);
            if (FAILED(hr)) goto cleanup;

		hr = factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
            if (FAILED(hr)) goto cleanup;

		hr = decoder->GetFrame(0, &frame);
            if (FAILED(hr)) goto cleanup;

		hr = frame->GetSize(&w, &h);
            if (FAILED(hr)) goto cleanup;

		hr = factory->CreateFormatConverter(&converter);
            if (FAILED(hr)) goto cleanup;

		hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
            if (FAILED(hr)) goto cleanup;

            stride = w * 4;
            bufSize = stride * h;
		buffer.resize(bufSize);
		hr = converter->CopyPixels(nullptr, stride, bufSize, buffer.data());
            if (FAILED(hr)) goto cleanup;

            if (convert_black_to_white)
            {
                for (UINT y = 0; y < h; ++y)
                {
                    for (UINT x = 0; x < w; ++x)
                    {
                        BYTE* pixel = &buffer[(y * stride) + (x * 4)];
                        BYTE r = pixel[0];
                        BYTE g = pixel[1];
                        BYTE b = pixel[2];
                        BYTE a = pixel[3];

                        if (a > 0 && (r + g + b) < 50)
                        {
                            pixel[0] = 232;
                            pixel[1] = 233;
                            pixel[2] = 235;
                        }
                    }
                }
            }

		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		sub.pSysMem = buffer.data();
		sub.SysMemPitch = stride;
            sub.SysMemSlicePitch = 0;

            hr = g_MenuDevice->CreateTexture2D(&desc, &sub, &texture);
            if (FAILED(hr) || !texture) goto cleanup;

		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;

            hr = g_MenuDevice->CreateShaderResourceView(texture, &srvDesc, out_srv);
            if (FAILED(hr)) goto cleanup;

		if (out_w) *out_w = w;
		if (out_h) *out_h = h;

            if (converter) converter->Release();
            if (frame) frame->Release();
            if (decoder) decoder->Release();
            if (stream) stream->Release();
            if (factory) factory->Release();
            if (texture) texture->Release();
            if (coInit) CoUninitialize();
		return true;

        cleanup:
            if (converter) converter->Release();
            if (frame) frame->Release();
            if (decoder) decoder->Release();
            if (stream) stream->Release();
            if (factory) factory->Release();
            if (texture) texture->Release();
            if (coInit) CoUninitialize();
            if (out_srv) *out_srv = nullptr;
			return false;
        }

        void EnsureTextures()
        {
            if (g_TexturesInitialized || !g_MenuDevice)
                return;

            auto loadTexture = [](const unsigned char* data, size_t size, ImTextureID& target, float& w, float& h, bool convert_black_to_white = false)
            {
                if (target)
                    return;
                ID3D11ShaderResourceView* srv = nullptr;
                UINT width = 0, height = 0;
                if (CreateTextureFromPNGMemoryInternal(data, size, &srv, &width, &height, convert_black_to_white))
                {
                    target = (ImTextureID)srv;
                    w = static_cast<float>(width);
                    h = static_cast<float>(height);
                }
            };

            loadTexture(loogooo, sizeof(loogooo), g_logoTexture, g_logoWidth, g_logoHeight);
            loadTexture(croshair, sizeof(croshair), g_crosshairTexture, g_crosshairWidth, g_crosshairHeight, true);
            loadTexture(eye, sizeof(eye), g_eyeTexture, g_eyeWidth, g_eyeHeight, true);
            loadTexture(zap, sizeof(zap), g_zapTexture, g_zapWidth, g_zapHeight, true);
            loadTexture(palette, sizeof(palette), g_paletteTexture, g_paletteWidth, g_paletteHeight, true);
            loadTexture(userround, sizeof(userround), g_userroundTexture, g_userroundWidth, g_userroundHeight, true);
            loadTexture(settings, sizeof(settings), g_settingsTexture, g_settingsWidth, g_settingsHeight, true);
            loadTexture(keybinds, sizeof(keybinds), g_keybindsTexture, g_keybindsWidth, g_keybindsHeight, true);
            loadTexture(dominikakurwisko, sizeof(dominikakurwisko), g_dominikaTexture, g_dominikaWidth, g_dominikaHeight);

            g_TexturesInitialized = true;
        }

        void RenderBypassNotifications()
        {
            if (!Bypass::g_ShowUnloadingNotification && !Bypass::g_ShowSuccessNotification)
                return;

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw = ImGui::GetForegroundDrawList();
            if (!draw)
                return;

            const float notificationWidth = 260.0f;
            const float notificationHeight = 82.0f;
            ImVec2 basePos = ImVec2(io.DisplaySize.x - notificationWidth - 24.0f, io.DisplaySize.y - notificationHeight - 28.0f);
            ImVec2 size = ImVec2(notificationWidth, notificationHeight);

            auto toU32 = [](const ImVec4& col, float alpha = 1.0f)
            {
                ImVec4 temp = col;
                temp.w *= alpha;
                return ImGui::GetColorU32(temp);
            };

            Bypass::UpdateNotificationProgress();

            auto drawBackdrop = [&](ImVec2 min, ImVec2 max)
            {
                ImVec2 roundedMin = RoundToPixel(min);
                ImVec2 roundedMax = RoundToPixel(max);

                ImVec2 shadowOffset = ImVec2(0.0f, 6.0f);
                ImVec2 shadowMin = ImVec2(roundedMin.x + shadowOffset.x, roundedMin.y + shadowOffset.y);
                ImVec2 shadowMax = ImVec2(roundedMax.x + shadowOffset.x, roundedMax.y + shadowOffset.y);
                draw->AddRectFilled(shadowMin, shadowMax, IM_COL32(0, 0, 0, 60), 12.0f, ImDrawFlags_RoundCornersAll);

                ImU32 bgColor = IM_COL32(18, 20, 30, 240);
                draw->AddRectFilled(roundedMin, roundedMax, bgColor, 12.0f, ImDrawFlags_RoundCornersAll);

                ImU32 borderCol = IM_COL32(98, 108, 255, 110);
                draw->AddRect(roundedMin, roundedMax, borderCol, 12.0f, ImDrawFlags_RoundCornersAll, 1.0f);
            };

            auto drawProgressBar = [&](const ImVec2& min, const ImVec2& max, float progress, ImU32 color)
            {
                float clamped = std::clamp(progress, 0.0f, 1.0f);
                ImVec2 barStart = ImVec2(min.x + 20.0f, max.y - 18.0f);
                ImVec2 barEnd = ImVec2(max.x - 20.0f, max.y - 18.0f);

                draw->AddLine(barStart, barEnd, IM_COL32(48, 52, 70, 160), 2.0f);

                if (clamped <= 0.0f)
                    return;

                float width = (barEnd.x - barStart.x) * clamped;
                ImVec2 activeEnd = ImVec2(barStart.x + width, barStart.y);
                float pulse = 0.4f + 0.6f * std::sinf(ImGui::GetTime() * 2.6f);
                ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color);
                colorVec.w *= 0.85f + 0.15f * pulse;
                draw->AddLine(barStart, activeEnd, ImGui::GetColorU32(colorVec), 2.8f);
            };

            auto drawIcon = [&](const ImVec2& center, bool success, ImU32 accent)
            {
                if (success)
                {
                    draw->AddCircleFilled(center, 16.0f, ImGui::GetColorU32(ImVec4(0.16f, 0.56f, 0.36f, 0.92f)), 48);
                    draw->AddCircle(center, 16.0f, ImGui::GetColorU32(ImVec4(0.27f, 0.76f, 0.51f, 0.7f)), 48, 1.8f);
                    ImVec2 p1 = ImVec2(center.x - 7.0f, center.y + 2.0f);
                    ImVec2 p2 = ImVec2(center.x - 1.0f, center.y + 8.0f);
                    ImVec2 p3 = ImVec2(center.x + 8.0f, center.y - 6.0f);
                    draw->AddLine(p1, p2, IM_COL32(232, 240, 250, 255), 2.5f);
                    draw->AddLine(p2, p3, IM_COL32(232, 240, 250, 255), 2.5f);
                }
                else
                {
                    float time = ImGui::GetTime();
                    float rotation = time * 2.8f;
                    float radius = 14.0f;
                    int segments = 48;
                    float thickness = 2.8f;

                    draw->AddCircleFilled(center, radius - 6.0f, IM_COL32(22, 24, 36, 220), 48);

                    float startAngle = rotation;
                    float sweep = IM_PI * 1.2f;

                    draw->PathClear();
                    for (int i = 0; i <= segments; ++i)
                    {
                        float t = startAngle + sweep * (static_cast<float>(i) / segments);
                        draw->PathLineTo(ImVec2(center.x + std::cosf(t) * radius, center.y + std::sinf(t) * radius));
                    }
                    draw->PathStroke(accent, 0, thickness);

                    float dotRadius = 3.4f + 1.0f * std::sinf(time * 3.8f);
                    ImVec2 dotPos = ImVec2(center.x + std::cosf(startAngle + sweep) * (radius - 2.0f),
                                           center.y + std::sinf(startAngle + sweep) * (radius - 2.0f));
                    draw->AddCircleFilled(dotPos, dotRadius, accent, 16);
                }
            };

            auto drawNotification = [&](const char* title, const char* description, float progress, bool success, ImU32 accent)
            {
                ImVec2 min = basePos;
                ImVec2 max = ImVec2(basePos.x + size.x, basePos.y + size.y);
                drawBackdrop(min, max);

                ImVec2 iconCenter = ImVec2(min.x + 34.0f, min.y + 36.0f);
                drawIcon(iconCenter, success, accent);

                if (g_ManropeFont)
                {
                    ImGui::PushFont(g_ManropeFont);
                    ImVec4 titleColor = success ? ImVec4(0.62f, 0.86f, 0.70f, 1.0f) : ImVec4(0.76f, 0.78f, 0.96f, 1.0f);
                    ImVec4 descColor = ImVec4(0.76f, 0.78f, 0.90f, 0.86f);
                    draw->AddText(g_ManropeFont, 15.0f, RoundToPixel(ImVec2(min.x + 68.0f, min.y + 20.0f)), ImGui::GetColorU32(titleColor), title);
                    draw->AddText(g_ManropeFont, 13.0f, RoundToPixel(ImVec2(min.x + 68.0f, min.y + 40.0f)), ImGui::GetColorU32(descColor), description);
                    ImGui::PopFont();
                }

                drawProgressBar(min, max, progress, accent);
            };

            if (Bypass::g_ShowUnloadingNotification)
            {
                drawNotification("Detaching & cleaning", "Please wait...", Bypass::g_NotificationProgress, false, IM_COL32(142, 151, 255, 255));
            }
            else if (Bypass::g_ShowSuccessNotification)
            {
                drawNotification("Bypass complete", "Done :)", 1.0f, true, IM_COL32(120, 200, 140, 255));
            }
        }

        void HandleSmoothWindowDragging()
        {
            if (!g_MenuWindow)
                return;

            ImGuiIO& io = ImGui::GetIO();
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImVec2 viewportPos = viewport->Pos;
            ImVec2 viewportSize = viewport->Size;

            float base_width = 950.0f; // Matched with RenderMenu
            float base_height = 600.0f;
            float scaled_width = base_width * g_MenuScale;
            float scaled_height = base_height * g_MenuScale;
            float header_height = 30.0f * g_MenuScale;

            ImVec2 currentOffset = ImVec2(g_MenuOffsetX, g_MenuOffsetY);
            ImVec2 menu_pos = ImVec2((viewportSize.x - scaled_width) * 0.5f + currentOffset.x,
                                     (viewportSize.y - scaled_height) * 0.5f + currentOffset.y);
            ImVec2 mousePos = io.MousePos - viewportPos;
            ImVec2 localPos = ImVec2(mousePos.x - menu_pos.x, mousePos.y - menu_pos.y);

            bool isMouseOverGUI = (localPos.x >= 0.0f && localPos.y >= 0.0f && localPos.x <= scaled_width && localPos.y <= scaled_height);
            bool isMouseOverHeader = (localPos.x >= 0.0f && localPos.x <= scaled_width && localPos.y >= 0.0f && localPos.y <= header_height);

            HWND hwnd = g_MenuWindow;

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isMouseOverGUI)
            {
                bool canDrag = isMouseOverHeader;
                if (canDrag)
                {
                    g_IsDragging = true;
                    SetCapture(hwnd);
                    POINT globalMouse;
                    GetCursorPos(&globalMouse);
                    g_DragStartMouse = globalMouse;
                    g_DragStartOffsetX = g_MenuOffsetX;
                    g_DragStartOffsetY = g_MenuOffsetY;
                    g_MenuOffsetTargetX = g_MenuOffsetX;
                    g_MenuOffsetTargetY = g_MenuOffsetY;
                }
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                if (g_IsDragging)
                    ReleaseCapture();
                g_IsDragging = false;
            }

            if (g_IsDragging)
            {
                POINT currentMouse;
                GetCursorPos(&currentMouse);
                float deltaX = static_cast<float>(currentMouse.x - g_DragStartMouse.x);
                float deltaY = static_cast<float>(currentMouse.y - g_DragStartMouse.y);
                g_MenuOffsetTargetX = g_DragStartOffsetX + deltaX;
                g_MenuOffsetTargetY = g_DragStartOffsetY + deltaY;
                float deltaTime = io.DeltaTime;
                float dragSmoothFactor = 1.0f - std::pow(1.0f - (g_DragSmoothness * 3.0f), deltaTime * 60.0f);
                g_MenuOffsetX += (g_MenuOffsetTargetX - g_MenuOffsetX) * dragSmoothFactor;
                g_MenuOffsetY += (g_MenuOffsetTargetY - g_MenuOffsetY) * dragSmoothFactor;
            }
            else
            {
                float deltaTime = io.DeltaTime;
                float smoothFactor = 1.0f - std::pow(1.0f - g_DragSmoothness, deltaTime * 60.0f);
                g_MenuOffsetX += (g_MenuOffsetTargetX - g_MenuOffsetX) * smoothFactor;
                g_MenuOffsetY += (g_MenuOffsetTargetY - g_MenuOffsetY) * smoothFactor;
            }
        }
} // anonymous namespace


namespace FrameWork
{

bool CreateTextureFromPNGMemory(const unsigned char* data, size_t size, ID3D11ShaderResourceView** out_srv, UINT* out_w, UINT* out_h)
{
    return CreateTextureFromPNGMemoryInternal(data, size, out_srv, out_w, out_h);
}

void HandleOverlayDragging(HWND overlayWindow)
{
    (void)overlayWindow;
    HandleSmoothWindowDragging();
}   

Interface::Interface() = default;

Interface::Interface(HWND window, HWND targetWindow, ID3D11Device* device, ID3D11DeviceContext* context)
{
    Initialize(window, targetWindow, device, context);
}

Interface::~Interface()
{
    ShutDown();
}

void Interface::Initialize(HWND window, HWND targetWindow, ID3D11Device* device, ID3D11DeviceContext* context)
{
    this->hWindow = window;
    this->hTargetWindow = targetWindow;
    this->IDevice = device;
    this->IDeviceContext = context;

    g_MenuWindow = window;
    g_MenuDevice = device;
    g_MenuDeviceContext = context;

    if (!ImGui::GetCurrentContext())
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, context);

    ApplyMenuStyle();
    EnsureFonts();
    EnsureTextures();
    SyncOptionsToMenu();

    UpdateOverlayInteraction(window, this->bIsMenuOpen);
}

void Interface::UpdateStyle()
{
    ApplyMenuStyle();
}

void Interface::RenderGui()
{
    EnsureFonts();
    EnsureTextures();
    SyncOptionsToMenu();

    if (Bypass::g_BypassInProgress || Bypass::g_ForceHideGUI || Bypass::g_CheatDisabled)
    {
        if (this->bIsMenuOpen)
        {
            this->bIsMenuOpen = false;
            UpdateOverlayInteraction(this->hWindow, false);
        }

        RenderBypassNotifications();
        SyncMenuToOptions();
        return;
    }

    bool modalActive = g_ShowSilentBypassModal || g_ShowLoadConfigModal || g_ShowSaveConfigModal;
    bool shouldRenderMenu = this->bIsMenuOpen || g_IsLoading || modalActive;

    if (!shouldRenderMenu)
    {
        g_IsLoading = false;
        g_MenuFadeInAlpha = 0.0f;
        g_MenuScale = 0.95f;
        RenderBypassNotifications();
        SyncMenuToOptions();
        return;
    }

    float current_time = ImGui::GetTime();
    if (g_IsLoading && g_LoadingStartTime == 0.0f)
        g_LoadingStartTime = current_time;
    if (g_IsLoading && (current_time - g_LoadingStartTime) >= g_LoadingDuration)
    {
        g_IsLoading = false;
        g_LoadingStartTime = current_time;
    }
    if (!g_IsLoading)
    {
        float fade_elapsed = current_time - g_LoadingStartTime;
        if (fade_elapsed < g_FadeInDuration)
        {
            float t = fade_elapsed / g_FadeInDuration;
            float eased = 1.0f - std::pow(1.0f - t, 3.0f);
            g_MenuFadeInAlpha = eased;
            g_MenuScale = 0.95f + (eased * 0.05f);
        }
        else
        {
            g_MenuFadeInAlpha = 1.0f;
            g_MenuScale = 1.0f;
        }
    }

    RenderBackground();
    if (g_IsLoading)
    {
        RenderLoadingScreen();
    }
    else
    {
        float alpha = std::clamp(g_MenuFadeInAlpha, 0.0f, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        RenderMenu();
        ImGui::PopStyleVar();

        if (alpha < 1.0f)
        {
            ImVec2 vp = ImGui::GetMainViewport()->Pos;
            ImVec2 vs = ImGui::GetMainViewport()->Size;
            ImGui::SetNextWindowPos(vp);
            ImGui::SetNextWindowSize(vs);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGuiWindowFlags overlay_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                                             ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;
            if (ImGui::Begin("##FadeInOverlay", nullptr, overlay_flags))
            {
                ImDrawList* overlay_draw_list = ImGui::GetWindowDrawList();
                float overlay_alpha = 1.0f - alpha;
                ImU32 overlay_color = IM_COL32(0, 0, 0, static_cast<int>(overlay_alpha * 255.0f));
                overlay_draw_list->AddRectFilled(vp, ImVec2(vp.x + vs.x, vp.y + vs.y), overlay_color);
                ImGui::End();
            }
        }

        g_MenuHasRenderedOnce = true;
    }

    RenderBypassNotifications();
    if (this->bIsMenuOpen)
    {
        HandleSmoothWindowDragging();
    }

    SyncMenuToOptions();
}

void Interface::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ::ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

void Interface::HandleMenuKey()
{
    static bool menuDebounce = false;
    if (Bypass::g_BypassInProgress || Bypass::g_ForceHideGUI || Bypass::g_CheatDisabled)
    {
        if (this->bIsMenuOpen)
            SetMenuOpen(false);
        menuDebounce = false;
        return;
    }

    int menuKey = g_Options.General.MenuKey == 0 ? VK_INSERT : g_Options.General.MenuKey;

    if (GetAsyncKeyState(menuKey) & 0x8000)
    {
        if (!menuDebounce)
        {
            SetMenuOpen(!this->bIsMenuOpen);
            menuDebounce = true;
        }
    }
    else
    {
        menuDebounce = false;
    }

    int panicKey = g_Options.General.PanicKey;
    if (panicKey != 0 && (GetAsyncKeyState(panicKey) & 0x8000))
        g_Options.General.ShutDown = true;
}

void Interface::ShutDown()
{
    this->hWindow = nullptr;
    this->hTargetWindow = nullptr;
    this->IDevice = nullptr;
    this->IDeviceContext = nullptr;
}

bool Interface::GetMenuOpen() const
{
    return this->bIsMenuOpen;
}

void Interface::SetMenuOpen(bool open)
{
    if (this->bIsMenuOpen == open)
        return;
    this->bIsMenuOpen = open;
    UpdateOverlayInteraction(this->hWindow, this->bIsMenuOpen);

    if (this->bIsMenuOpen)
    {
        g_LoadingStartTime = 0.0f;
        g_MenuFadeInAlpha = 0.0f;
        g_MenuScale = 0.95f;
        g_IsLoading = !g_MenuHasRenderedOnce;
    }
    else
    {
        g_IsLoading = false;
        g_MenuFadeInAlpha = 0.0f;
        CancelKeybindCapture();
    }
}

void RenderESPAndFOV()
{
    if (Bypass::g_BypassInProgress || Bypass::g_CheatDisabled)
        return;

    if (ImGui::GetCurrentContext() == nullptr)
        return;

    auto& opts = g_Options;

    const bool renderPlayers = opts.Visuals.ESP.Players.Enabled;
    const bool renderSilentFov = opts.Misc.Screen.ShowSilentAimFov && opts.LegitBot.SilentAim.Fov > 0;
    const bool renderAimbotFov = opts.Misc.Screen.ShowAimbotFov && opts.LegitBot.AimBot.FOV > 0;

    if (!renderPlayers && !renderSilentFov && !renderAimbotFov)
        return;

    if (renderSilentFov || renderAimbotFov)
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();
        if (drawList)
        {
            const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
            if (displaySize.x > 0.0f && displaySize.y > 0.0f)
            {
                const ImVec2 center(displaySize.x * 0.5f, displaySize.y * 0.5f);

                if (renderSilentFov)
                {
                    ImColor silentColor(
                        opts.Misc.Screen.SilentFovColor[0],
                        opts.Misc.Screen.SilentFovColor[1],
                        opts.Misc.Screen.SilentFovColor[2],
                        opts.Misc.Screen.SilentFovColor[3]);
                    if (opts.LegitBot.SilentAim.Fov > 0)
                    {
                        drawList->AddCircle(center, static_cast<float>(opts.LegitBot.SilentAim.Fov), silentColor, 128, 2.0f);
                    }
                }

                if (renderAimbotFov)
                {
                    ImColor aimbotColor(
                        opts.Misc.Screen.AimbotFovColor[0],
                        opts.Misc.Screen.AimbotFovColor[1],
                        opts.Misc.Screen.AimbotFovColor[2],
                        opts.Misc.Screen.AimbotFovColor[3]);
                    if (opts.LegitBot.AimBot.FOV > 0)
                    {
                        drawList->AddCircle(center, static_cast<float>(opts.LegitBot.AimBot.FOV), aimbotColor, 128, 2.0f);
                    }
                }
            }
        }
    }

    if (!g_Fivem.IsInitialized())
        return;

    if (renderPlayers)
        ESP::Players();
}

} // namespace FrameWork


