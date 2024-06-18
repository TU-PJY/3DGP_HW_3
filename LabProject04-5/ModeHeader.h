#pragma once
#include "Framework.h"
#include "CustomObjects.h"
#include <string>

inline std::string Mode1() {
	fw.AddObject(new Aircraft(Layer::L1, "obj1"), Layer::L1);
	fw.AddObject(new Map(Layer::Terrain, "map"), Layer::Terrain);

	return __func__;
}