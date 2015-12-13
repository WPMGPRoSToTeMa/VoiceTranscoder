#pragma once

#include "UtilTypes.h"

class Hook {
public:
	virtual void ReHook() = NULL; // i want nullptr
	virtual void UnHook() = NULL;
	virtual ~Hook() {}
};