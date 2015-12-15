#pragma once

#include "UtilTypes.h"

class Hook {
public:
	virtual void ReHook() = 0; // i want NULL
	virtual void UnHook() = 0;
	virtual ~Hook() {}
};