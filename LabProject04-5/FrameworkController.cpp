#include "Framework.h"
#include "ModeHeader.h"
#include "Controller.h"

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
	if (RunningMode == "Mode1")
		Mode_1::KeyboardController(hWnd, nMessageID, wParam, lParam);
}

void Framework::MouseMotionController(HWND hwnd) {
	if (RunningMode == "Mode1")
		Mode_1::MouseMotionController(hwnd);
}

void Framework::MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	switch (nMessageID) {
	case WM_LBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&fw.PrevCursorPosition);
		fw.LButtonDownState = true;
		break;

	case WM_RBUTTONDOWN:
		::SetCapture(hWnd);
		::GetCursorPos(&fw.PrevCursorPosition);
		fw.RButtonDownState = true;
		break;

	case WM_LBUTTONUP:
		::ReleaseCapture();
		fw.LButtonDownState = false;
		break;

	case WM_RBUTTONUP:
		::ReleaseCapture();
		fw.RButtonDownState = false;
		break;
	}

	if (RunningMode == "Mode1")
		Mode_1::MouseController(hWnd, nMessageID, wParam, lParam);
}