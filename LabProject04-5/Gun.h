#pragma once
#include "Framework.h"
#include "Object.h"
#include <cmath>

class Gun : public OBJ {
private:
	XMFLOAT3 TargetRotation{};
	bool FireState{};
	float FireDelay{};

public:
	Gun(std::string tag, Layer layer) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pM60Mesh"));
		SetColor(XMFLOAT3(0.3, 0.3, 0.3));

		ObjectLayer = layer;
		Tag = tag;
	}

	bool GetFireState() {
		return FireState;
	}

	void Update(float FT) {
		InitTransform();

		auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
		if (player) {
			Position = player->Position;
			TargetRotation = player->Rotation;
		}

		Rotation.x = std::lerp(Rotation.x, TargetRotation.x, 20 * FT);
		Rotation.y = std::lerp(Rotation.y, TargetRotation.y, 20 * FT);
		Rotation.z = std::lerp(Rotation.z, TargetRotation.z, 20 * FT);

		SetPosition(Position);
		Rotate(Rotation.x, Rotation.y, Rotation.z);
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

	void ObjectMouseController(POINT CursorPos, bool LButtonDownState, bool RButtonDownState) {
		if (LButtonDownState)
			FireState = true;

		if (!LButtonDownState)
			FireState = false;
	}
};


class Fire : public OBJ {
private:
public:
	Fire(std::string tag, Layer layer) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pFireMesh"));
		SetColor(XMFLOAT3(1.0, 0.0, 0.0));

		ObjectLayer = layer;
		Tag = tag;
	}

	void Update(float FT) {
		InitTransform();

		auto gun = fw.FindObject("gun", LayerRange::Single, Layer::L1);
		if (gun) {
			Position = gun->Position;
			Rotation = gun->Rotation;
		}
		SetPosition(Position);
		Rotate(Rotation.x, Rotation.y, Rotation.z);
	}

	void Render(ID3D12GraphicsCommandList* CmdList) {
		if (ObjectShader)
			ObjectShader->Render(CmdList);

		UpdateShaderVariables(CmdList);

		auto gun = fw.FindObject("gun", LayerRange::Single, Layer::L1);
		if (gun) {
			if (gun->GetFireState()) {
				if (ObjectMesh)
					ObjectMesh->Render(CmdList);
			}
		}
	}
};