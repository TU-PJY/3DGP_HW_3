#pragma once
#include "Framework.h"
#include "Object.h"
#include <string>

// �ͷ��� Ŭ����
class Terrain : public OBJ {
private:

public:
	Terrain(std::string tag, Layer layer) {
		SetShader(pShader);
		SetTerrain(fw.FindTerrain("pTerrain"));
		SetColor(XMFLOAT3(0.133333, 0.545098, 0.133333));
		Scale(5.0, 5.0, 5.0);

		ObjectLayer = layer;
		Tag = tag;
	}
};