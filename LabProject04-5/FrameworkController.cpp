#include "Framework.h"
#include "HomeMode.h"
#include "Mode1.h"

// global scope shader
PseudoLightingShader* pShader;
// global scope framework
Framework fw;
// global scope camera
Camera cam;


// mesh list to load
std::unordered_map<std::string, char*> MeshList {
	{ "pUfoMesh", "Models//UFO.txt" },
	{ "pFlyerMesh", "Models//FlyerPlayerShip.txt" },
	{ "pM60Mesh", "Models//m60.txt" },
	{ "pFireMesh", "Models//fire.txt" },
	{ "pAmmoMesh", "Models//ammo.txt"}
};

// teerain list to load
std::unordered_map<std::string, char*> TerrainList {
	{ "pTerrain", "Models//map.txt"}
};


void Framework::Init(ID3D12Device* Device, ID3D12GraphicsCommandList* CmdList) {
	RootSignature = CreateGraphicsRootSignature(Device);
	pShader = ShaderLoader(RootSignature, Device, CmdList);
	LoadMeshFromList(Device, CmdList);
	LoadTerrainFromList(Device, CmdList);

	SetMode(Mode1);
}

void Framework::KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	if(RunningMode == "HomeMode")
		Home_Mode::KeyboardController(hWnd, nMessageID, wParam, lParam);

	else if (RunningMode == "Mode1")
		Mode_1::KeyboardController(hWnd, nMessageID, wParam, lParam);
}

void Framework::MouseMotionController(HWND hWnd) {
	if (RunningMode == "Mode1") {
		if (!SetCaptureLockState) {
			::SetCapture(hWnd);
			::GetCursorPos(&fw.PrevCursorPosition);
			SetCaptureLockState = true;
		}

		Mode_1::MouseMotionController(hWnd);
	}
}

void Framework::MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		fw.LButtonDownState = true;
		break;

	case WM_RBUTTONDOWN:
		fw.RButtonDownState = true;
		break;

	case WM_LBUTTONUP:
		fw.LButtonDownState = false;
		break;

	case WM_RBUTTONUP:
		fw.RButtonDownState = false;
		break;
	}

	if (RunningMode == "Mode1") {
		Mode_1::MouseController(hWnd, nMessageID, wParam, lParam);
	}
}