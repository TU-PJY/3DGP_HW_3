#pragma once
#include "Framework.h"
#include "Object.h"
#include "EnemyMissile.h"

// ��
class Enemy : public OBJ {
private:
	float HP = 1.0;
	float MissileDelay = 20;
	XMFLOAT3 LookAtPlayer{};
public:
	Enemy(std::string tag, Layer layer) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pEnemyMesh"));
		SetColor(XMFLOAT3(0.0, 0.0, 1.0));

		ObjectLayer = layer;
		Tag = tag;
		Position = XMFLOAT3(100, 100, 20);
		SetPosition(Position);
	}

	void Update(float FT) {
		SetPosition(Position);
		MissileDelay -= FT * 10;
		if (MissileDelay <= 0) {
			auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
			if (player) {
				fw.AddObject(new EnemyMissile("missile", Layer::L2, Up, Look, Position), Layer::L2);
			}
			MissileDelay = 20;
		}

		//  �÷��̾ ����ٴѴ�
		auto player = fw.FindObject("player", LayerRange::Single, Layer::L1);
		if (player) {
			XMVECTOR xmvPosition = XMLoadFloat3(&Position);
			XMFLOAT3 xmf3targetPosition = player->Position;
			XMVECTOR xmvtargetPosition = XMLoadFloat3(&xmf3targetPosition);
			XMVECTOR xmvToTargetObject = xmvtargetPosition - xmvPosition;
			xmvToTargetObject = XMVector3Normalize(xmvToTargetObject);

			XMVECTOR xmvMovingDirection = XMLoadFloat3(&MovingDirection);
			xmvMovingDirection = XMVector3Normalize(xmvToTargetObject);
			XMStoreFloat3(&MovingDirection, xmvMovingDirection);

			LookTo(MovingDirection, XMFLOAT3(0.0, 1.0, 0.0));
			MoveNoY(MovingDirection, 20 * FT);
		}

		Scale(10.0, 10.0, 10.0);
		SetPosition(Position);

		UpdateOOBB();

		// �Ѿ˿� ������ ü���� �����ϰ� ���� ���� ���������� ���Ѵ�
		for (int i = 0; i < fw.Size(Layer::L2); ++i) {
			auto ammo = fw.FindObject("ammo", Layer::L2, i);
			if (ammo) {
				if (fw.CheckCollision(this, ammo)) {
					HP -= 0.005;
					if (HP <= 0)
						HP = 0;
					SetColor(XMFLOAT3(1.0 - HP, 0.0, HP));
				}
			}
		}
	}

	void CheckDelete() {

	}

	void ObjectKeyboardController(UINT nMessageID, WPARAM wParam) {
		switch (nMessageID) {
		case WM_KEYDOWN:
			break;

		case WM_KEYUP:
			break;
		}
	}
};