#pragma once
#include "Framework.h"
#include "Object.h"
#include "Ammo.h"
#include <cmath>

class Gun : public OBJ {
private:
	XMFLOAT3 TargetRotation{};
	bool TriggerState{};
	bool Fire{};
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
		return Fire;
	}

	void UpdateShooting(float FT) {
		if (FireDelay > 0) {
			FireDelay -= FT;
			if (FireDelay <= 0)
				FireDelay = 0;

			Fire = false;
		}

		if (TriggerState) {
			if (FireDelay == 0) {
				FireDelay = 0.12;
				Fire = true;
				auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
				if (player) player->GiveRecoil(3.0);

				// 총알을 발사한다
				fw.AddObject(new Ammo("ammo", Layer::L2, Up, Look, Position), Layer::L2);
			}
		}
	}

	void Update(float FT) {
		InitTransform();

		UpdateShooting(FT);

		auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
		if (player) {
			Position = player->Position;
			TargetRotation = player->Rotation;
		}

		Rotation.x = std::lerp(Rotation.x, TargetRotation.x, 15 * FT);
		Rotation.y = std::lerp(Rotation.y, TargetRotation.y, 15 * FT);
		Rotation.z = std::lerp(Rotation.z, TargetRotation.z, 15 * FT);

		SetPosition(Position.x, Position.y, Position.z );
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
			TriggerState = true;

		if (!LButtonDownState)
			TriggerState = false;
	}
};


class Fire : public OBJ {
private:
	float RenderTime{};

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
			if (gun->GetFireState())
				RenderTime = 0.05;
		}

		if(RenderTime > 0)
			RenderTime -= FT;

		SetPosition(Position);
		Rotate(Rotation.x, Rotation.y, Rotation.z);
	}

	void Render(ID3D12GraphicsCommandList* CmdList) {
		if (RenderTime > 0) {
			if (ObjectShader)
				ObjectShader->Render(CmdList);

			UpdateShaderVariables(CmdList);

			if (ObjectMesh)
				ObjectMesh->Render(CmdList);
		}
	}
};