#include "Camera.h"
#include "Framework.h"


void Camera::Update(float FT) {
	if (RunningMode == CamMode::MODE1) {
		auto obj = fw.FindObject("obj1", LayerRange::Single, Layer::L1);
		if (obj) Track(obj->Position, obj, FT);
	}
}


void Camera::CameraKeyboardController(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
}