#pragma once
#include "Framework.h"
#include "Object.h"
#include "StartMenu.h"


inline std::string HomeMode() {
	::ReleaseCapture();
	cam.SetPosition(XMFLOAT3(0.0, 0.0, 0.0));
	cam.SetLookAt(XMFLOAT3(0.0, 0.0, 1.0));
	fw.AddObject(new StartMenu("start_menu", Layer::L1), Layer::L1);

	return __func__;
}


namespace Home_Mode {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			switch (wParam) {
			case VK_ESCAPE:
				PostQuitMessage(1);
				break;
				
			// 스페이스 누르면 게임 시작
			case VK_SPACE:
				fw.ReserveMode("MainMode");
				break;
			}
		}
	}

	inline void MouseMotionController(HWND hwnd) {}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
}