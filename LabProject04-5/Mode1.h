#pragma once
#include "Framework.h"
#include "terrain.h"
#include <string>

// �Լ� �̸��� �� ���� ���� ��� �̸��� �ȴ�
inline std::string Mode1() {
	fw.AddObject(new Terrain("terrain", Layer::Terrain), Layer::Terrain);

	return __func__;
}

namespace Mode_1 {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
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

		}
	}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	}
}