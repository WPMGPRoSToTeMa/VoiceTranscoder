#pragma once

class FunctionHook {
public:
	virtual void Remove() = 0;
	virtual bool IsCreated() = 0;
	virtual void ReHook() = 0;
	virtual void UnHook() = 0;
	virtual ~FunctionHook() {}
};