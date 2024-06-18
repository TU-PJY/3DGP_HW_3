#pragma once
#include "Framework.h"
#include "Object.h"
#include <string>

// �ͷ��� Ŭ����
class Terrain : public OBJ {
private:

public:
	Terrain(std::string tag, Layer layer) {
		SetTerrain(fw.FindTerrain("pTerrain"));
		SetShader(pShader);
		SetPosition(0.0, -10.0, 0.0);
		Scale(5.0, 5.0, 5.0);
		SetColor(XMFLOAT3(0.133333, 0.545098, 0.133333));

		ObjectLayer = layer;
		Tag = tag;
	}
};