#pragma once
#include "Framework.h"
#include "Object.h"

class EnemyMissile : public OBJ {
private:

public:
	EnemyMissile(std::string tag, Layer layer, XMFLOAT3 Up, XMFLOAT3 Direction, XMFLOAT3 Position) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pMissileMesh"));

		LookAt(Direction, Up);
		SetPosition(Position);
		SetMovingDirection(Direction);
		Scale(10, 10, 10);
		SetColor(XMFLOAT3(1.0, 0.0, 0.0));

		ObjectLayer = layer;
		Tag = tag;
	}

	void Update(float FT) {
		Move(MovingDirection, FT * 100);
		Rotate(0.0, 0.0, 500 * FT);

		SetPosition(Position);
	}

	void ProcessDelete() {

	}
};