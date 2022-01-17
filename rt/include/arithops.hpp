#pragma once

#define ACL_RT_ARITHOPS_DECL(T)                         \
	StrongRef<T> operator+(ObserverRef<T> other) const; \
	StrongRef<T> operator-(ObserverRef<T> other) const; \
	StrongRef<T> operator*(ObserverRef<T> other) const; \
	StrongRef<T> operator/(ObserverRef<T> other) const;

#define ACL_RT_BITOPS_DECL(T)                            \
	StrongRef<T> operator&(ObserverRef<T> other) const;  \
	StrongRef<T> operator|(ObserverRef<T> other) const;  \
	StrongRef<T> operator^(ObserverRef<T> other) const;  \
	StrongRef<T> operator<<(ObserverRef<T> other) const; \
	StrongRef<T> operator>>(ObserverRef<T> other) const; \
	StrongRef<T> operator~() const;

#define ACL_RT_INTOPS_DECL(T) \
	ACL_RT_ARITHOPS_DECL(T)   \
	ACL_RT_BITOPS_DECL(T)     \
	StrongRef<T> operator%(ObserverRef<T> other) const;

#define ACL_RT_ARITHOPS_DEF(T)                                  \
	StrongRef<T> T::operator+(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value + other->value)); \
	}                                                           \
	StrongRef<T> T::operator-(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value - other->value)); \
	}                                                           \
	StrongRef<T> T::operator*(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value * other->value)); \
	}                                                           \
	StrongRef<T> T::operator/(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value / other->value)); \
	}

#define ACL_RT_BITOPS_DEF(T)                                     \
	StrongRef<T> T::operator&(ObserverRef<T> other) const {      \
		return StrongRef<T>(new T(this->value & other->value));  \
	}                                                            \
	StrongRef<T> T::operator|(ObserverRef<T> other) const {      \
		return StrongRef<T>(new T(this->value | other->value));  \
	}                                                            \
	StrongRef<T> T::operator^(ObserverRef<T> other) const {      \
		return StrongRef<T>(new T(this->value ^ other->value));  \
	}                                                            \
	StrongRef<T> T::operator<<(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value << other->value)); \
	}                                                            \
	StrongRef<T> T::operator>>(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value >> other->value)); \
	}                                                            \
	StrongRef<T> T::operator~() const {                          \
		return StrongRef<T>(new T(~this->value));                \
	}

#define ACL_RT_INTOPS_DEF(T)                                    \
	ACL_RT_ARITHOPS_DEF(T)                                      \
	ACL_RT_BITOPS_DEF(T)                                        \
	StrongRef<T> T::operator%(ObserverRef<T> other) const {     \
		return StrongRef<T>(new T(this->value % other->value)); \
	}

// This is just so the compiler will shut up about the fact that there is
// otherwise a backslash-newline at the end of the file
namespace acl {}