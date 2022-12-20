#include "FN Utils\includes.h"

DWORD ScreenCenterX;
DWORD ScreenCenterY;
DWORD ScreenCenterZ;
DWORD ScreenCenter = ScreenCenterX / 2 + ScreenCenterY / 2 + ScreenCenterZ / 2;

ImFont* SkeetFont;
DWORD process_id;
DWORD64 base_address;
HWND hwnd = NULL;

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };
HWND MyWnd = NULL;
HWND GameWnd = NULL;
RECT GameRect = { NULL };
MSG Message = { NULL };

namespace Overlay
{
	namespace cheat
	{

		void DrawCircleFilled(int x, int y, int radius, RGBA* color)
		{
			ImGui::GetOverlayDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
		}

		void DrawCircle(int x, int y, int size, RGBA* color, int Segaments, int Thickness)
		{
			ImGui::GetOverlayDrawList()->AddCircle(ImVec2(x, y), size, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), Segaments, Thickness);
		}

		void DrawSnapline(int x, int y, int w, int h, int Thickness, RGBA* color)
		{
			ImGui::GetOverlayDrawList()->AddLine(ImVec2(x, y), ImVec2(w, h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), Thickness);
		}

		void DrawFilledPlayerBox(int x, int y, int w, int h, ImU32 color)
		{
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
		}

		void DrawRect(int x, int y, int w, int h, int thickness, RGBA* color)
		{
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0, thickness);
		}

		void DrawRoundedRect(int x, int y, int w, int h, ImU32& color, int thickness)
		{
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), ImGui::ColorConvertFloat4ToU32(ImVec4(1 / 255.0, 1 / 255.0, 1 / 255.0, 255 / 255.0)), 0, 0, 3);
			ImGui::GetOverlayDrawList()->AddRect(ImVec2(x, y), ImVec2(w, h), ImGui::GetColorU32(color), 0, 0, thickness);
		}

		void FilledRect(int x, int y, int w, int h, ImColor color)
		{
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color, 0, 0);
			ImGui::GetOverlayDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 150)), 0, 0);
		}

		void DrawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color) {
			DrawFilledPlayerBox(x + borderPx, y, w / 3, borderPx, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x + w - w / 3 + borderPx, y, w / 3, borderPx, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x, y, borderPx, h / 3, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x, y + h - h / 3 + borderPx * 2, borderPx, h / 3, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x + borderPx, y + h + borderPx, w / 3, borderPx, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x + w - w / 3 + borderPx, y + h + borderPx, w / 3, borderPx, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x + w + borderPx, y, borderPx, h / 3, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
			DrawFilledPlayerBox(x + w + borderPx, y + h - h / 3 + borderPx * 2, borderPx, h / 3, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)));
		}
	}
}
