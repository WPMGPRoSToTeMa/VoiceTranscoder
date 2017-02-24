#pragma once

#include <cstddef>
#include <utility>

using namespace std;

template <typename T>
class Optional {
	union {
		T _value;
	};
	bool _hasValue;

	void DeconstructIfPresent() {
		if (_hasValue) {
			_value.~T();
			_hasValue = false;
		}
	}

	template <typename ...TArgs>
	void Construct(TArgs&&... args) {
		DeconstructIfPresent();

		new (&_value) T(std::forward<TArgs>(args)...);
		_hasValue = true;
	}
public:
	Optional() { _hasValue = false; }
	Optional(nullptr_t) : Optional() { }

	~Optional() { DeconstructIfPresent(); }

	bool HasValue() const { return _hasValue; }

	Optional& operator=(nullptr_t) {
		DeconstructIfPresent();
		return *this;
	}
	Optional& operator=(const T& value) {
		Construct(value);
		return *this;
	}

	const T& operator*() const& { return _value; }
	T& operator*() & { return _value; }
};

template <typename T>
bool operator>=(const T& lhs, const Optional<T>& rhs) { return !rhs.HasValue() ? true : lhs >= *rhs; }

template <typename T>
bool operator>(const Optional<T>& lhs, const T& rhs) { return !lhs.HasValue() ? false : *lhs > rhs; }