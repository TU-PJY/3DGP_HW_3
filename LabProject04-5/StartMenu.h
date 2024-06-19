#pragma once
#include "Framework.h"
#include "Object.h"

class StartMenu : public OBJ {
private:

public:
	StartMenu(std::string tag, Layer layer) {
		SetShader(pShader);
		SetMesh(fw.FindMesh("pMenuMesh"));

		ObjectLayer = layer;
		Tag = tag;

		SetPosition(0.0, 0.0, 10.0);
	}

	void Update(float FT) {
		Rotate(0.0, 300 * FT, 0.0);
	}
};