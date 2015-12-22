#pragma once

#include "AnyPointer.h"
#include "UtilTypes.h"

class Section {
public:
	Section() {
		m_startPtr = (uintptr_t)nullptr;
		m_endPtr = (uintptr_t)nullptr;
		m_pNext = nullptr;
	}
	Section(AnyPointer startPtr, size_t size) : m_startPtr(startPtr), m_endPtr((uintptr_t)startPtr + size), m_pNext(nullptr) {}
	void Set(AnyPointer startPtr, AnyPointer endPtr) {
		m_startPtr = startPtr;
		m_endPtr = endPtr;
	}
	AnyPointer FindString(const char *str) const;
	Section *GetNext() const { return m_pNext; }
	AnyPointer GetStart() const { return m_startPtr; }
	AnyPointer GetEnd() const { return m_endPtr; }
	void SetNext(Section *pSection) { m_pNext = pSection; }
private:
	uintptr_t m_startPtr;
	uintptr_t m_endPtr;
	Section *m_pNext;
};