#pragma once

#include "UtilTypes.h"

class Hook {
public:
	virtual void ReHook() = NULL;
	virtual void UnHook() = NULL;
	virtual ~Hook() {
		UnHook();
	}
};