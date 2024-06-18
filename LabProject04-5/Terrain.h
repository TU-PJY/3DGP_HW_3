#pragma once
#include "Framework.h"
#include "Object.h"
#include <string>

// 터레인 클래스
class Terrain : public OBJ {
private:

public:
	Terrain(std::string tag, Layer layer) {
		SetTerrain(fw.FindTerrain("pTerrain"));
		SetShader(pShader);
		Scale(5.0, 5.0, 5.0);
		SetColor(XMFLOAT3(0.133333, 0.545098, 0.133333));

		ObjectLayer = layer;
		Tag = tag;
	}
};