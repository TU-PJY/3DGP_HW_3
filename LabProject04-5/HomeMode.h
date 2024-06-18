#pragma once
#include "Framework.h"
#include "Object.h"
#include "Mode1.h"

// 일단은 빈 화면으로 시작한다
inline std::string HomeMode() {

	return __func__;
}

namespace Home_Mode {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			switch (wParam) {

			// 스페이스 누르면 게임 시작
			case VK_SPACE:
				fw.SetMode(Mode1);
				break;
			}
		}
	}

	inline void MouseMotionController(HWND hwnd) {}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
}