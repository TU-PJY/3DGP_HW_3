#pragma once
#include "Framework.h"
#include "Object.h"
#include <string>

// 1��Ī �����̹Ƿ� �Ž��� ������ ���� �ʴ´�
class Player : public OBJ {
private:
	// ���콺 ����
	float MouseSensitivity = 0.3;
	bool MoveFront{}, MoveBack{}, MoveRight{}, MoveLeft{};
	float ForwardSpeed{}, StrafeSpeed{};

	float FallingAcc;

	bool JumpState{};

public:
	Player(std::string tag, Layer layer) {
		ObjectLayer = layer;
		Tag = tag;

		Position = XMFLOAT3(-30.0, 80.0, 0.0);
	}

	// �÷��̾� �̵�
	void MovePlayer(float FT) {
		if (MoveFront)
			LerpAcc(ForwardSpeed, 0.2, 5, FT);
		if (MoveBack)
			LerpAcc(ForwardSpeed, -0.2, 5, FT);
		if (MoveRight)
			LerpAcc(StrafeSpeed, 0.2, 5, FT);
		if (MoveLeft)
			LerpAcc(StrafeSpeed, -0.2, 5, FT);

		if(!MoveFront && !MoveBack)
			LerpDcc(ForwardSpeed, 5, FT);

		if(!MoveRight && !MoveLeft)
			LerpDcc(StrafeSpeed, 5, FT);

		MoveForwardNoY(ForwardSpeed);
		MoveStrafe(StrafeSpeed);
	}

	void Update(float FT) {
		InitTransform();

		MovePlayer(FT);

		SetPosition(Position);
		Rotate(Rotation.x, Rotation.y, Rotation.z);

		auto map = fw.FindObject("terrain", LayerRange::Single, Layer::Terrain);
		if (map) {
			if (fw.CheckTerrainFloor(this, map, 25.0f) && FallingAcc <= 0) {
				Position.y = std::lerp(Position.y, map->TerrainMesh->GetHeightAtPosition(map->TerrainMesh, Position.x, Position.z, map->Matrix) + 25.0, FT * 5);
				//fw.MoveToTerrainFloor(this, map, 25.0f);
				FallingAcc = 0.0f;
				JumpState = false;
			}

			else {
				FallingAcc -= FT * 150;
				Position.y += FallingAcc * FT * 2;
			}
		}
	}

	void ObjectKeyboardController(UINT nMessageID, WPARAM wParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			switch (wParam) {
			// ���콺 ���� ����
			case VK_PRIOR:
				MouseSensitivity += 0.1;
				break;

			// ���콺 ���� ����
			case VK_NEXT:
				if(MouseSensitivity > 0.2)
					MouseSensitivity -= 0.1;
				break;

			case 'W':
				MoveFront = true;
				break;

			case 'S':
				MoveBack = true;
				break;

			case 'D':
				MoveRight = true;
				break;

			case 'A':
				MoveLeft = true;
				break;

			case VK_SPACE:
				if (!JumpState) {
					FallingAcc = 60.0f;
					JumpState = true;
				}
				break;
			}
			break;

		case WM_KEYUP:
			switch (wParam) {
			case 'W':
				MoveFront = false;
				break;
			case 'S':
				MoveBack = false;
				break;
			case 'D':
				MoveRight = false;
				break;
			case 'A':
				MoveLeft = false;
				break;
			}
			break;
		}
	}

	void ObjectMouseMotionController(POINT PrevCursorPos, bool LButtonDownState, bool RButtonDownState) {
		::SetCursor(NULL);
		POINT CursorPos;
		::GetCursorPos(&CursorPos);

		float cxDelta = 0.0;
		float cyDelta = 0.0;

		cxDelta = (float)(CursorPos.x - PrevCursorPos.x) / 5.0f;
		cyDelta = (float)(CursorPos.y - PrevCursorPos.y) / 5.0f;
		::SetCursorPos(PrevCursorPos.x, PrevCursorPos.y);

		UpdateRotation(cyDelta * MouseSensitivity, cxDelta * MouseSensitivity, 0.0);
	}

	void ObjectMouseController(POINT CursorPos, bool LButtonDownState, bool RButtonDownState) {}
};