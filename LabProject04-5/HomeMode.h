#pragma once
#include "Framework.h"
#include "Object.h"
#include "Mode1.h"

// �ϴ��� �� ȭ������ �����Ѵ�
inline std::string HomeMode() {

	return __func__;
}

namespace Home_Mode {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			switch (wParam) {

			// �����̽� ������ ���� ����
			case VK_SPACE:
				fw.SetMode(Mode1);
				break;
			}
		}
	}

	inline void MouseMotionController(HWND hwnd) {}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {}
}