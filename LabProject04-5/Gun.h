#pragma once
#include "Framework.h"
#include "Object.h"
#include <cmath>

class Gun : public OBJ {
private:
	XMFLOAT3 TargetRotation{};
	XMFLOAT3 RealRotation{};
public:
	Gun(std::string tag, Layer layer) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pM60Mesh"));
		SetColor(XMFLOAT3(0.3, 0.3, 0.3));

		ObjectLayer = layer;
		Tag = tag;
	}

	void Update(float FT) {
		InitTransform();

		auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
		if (player) {
			Position.x = player->Position.x;
			Position.y = player->Position.y;
			Position.z = player->Position.z;

			TargetRotation = player->Rotation;
		}

		RealRotation.x = std::lerp(RealRotation.x, TargetRotation.x, 10 * FT);
		RealRotation.y = std::lerp(RealRotation.y, TargetRotation.y, 10 * FT);
		RealRotation.z = std::lerp(RealRotation.z, TargetRotation.z, 10 * FT);

		SetPosition(Position);
		Rotate(RealRotation.x, RealRotation.y, RealRotation.z);
	}

	void Render(ID3D12GraphicsCommandList* CmdList) {
		if (ObjectShader)
			ObjectShader->Render(CmdList);

		UpdateShaderVariables(CmdList);

		if (ObjectMesh)
			ObjectMesh->Render(CmdList);
	}

	void ObjectKeyboardController(UINT nMessageID, WPARAM wParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			break;

		case WM_KEYUP:
			break;
		}
	}

	void ObjectMouseMotionController(POINT PrevCursorPos, bool LButtonDownState, bool RButtonDownState) {}

	void ObjectMouseController(POINT CursorPos, bool LButtonDownState, bool RButtonDownState) {}
};