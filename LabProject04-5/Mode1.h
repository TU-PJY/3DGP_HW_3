#pragma once
#include "Framework.h"
#include "Terrain.h"
#include "Player.h"
#include "Gun.h"
#include <string>

// �Լ� �̸��� �� ���� ���� ��� �̸��� �ȴ�
inline std::string Mode1() {
	fw.AddObject(new Terrain("terrain", Layer::Terrain), Layer::Terrain);
	fw.AddObject(new Player("player", Layer::L1), Layer::L1);
	fw.AddObject(new Gun("gun", Layer::L1), Layer::L1);
	fw.AddObject(new Fire("fire", Layer::L1), Layer::L1);

	return __func__;
}

namespace Mode_1 {
	inline void KeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
		switch (nMessageID) {
		case WM_KEYDOWN: case WM_KEYUP:
			switch (wParam) {
			case VK_ESCAPE:
				PostQuitMessage(1);
				break;
			}

			// �÷��̾� Ű���� ����
			{
				auto obj = fw.FindObject("player", LayerRange::Single, Layer::L1);
				if (obj) obj->ObjectKeyboardController(nMessageID, wParam);
			}

		break;
		}
	}

	inline void MouseMotionController(HWND hwnd) {
		if (GetCapture() == hwnd) {
			// �÷��̾� ���콺 ��� ����
			auto obj = fw.FindObject("player", LayerRange::Single, Layer::L1);
			if (obj) obj->ObjectMouseMotionController(fw.PrevCursorPosition, fw.LButtonDownState, fw.RButtonDownState);
		}
	}

	inline void MouseController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	}
}