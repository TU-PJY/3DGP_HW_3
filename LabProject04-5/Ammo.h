#pragma once
#include "Framework.h"
#include "Object.h"

class Ammo : public OBJ {
private:
	float MoveDistance{};

public:
	Ammo(std::string tag, Layer layer, XMFLOAT3 Up, XMFLOAT3 Direction, XMFLOAT3 Position) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pAmmoMesh"));
		SetColor(XMFLOAT3(1.0, 1.0, 0.0));
		Tag = tag;
		ObjectLayer = layer;

		LookAt(Direction, Up);
		SetPosition(Position);
		SetMovingDirection(Direction);
	}

	void Update(float FT) {
		Move(MovingDirection,  FT * 400);

		MoveDistance += FT * 200;

		SetPosition(Position);

		UpdateOOBB();
	}

	void ProcessDelete() {
		// 일정 거리 이상 움직이거나 맵에 부딫히거냐 적과 충돌하면 스스로 삭제한다 
		if (MoveDistance >= 250)
			fw.DeleteObject(this, ObjectLayer);

		auto map = fw.FindObject("terrain", LayerRange::Single, Layer::Terrain);
		if (map) {
		if (fw.CheckTerrainFloor(this, map, 0.0))
			fw.DeleteObject(this, ObjectLayer);
		}

		auto enemy = fw.FindObject("enemy", LayerRange::Single, Layer::L3);
		if (enemy) {
			if (fw.CheckCollision(this, enemy))
				fw.DeleteObject(this, ObjectLayer);
		}
	}
};