#pragma once
#include "includes.h"

struct Settings
{
	bool Esp = true;
	bool DrawMenu = true;
	bool aimbot = true;
	bool Trademark = true;
	int smooth = 4;
	bool FOVCircle = true;
	bool Prediction = false;
	bool Guntracer = false;
	bool skeleton = false;
	bool D3D = false;
	bool corner = false;
	bool box = false;
	bool filled = false;
	int hitbone;
	bool snaplines = false;
	bool SkippedDowned = false;
	bool distance = false;
	bool MuzzleLines = false;
	int Boneoffset = 0;
	bool VisableCheck = false;
	int esp_distance = 500;
	int AimFOV = 200;
	bool hvh = false;
	int AimDistance = 500;
	bool IsVis;
	bool Focused;
} Settings_options;

static const char* boxes[]
{
	"Corner Box",
	"Outlined Box"
};