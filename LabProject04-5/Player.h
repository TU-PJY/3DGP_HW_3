#pragma once
#include "Framework.h"
#include "Object.h"
#include <string>

// 1인칭 게임이므로 매쉬를 가지고 있지 않는다
class Player : public OBJ {
private:
	// 마우스 감도
	float MouseSensitivity = 0.3;
	bool MoveFront{}, MoveBack{}, MoveRight{}, MoveLeft{};
	float ForwardSpeed{}, StrafeSpeed{};
	float FallingAcc;

	bool JumpState{};

	float WalkingShake{};
	float ShakeValue{};
	float LandShake{};
	float LandShakeValue{};
	float LandShakeNum{};

	bool SpaceDown{};

public:
	Player(std::string tag, Layer layer) {
		ObjectLayer = layer;
		Tag = tag;

		Position = XMFLOAT3(0.0, 0.0, 0.0);
	}

	// 플레이어 이동
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

	// 걷기 모션 업데이트
	void UpdateWalkingShake(float FT) {
		if ((MoveFront || MoveBack || MoveRight || MoveLeft) && !JumpState)
			ShakeValue += FT * 10;

		else {
			if (ShakeValue > 1.0)
				ShakeValue = sin(ShakeValue);
			LerpDcc(ShakeValue, 5, FT);
		}

		WalkingShake = sin(ShakeValue) * 2.0;
	}

	// 착지 모션 업데이트
	void UpdateLandShake(float FT) {
		if (LandShakeValue > 0.0) 
			LandShakeNum += FT * 10;

		LerpDcc(LandShakeValue, 5, FT);
		LandShake = sin(LandShakeNum) * LandShakeValue;
	}

	// 플레이어는 쉐이더, 매쉬가 없는 객체이므로 렌더링 작업 생략
	void Render(ID3D12GraphicsCommandList* CmdList) {}


	void Update(float FT) {
		InitTransform();

		MovePlayer(FT);
		UpdateWalkingShake(FT);
		UpdateLandShake(FT);

		// 맵에 착지하면 흔들림 모션이 발생한다
		auto map = fw.FindObject("terrain", LayerRange::Single, Layer::Terrain);
		if (map) {
			if (fw.CheckTerrainFloor(this, map, 25.0f) && FallingAcc <= 0) {
				fw.MoveToTerrainFloor(this, map, 25.0f);

				if (JumpState) {
					LandShakeNum = 0;
					LandShakeValue = -FallingAcc * 0.3;
				}

				FallingAcc = 0.0f;
				JumpState = false;
			}

			else {
				// 높은곳에서 떨어질수록 더 많이 흔들리게 된다
				FallingAcc -= FT * 150;
				Position.y += FallingAcc * FT * 2;
			}
		}

		SetPosition(Position.x, Position.y, Position.z);
		Rotate(Rotation.x + LandShake, Rotation.y, Rotation.z + WalkingShake);
	}

	void ObjectKeyboardController(UINT nMessageID, WPARAM wParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			switch (wParam) {
			// 마우스 감도 증가
			case VK_PRIOR:
				MouseSensitivity += 0.1;
				break;

			// 마우스 감도 감소
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
				if (SpaceDown)
					break;

				// 점프 키는 한 번에 한 번씩만 누르도록 한다
				SpaceDown = true;
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
			case VK_SPACE:
				SpaceDown = false;
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