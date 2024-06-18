#pragma once
#include "Framework.h"

// 모드마다 별도의 네임스페이스
namespace Mode_1 {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN: case WM_KEYUP:
		{
			auto obj = fw.FindObject("obj1", LayerRange::Single, Layer::L1);
			if (obj) obj->ObjectKeyboardController(nMessageID, wParam);
		}

		switch (wParam) {
		case VK_ESCAPE:
			PostQuitMessage(1);
			break;
		}

		break;
		}
	}

	inline void MouseMotionController(HWND hwnd) {
		if (GetCapture() == hwnd) {
			auto obj = fw.FindObject("obj1", LayerRange::Single, Layer::L1);
			if (obj) obj->ObjectMouseMotionController(fw.PrevCursorPosition, fw.LButtonDownState, fw.RButtonDownState);
		}
	}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	}
}