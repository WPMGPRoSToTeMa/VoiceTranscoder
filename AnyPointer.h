#pragma once

#include "UtilTypes.h"

class AnyPointer {
	uintptr_t m_ptr;
public:
	AnyPointer() {
		m_ptr = (uintptr_t)nullptr;
	}
	AnyPointer(uintptr_t ptr) {
		m_ptr = ptr;
	}
	AnyPointer(void *ptr) {
		m_ptr = (uintptr_t)ptr;
	}

	template <typename T>
	operator T() const {
		return (T)m_ptr;
	}
};